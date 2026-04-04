use core::fmt;

use crate::utils::intrin::{inb, outb};

pub const COM1_PORT: u16 = 0x3F8;

pub struct Serial {
    pub serial_port: u16,
}

impl Serial {
    pub fn new(port: u16) -> Option<Self> {
        outb(port + 1, 0x00);
        outb(port + 3, 0x80); // Enable DLAB (set baud rate divisor)
        outb(port, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        outb(port + 1, 0x00); //                  (hi byte)
        outb(port + 3, 0x03); // 8 bits, no parity, one stop bit
        outb(port + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        outb(port + 4, 0x0B); // IRQs enabled, RTS/DSR set
        outb(port + 4, 0x1E); // Set in loopback mode, test the serial chip
        outb(port, 0xAE);

        if inb(port) != 0xAE {
            return None
        }

        outb(port + 4, 0x0F);

        Some(Self {
            serial_port: port,
        })
    }

    pub fn write_char(&self, c: u8) {
        while inb(self.serial_port + 5) & 0x20 == 0 {}

        outb(self.serial_port, c);
    }

    pub fn read_char(&self) -> u8 {
        while inb(self.serial_port + 5) & 1 == 0 {}

        inb(self.serial_port)
    }
}

impl fmt::Write for Serial {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for byte in s.bytes() {
            Serial::write_char(self, byte);
        }
        Ok(())
    }
}
