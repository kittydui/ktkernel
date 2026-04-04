use core::fmt;

use crate::utils::serial::{Serial, COM1_PORT};

pub fn _log(args: fmt::Arguments) {
    use fmt::Write;

    let mut serial = Serial { serial_port: COM1_PORT };
    serial.write_fmt(args).unwrap();
}

#[macro_export]
macro_rules! log {
    ($($arg:tt)*) => {
        $crate::utils::log::_log(format_args!("{}\n", format_args!($($arg)*)))
    };
}
