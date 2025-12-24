//! WS2812B LED control using ESP32-C6 RMT peripheral
//!
//! Provides async LED control for Matter On/Off Light functionality.

use esp_hal::{gpio::Level, rmt::PulseCode};

// WS2812B timing (in RMT ticks at 80MHz, 1 tick = 12.5ns)
// T0H: 400ns = 32 ticks, T0L: 850ns = 68 ticks
// T1H: 800ns = 64 ticks, T1L: 450ns = 36 ticks
const WS_T0H: u16 = 32;
const WS_T0L: u16 = 68;
const WS_T1H: u16 = 64;
const WS_T1L: u16 = 36;

/// Generate WS2812B pulse code for a single bit
fn ws2812_bit(bit: bool) -> PulseCode {
    if bit {
        PulseCode::new(Level::High, WS_T1H, Level::Low, WS_T1L)
    } else {
        PulseCode::new(Level::High, WS_T0H, Level::Low, WS_T0L)
    }
}

/// Convert RGB to WS2812B pulse codes (GRB order)
pub fn rgb_to_pulses(r: u8, g: u8, b: u8, pulses: &mut [PulseCode; 25]) {
    let grb: u32 = ((g as u32) << 16) | ((r as u32) << 8) | (b as u32);
    for (i, pulse) in pulses.iter_mut().take(24).enumerate() {
        *pulse = ws2812_bit((grb >> (23 - i)) & 1 == 1);
    }
    pulses[24] = PulseCode::end_marker();
}

/// Simple HSV to RGB conversion (hue 0-359, sat/val 0-255)
pub fn hsv_to_rgb(h: u16, s: u8, v: u8) -> (u8, u8, u8) {
    if s == 0 {
        return (v, v, v);
    }

    let region = h / 60;
    let remainder = ((h % 60) * 255) / 60;

    let p = ((v as u16) * (255 - s as u16) / 255) as u8;
    let q = ((v as u16) * (255 - (s as u16 * remainder) / 255) / 255) as u8;
    let t = ((v as u16) * (255 - (s as u16 * (255 - remainder)) / 255) / 255) as u8;

    match region {
        0 => (v, t, p),
        1 => (q, v, p),
        2 => (p, v, t),
        3 => (p, q, v),
        4 => (t, p, v),
        _ => (v, p, q),
    }
}
