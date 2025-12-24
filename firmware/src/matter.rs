//! Matter device implementation
//!
//! Provides On/Off Light device type using rs-matter stack.
//! Uses test device attestation credentials for development.

use core::cell::Cell;
use core::future::Future;

use rs_matter_embassy::matter::dm::clusters::basic_info::BasicInfoConfig;
use rs_matter_embassy::matter::dm::clusters::decl::on_off as on_off_decl;
use rs_matter_embassy::matter::dm::clusters::on_off::{
    EffectVariantEnum, OnOffHooks, OutOfBandMessage, StartUpOnOffEnum,
};
use rs_matter_embassy::matter::dm::devices::test::TEST_DEV_COMM;
use rs_matter_embassy::matter::dm::Cluster;
use rs_matter_embassy::matter::error::Error;
use rs_matter_embassy::matter::tlv::Nullable;
use rs_matter_embassy::matter::with;
use rs_matter_embassy::matter::BasicCommData;

/// On/Off light state (shared with LED control)
pub struct LightState {
    on: Cell<bool>,
    start_up_on_off: Cell<Option<StartUpOnOffEnum>>,
}

impl LightState {
    pub const fn new() -> Self {
        Self {
            on: Cell::new(false),
            start_up_on_off: Cell::new(None),
        }
    }

    pub fn is_on(&self) -> bool {
        self.on.get()
    }
}

impl OnOffHooks for &LightState {
    const CLUSTER: Cluster<'static> = on_off_decl::FULL_CLUSTER
        .with_revision(6)
        .with_attrs(with!(
            required;
            on_off_decl::AttributeId::OnOff
        ))
        .with_cmds(with!(
            on_off_decl::CommandId::Off
                | on_off_decl::CommandId::On
                | on_off_decl::CommandId::Toggle
        ));

    fn on_off(&self) -> bool {
        self.on.get()
    }

    fn set_on_off(&self, on: bool) {
        log::info!("Light state changed to: {}", if on { "ON" } else { "OFF" });
        self.on.set(on);
    }

    fn start_up_on_off(&self) -> Nullable<StartUpOnOffEnum> {
        match self.start_up_on_off.get() {
            Some(value) => Nullable::some(value),
            None => Nullable::none(),
        }
    }

    fn set_start_up_on_off(&self, value: Nullable<StartUpOnOffEnum>) -> Result<(), Error> {
        self.start_up_on_off.set(value.into_option());
        Ok(())
    }

    async fn handle_off_with_effect(&self, _effect: EffectVariantEnum) {}

    fn run<F: Fn(OutOfBandMessage)>(&self, _notify: F) -> impl Future<Output = ()> {
        // No external input handling for now
        core::future::pending::<()>()
    }
}

/// Device information for Smart Garland
pub const DEV_INFO: BasicInfoConfig = BasicInfoConfig {
    vid: 0xFFF1,         // Test vendor ID
    pid: 0x8002,         // Test product ID
    hw_ver: 1,
    hw_ver_str: "1.0",
    sw_ver: 1,
    sw_ver_str: "0.1.0",
    serial_no: "GARLAND001",
    product_name: "Smart Garland",
    vendor_name: "DIY",
    device_name: "Smart Garland",
    ..BasicInfoConfig::new()
};

/// Commissioning data (standard test values)
pub const DEV_COMM: BasicCommData = TEST_DEV_COMM;

/// Re-export test device attestation
pub use rs_matter_embassy::matter::dm::devices::test::TEST_DEV_ATT;
