This library allows an Arduino board to control liquid crystal displays (LCDs) based on the Hitachi HD44780 (or a compatible) chipset, which is found on most text-based LCDs.

Modified by John Greenwell to adapt driver for custom HAL support, 2024.

## Usage

For this modified version, the following hardware abstraction layer (HAL) requirements must be satisfied:

* A header file `hal.h` providing access to HAL namespace classes and methods.
* A `GPIO` class within the `HAL` namespace with the following methods:
  - Set pin mode: `pinMode(uint8_t mode)`
  - Write logic level to pin: `digitalWrite(uint8_t val)`
* A `GPIOPort` class within the `HAL` namespace which is initialized with an array of pins and supplies the following methods:
  - Set pin mode: `pinMode(uint8_t pin, uint8_t mode)`
  - Write logic level to pin: `digitalWrite(uint8_t pin, uint8_t val)`
  - Note that pin refers to index of pin in array, not the original pin value. This permits abstracting actual pin values to an array which may be treated similar to a port.
* A delay_us() function in the HAL namespace that delays an accurate microseconds to be used for timing.

Some further requirements may also be found. Typically, these will mirror the Arduino framework and should be added to `hal.h`.

For more information about this library please visit us at

https://www.arduino.cc/en/Reference/{repository-name}

== License ==

Copyright (C) 2006-2008 Hans-Christoph Steiner. All rights reserved.
Copyright (c) 2010 Arduino LLC. All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
