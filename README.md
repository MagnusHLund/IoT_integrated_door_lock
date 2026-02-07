# IoT Integrated door lock

## Features

## prerequisites

- 1x ESP32-C6
- 1x 5v relay (or mosfet)
- 1x 5v buck converter
- 1x 2a fuse
- 1x 12v solenoid door lock

Additionally you need a way to supply 12 volts 2-3 amps. <br>
Battery or wall power.

## Wiring

## How to setup

Configure the configuration file. Read the README file within the main/ directory.

Flash the code to your ESP32.

Within home assistant and within Zigbee2MQTT, set permit join.

The device will then be available within home assistant.
