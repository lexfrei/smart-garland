fn main() {
    // Link against ESP32-C6 ROM functions
    println!("cargo:rustc-link-arg=-Tlinkall.x");
}
