#![no_std]
#![no_main]
#![recursion_limit = "256"]

extern crate alloc;

mod led;
mod matter;

use core::pin::pin;

use embassy_executor::Spawner;
use embassy_time::{Duration, Timer};
use esp_backtrace as _;
use esp_hal::{
    interrupt::software::SoftwareInterruptControl,
    rmt::{Channel, PulseCode, Rmt, Tx, TxChannelConfig, TxChannelCreator},
    time::Rate,
    timer::timg::TimerGroup,
};
use log::info;

use rs_matter_embassy::epoch::epoch;
use rs_matter_embassy::matter::dm::clusters::desc::{self, ClusterHandler as _};
use rs_matter_embassy::matter::dm::clusters::on_off::{self, OnOffHooks};
use rs_matter_embassy::matter::dm::devices::DEV_TYPE_ON_OFF_LIGHT;
use rs_matter_embassy::matter::dm::{Async, Dataver, EmptyHandler, Endpoint, EpClMatcher, Node};
use rs_matter_embassy::matter::utils::init::InitMaybeUninit;
use rs_matter_embassy::matter::{clusters, devices, BasicCommData};
use rs_matter_embassy::rand::esp::{esp_init_rand, esp_rand};
use rs_matter_embassy::stack::persist::DummyKvBlobStore;
use rs_matter_embassy::stack::rand::RngCore;
use rs_matter_embassy::wireless::esp::EspThreadDriver;
use rs_matter_embassy::wireless::{EmbassyThread, EmbassyThreadMatterStack};

use tinyrlibc as _;

use crate::led::{hsv_to_rgb, rgb_to_pulses};
use crate::matter::{DEV_INFO, LightState};

esp_bootloader_esp_idf::esp_app_desc!();

/// Static cell macro for allocation
macro_rules! mk_static {
    ($t:ty) => {{
        static STATIC_CELL: static_cell::StaticCell<$t> = static_cell::StaticCell::new();
        STATIC_CELL.uninit()
    }};
}

/// Memory for rs-matter-stack futures (bump allocator)
/// ESP32 needs more memory for Matter-over-Thread with BLE commissioning
const BUMP_SIZE: usize = 40000;

/// Heap size for Matter stack + x509
/// ESP32 has non-contiguous memory, needs larger heap
const HEAP_SIZE: usize = 140 * 1024;

/// Endpoint 1 for the light device
const LIGHT_ENDPOINT_ID: u16 = 1;

/// Matter Light device Node
const NODE: Node = Node {
    id: 0,
    endpoints: &[
        EmbassyThreadMatterStack::<0, ()>::root_endpoint(),
        Endpoint {
            id: LIGHT_ENDPOINT_ID,
            device_types: devices!(DEV_TYPE_ON_OFF_LIGHT),
            clusters: clusters!(desc::DescHandler::CLUSTER, <&LightState>::CLUSTER),
        },
    ],
};

#[esp_rtos::main]
async fn main(spawner: Spawner) {
    esp_println::logger::init_logger(log::LevelFilter::Debug);

    info!("Smart Garland starting...");

    // Initialize heap allocator
    esp_alloc::heap_allocator!(size: HEAP_SIZE);

    // Initialize ESP-HAL
    let peripherals = esp_hal::init(esp_hal::Config::default());

    // Initialize RNG
    let mut rng = esp_hal::rng::Rng::new();

    // Random discriminator for each session (avoids SRP conflicts)
    let discriminator = (rng.next_u32() & 0xfff) as u16;

    // Random IEEE EUI-64 for Thread
    let mut ieee_eui64 = [0u8; 8];
    rng.fill_bytes(&mut ieee_eui64);

    // Initialize global rand function for Matter
    esp_init_rand(rng);

    // Initialize timer and software interrupt for esp-rtos
    let sw_int = SoftwareInterruptControl::new(peripherals.SW_INTERRUPT);
    let timg0 = TimerGroup::new(peripherals.TIMG0);
    esp_rtos::start(timg0.timer0, sw_int.software_interrupt0);

    // Initialize ESP radio (handles both 802.15.4 and BLE)
    let init = esp_radio::init().unwrap();
    info!("ESP radio initialized (Thread + BLE)");

    // Create commissioning data with random discriminator
    let comm_data = BasicCommData {
        password: matter::DEV_COMM.password,
        discriminator,
    };

    // Allocate Matter stack statically
    let stack = mk_static!(EmbassyThreadMatterStack::<BUMP_SIZE, ()>).init_with(
        EmbassyThreadMatterStack::init(
            &DEV_INFO,
            comm_data,
            &matter::TEST_DEV_ATT,
            epoch,
            esp_rand,
        ),
    );

    info!(
        "Matter stack initialized (VID={:#06x}, PID={:#06x})",
        DEV_INFO.vid, DEV_INFO.pid
    );

    // Print pairing info
    info!("=== PAIRING INFO ===");
    info!("Discriminator: {}", discriminator);
    info!("Passcode: {}", matter::DEV_COMM.password);

    // Create light state
    let light_state: &'static LightState = mk_static!(LightState).write(LightState::new());

    // Create OnOff handler for endpoint 1
    let on_off = on_off::OnOffHandler::new_standalone(
        Dataver::new_rand(stack.matter().rand()),
        LIGHT_ENDPOINT_ID,
        light_state,
    );

    // Chain endpoint handlers
    let handler = EmptyHandler
        .chain(
            EpClMatcher::new(Some(LIGHT_ENDPOINT_ID), Some(<&LightState>::CLUSTER.id)),
            on_off::HandlerAsyncAdaptor(&on_off),
        )
        .chain(
            EpClMatcher::new(Some(LIGHT_ENDPOINT_ID), Some(desc::DescHandler::CLUSTER.id)),
            Async(desc::DescHandler::new(Dataver::new_rand(stack.matter().rand())).adapt()),
        );

    // Create persister (dummy for now - no flash persistence)
    let persist = stack
        .create_persist_with_comm_window(DummyKvBlobStore)
        .await
        .unwrap();

    info!("Commissioning window open (BLE advertising active)");

    // Initialize LED on GPIO8
    let rmt = Rmt::new(peripherals.RMT, Rate::from_mhz(80))
        .unwrap()
        .into_async();

    let channel = rmt
        .channel0
        .configure_tx(peripherals.GPIO8, TxChannelConfig::default().with_clk_divider(1))
        .unwrap();

    info!("WS2812B on GPIO8 ready");

    // Spawn LED control task
    spawner.spawn(led_control_task(light_state, channel)).unwrap();

    // Run Matter stack with Thread + BLE commissioning
    let matter = pin!(stack.run(
        // Thread driver with BLE support
        EmbassyThread::new(
            EspThreadDriver::new(&init, peripherals.IEEE802154, peripherals.BT),
            ieee_eui64,
            persist.store(),
            stack,
        ),
        // Persister
        &persist,
        // Handler chain
        (NODE, handler),
        // No user future
        (),
    ));

    info!("Matter stack running - waiting for commissioning via BLE...");

    // Run Matter
    matter.await.unwrap();
}

/// LED control task - responds to Matter On/Off commands
#[embassy_executor::task]
async fn led_control_task(
    state: &'static LightState,
    mut channel: Channel<'static, esp_hal::Async, Tx>,
) {
    let mut pulses = [PulseCode::end_marker(); 25];
    let mut hue: u16 = 0;
    let mut last_on = false;

    loop {
        let is_on = state.is_on();

        // Log state changes
        if is_on != last_on {
            info!("LED state: {}", if is_on { "ON" } else { "OFF" });
            last_on = is_on;
        }

        // Rainbow when on, dim when off
        let brightness = if is_on { 200 } else { 20 };
        let (r, g, b) = hsv_to_rgb(hue, 255, brightness);
        rgb_to_pulses(r, g, b, &mut pulses);
        channel.transmit::<PulseCode>(&pulses).await.unwrap();

        hue = (hue + 1) % 360;
        Timer::after(Duration::from_millis(if is_on { 20 } else { 100 })).await;
    }
}
