#![no_std]
#![no_main]

extern crate alloc;

mod thread;

use embassy_executor::Spawner;
use embassy_time::{Duration, Timer};
use esp_backtrace as _;
use esp_hal::{
    gpio::Level,
    interrupt::software::SoftwareInterruptControl,
    rmt::{PulseCode, Rmt, TxChannelConfig, TxChannelCreator},
    time::Rate,
    timer::timg::TimerGroup,
};
use log::info;

esp_bootloader_esp_idf::esp_app_desc!();

// WS2812B timing (in RMT ticks at 80MHz, 1 tick = 12.5ns)
// T0H: 400ns = 32 ticks, T0L: 850ns = 68 ticks
// T1H: 800ns = 64 ticks, T1L: 450ns = 36 ticks
const WS_T0H: u16 = 32;
const WS_T0L: u16 = 68;
const WS_T1H: u16 = 64;
const WS_T1L: u16 = 36;

fn ws2812_bit(bit: bool) -> PulseCode {
    if bit {
        PulseCode::new(Level::High, WS_T1H, Level::Low, WS_T1L)
    } else {
        PulseCode::new(Level::High, WS_T0H, Level::Low, WS_T0L)
    }
}

// Convert RGB to WS2812B pulse codes (GRB order)
fn rgb_to_pulses(r: u8, g: u8, b: u8, pulses: &mut [PulseCode; 25]) {
    let grb: u32 = ((g as u32) << 16) | ((r as u32) << 8) | (b as u32);
    for i in 0..24 {
        pulses[i] = ws2812_bit((grb >> (23 - i)) & 1 == 1);
    }
    pulses[24] = PulseCode::end_marker();
}

#[esp_rtos::main]
async fn main(_spawner: Spawner) {
    esp_println::logger::init_logger_from_env();
    let peripherals = esp_hal::init(esp_hal::Config::default());

    // Initialize heap allocator (64KB)
    esp_alloc::heap_allocator!(size: 64 * 1024);

    info!("Smart Garland starting...");

    // Initialize esp-rtos runtime
    let sw_int = SoftwareInterruptControl::new(peripherals.SW_INTERRUPT);
    let timg0 = TimerGroup::new(peripherals.TIMG0);
    esp_rtos::start(timg0.timer0, sw_int.software_interrupt0);

    // RMT for WS2812B on GPIO8
    let rmt = Rmt::new(peripherals.RMT, Rate::from_mhz(80))
        .unwrap()
        .into_async();

    let mut channel = rmt
        .channel0
        .configure_tx(peripherals.GPIO8, TxChannelConfig::default().with_clk_divider(1))
        .unwrap();

    info!("WS2812B on GPIO8 ready");

    let mut pulses = [PulseCode::end_marker(); 25];
    let mut hue: u16 = 0;

    loop {
        // Simple HSV to RGB (hue only, full saturation/value)
        let (r, g, b) = hsv_to_rgb(hue, 255, 128);
        rgb_to_pulses(r, g, b, &mut pulses);

        channel.transmit(&pulses).await.unwrap();

        hue = (hue + 1) % 360;
        Timer::after(Duration::from_millis(20)).await;
    }
}

fn hsv_to_rgb(h: u16, s: u8, v: u8) -> (u8, u8, u8) {
    if s == 0 {
        return (v, v, v);
    }

    let region = h / 60;
    let remainder = ((h % 60) * 255) / 60;

    let p = ((v as u16) * (255 - s as u16) / 255) as u8;
    let q = ((v as u16) * (255 - (s as u16 * remainder as u16) / 255) / 255) as u8;
    let t = ((v as u16) * (255 - (s as u16 * (255 - remainder as u16)) / 255) / 255) as u8;

    match region {
        0 => (v, t, p),
        1 => (q, v, p),
        2 => (p, v, t),
        3 => (p, q, v),
        4 => (t, p, v),
        _ => (v, p, q),
    }
}
