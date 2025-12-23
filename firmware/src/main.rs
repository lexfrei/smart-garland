#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_time::{Duration, Timer};
use esp_backtrace as _;
use esp_hal::{gpio::{Level, Output}, timer::timg::TimerGroup};
use log::info;

#[esp_hal_embassy::main]
async fn main(_spawner: Spawner) {
    // Initialize logging
    esp_println::logger::init_logger_from_env();

    info!("Smart Garland starting...");

    // Initialize HAL
    let peripherals = esp_hal::init(esp_hal::Config::default());

    // Initialize timer for Embassy
    let timg0 = TimerGroup::new(peripherals.TIMG0);
    esp_hal_embassy::init(timg0.timer0);

    // LED on GPIO8 (WS2812B data pin on most C6 devkits)
    let mut led = Output::new(peripherals.GPIO8, Level::Low);

    info!("Blinking LED on GPIO8");

    loop {
        led.set_high();
        info!("LED ON");
        Timer::after(Duration::from_millis(500)).await;

        led.set_low();
        info!("LED OFF");
        Timer::after(Duration::from_millis(500)).await;
    }
}
