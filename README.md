# OpenPAYGO HDK

This repository is for the OpenPAYGO HDK (Hardware Development Kit), a ready-to-use implementation of OpenPAYGO technologies (currently Token and Pass) in a dev kit that is both convenient for prototyping and easy to turn into optimized production products. 


## Table of Content

  - [Hardware](#hardware)
  - [Firmware](#firmware)
  - [Integration](#integration)
  - [Getting Started](#getting-started)
    - [Compile and upload](#compile-and-upload)
    - [Initial Setup / Factory Setup](#initial-setup--factory-setup)
    - [Customisation](#customisation)
  - [Serial Commands](#serial-commands)
    - [Setup Command](#setup-command)
    - [Token Command](#token-command)
    - [Status command](#status-command)
  - [Understanding LED signals](#understanding-led-signals)
  - [OpenPAYGO Metrics Data Format](#openpaygo-metrics-data-format)



## Hardware

The HDK is based on the ESP32, this decision was taken because it offers the following advantages: 
- Arduino compatible (easy to prototype with)
- Very low power consumption in deep-sleep mode (0.005 mA), with RTC running and timer wake up to grab metrics at regular interval. This means it is usable even in small solar kits without draining the battery.
- Low cost
- Integrated RTC (for OpenPAYGO Token without the need for external components)
- Integrated BLE (for the upcoming OpenPAYGO AirBridge)
- Integrated WiFi (for OpenPAYGO Metrics)
- Has a lot of Flash storage and RAM, making it easy to add device features onto and allowing to store many days (even several months!) of metrics data pending to be sent

To use OpenPAYGO Pass, you will need any PN53x series NFC modules, wired via I2C. 


## Firmware

The provided firmware of the HDK offers support for: 
- OpenPAYGO Token: with all features, including unordered token entry, etc.
- OpenPAYGO Pass: with all features
- OpenPAYGO Metrics: fully supports data generation, but does not have mechanism for directly sending the data via WiFi (only via Pass). 
- OpenPAYGO Bridge: not yet supported (but possible on this hardware)
- OpenPAYGO AirBridge: not yet supported (but possible on this hardware)

The example in the firmware generates random value for the metrics measured (battery volage, power generated, hours of light provided) and allows storing up to 31 days of daily averages for those metrics on the OpenPAYGO Pass, pushing the system to its limits. 


## Integration

There are two approach to integrating OpenPAYGO Technologies into your system using this HDK: 

1. Simply place the module flashed with the code as-is into your circuit, and use the HDK as a PAYGO "coprocessor" handling all of the PAYGO functions for you. You just need to connect the UART of the ESP32 to the UART of your main processor to exchange data (token entered, metrics to save, etc.). 

2. Modify the code and use the module as your main controller, handling all the other functions that your system needs (UI, charge control, etc.) and calling directly the provided functions from the rest of your code (e.g. your UI can call the function to enter a token). The module has significant memory available to allow you to integrate many other features. 

**Recommendations:**
- The code comes with feature flags, you should disable the features that you do not need (e.g. you can disabled OpenPAYGO Pass if you do not have an NFC module). 
- Read the hardware design recommendation from the OpenPAYGO Token repository for guidelines on how to best secure the PAYGO functionality in your hardware design as a whole. 
- The UART communication with the HDK should be only with a safe source (e.g. your MCU or factory setup system) and should not be exposed. 
- Ensure to always supply power to keep the RTC running, it still runs even in very deep sleep using negligible power (5uA) so it can be powered by a backup battery if needed. 


## Getting Started

### Compile and upload
Just pull the Git repository, and open it either in the Arduino IDE, or in VSCode with the Arduino extension (recommended). 

Install the following libraries:
- ESP32Time (for the internal RTC)
- OpenPAYGO Token (included in the Git for now)

Just select your board (ESP32 Wroom), press "Upload" and everything should work. 

### Initial Setup / Factory Setup

To get started with a board, you first need to set it up with it's unique data; the serial number and secret key. You can do so using the `SETUP` command described in the **Serial Commands** section below. 

For **testing purposes ONLY** you can use the following command to setup your device: 
`#SETUP;123456789;123456789;a29ab82edc5fbbc41ec9530f6dac86b1`

When doing production runs, you can should randomly generate those data and store them in a CSV file to be given to your software provider. You can use the tools available on the OpenPAYGO Token repository for that purpose, or integrate with your existing factory tooling. 

### Customisation

You can find in the `hal.h` and `hal.cpp` files the configuration for the pins and functions that interact with the hardware (e.g. LED pins, load switch pin, number of keypad digits, etc.). In most cases you should only modify those files to get things working for your particular circuit if different from the example one. 


## Serial Commands

The commands are sent through serial to interact with the device, both during factory setup phase and as well during operation, if the device is used as a "coprocessor". Any command that is not recognized will cause the device to return `#INVALID`. 

**Note:** By default the serial baud rate is 115200.

### Setup Command

The `#SETUP` command is only available when the device has not been factory configured yet, it is used to set the factory settings into the memory of the device. The format of the command is the following: `#SETUP;<serial number>;<starting code>;<secret key>`. For example: `#SETUP;99999991;123456789;a29ab82edc5fbbc41ec9530f6dac86b1`. 
  

### Token Command

The `#TOKEN` command is used to report a token entry to the device. The token might have been entered via the UI of the device or any other method. The format of the command is the following: `#TOKEN;<token>`. You can expect the following answers, depending on the validity of the token: 

  - `#TOKEN;INVALID`: The token is invalid
  - `#TOKEN;ALREADY_USED`: The token is valid but was already used
  - `#TOKEN;PAYG_DISABLED`: The token is valid and PAYG mode is now disabled (device is unlocked)
  - `#TOKEN;VALID;3600`: The token is valid, the second value is the number of seconds of activation left.


### Status command

The `#STATUS` command is used to get the current PAYGO status of the device. There is no parameters to pass, just `#STATUS`. You can expect the following answers, depending on the PAYG status:

  - `#STATUS;INACTIVE`: The device is inactive (out of credit)
  - `#STATUS;ACTIVE;3600`: The device is active. the second value is the number of seconds of activation left. 
  - `#STATUS;PAYG_DISABLED`: PAYG is disabled on the device (it is unlocked forever)


## Understanding LED signals

The blue LED indicates an input, the green LED a success and the red LED an error. 

When tapping an NFC card:
- 1 red: card detected could not be read
- 1 blue + 1 red: card could be read but is not OpenPAYGO Pass
- 1 blue + 1 green + 1 red: card could be read but not written
- 1 blue + 2 green: card could be read and written

When entering a token (or receiving it through OpenPAYGO Metrics, Pass, etc.): 
- 5 red blinks: Invalid token
- 1 green blink: Valid token, but already used
- 2 green blink: Valid token, device active for a given period of time
- 5 green blink: Valid token, device with unlocked forever (PAYG disabled)


## OpenPAYGO Metrics Data Format

For this example, the following basic OpenPAYGO Metrics data format is used. You can check the (OpenPAYGO Metrics documentation)[https://github.com/openpaygo/metrics/blob/main/Specifications.md] to create a data format that matches your monitoring need. It can support daily averages for several days. 

**Data Format:**
```json
{
   "data_order":[
      "token_count",
      "firmware_version"
   ],
   "historical_data_interval": 86400,
   "historical_data_order": [
      "power_generated",
      "hours_of_lighting",
      "average_battery_voltage" 
   ],
   "variables":{
      "power_generated":{
         "name":"Power Generated",
         "type":"float",
         "unit":"Wh",
         "description": "The total power that was generated by the solar panel"
      },
      "hours_of_lighting":{
         "name":"Hours of Lighting Provided",
         "type":"float",
         "unit":"h",
         "description": "The number of hours that lights were on"
      },
      "average_battery_voltage":{
         "name":"Average Battery Voltage",
         "type":"float",
         "unit":"V",
         "description": "The average voltage of the battery (used for health)"
      }
   }
}
```
