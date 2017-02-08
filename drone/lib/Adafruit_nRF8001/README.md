# Adafruit_nRF8001 #

Driver and example code for Adafruit's nRF8001 Bluetooth Low Energy Breakout.

## PINOUT ##

The pin locations are defined in **ble_system.h**, the supported systems are defined in **hal_aci_tl.cpp**. The following pinout is used by default for the Arduino Uno:

* SCK -> Pin 13
* MISO -> Pin 12
* MOSI -> Pin 11
* REQ -> Pin 10
* RDY -> Pin 2 (HW interrupt)
* ACT -> Not connected
* RST -> Pin 9
* 3V0 - > Not connected
* GND -> GND
* VIN -> 5V

RDY must be on pin 2 since this pin requires a HW interrupt.

3V0 is an optional pin that exposes the output of the on-board 3.3V regulator. You can use this to supply 3.3V to other peripherals, but normally it will be left unconnected.

ACT is not currently used in any of the existing examples, and can be left unconnected if necessary.

# Examples #

The following examples are included for the Adafruit nRF8001 Breakout

## UART ##

This example creates a UART-style bridge between the Arduino and any BLE capable device.

You can send and receive up to 20 bytes at a time between your BLE-enabled phone or tablet and the Arduino.

Any data sent to the Arduino will be displayed in the Serial Monitor output, and echo'ed back to the phone or tablet on the mobile device's RX channel.

This demo creates a custom UART service, with one characteristic for TX and one for RX using the following UUIDs:

* 6E400001-B5A3-F393-E0A9-E50E24DCCA9E for the Service
* 6E400002-B5A3-F393-E0A9-E50E24DCCA9E for the TX Characteristic (Property = Notify)
* 6E400003-B5A3-F393-E0A9-E50E24DCCA9E for the RX Characteristic (Property = Write without response)

You can test the UART service with the free nRF UART apps from Nordic Semiconductors, available for both iOS and Android:

* Compatible iOS devices: https://itunes.apple.com/us/app/nrf-uart/id614594903?mt=8
* Compatible Android 4.3 and higher devices: https://play.google.com/store/apps/details?id=com.nordicsemi.nrfUARTv2
* Compatible pre Android 4.3 Samsung devices (uses a proprietary Samsung BLE stack): https://play.google.com/store/apps/details?id=com.nordicsemi.nrfUART
