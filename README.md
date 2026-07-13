# lynk-ats-box
An ESP32-based controller for automatically switching household loads between the electrical grid and an inverter powered by a battery/solar system.

The controller monitors battery state, AC output power, and user-defined thresholds to maximize solar energy usage while protecting the battery from excessive discharge.

## Features

* Automatic switching between **Grid** and **Inverter** modes
* Battery state monitoring via **JK-BMS**
* AC power measurement using **JSY-1050**
* Configurable switching thresholds
* OLED display with rotary encoder navigation
* Telegram bot for remote monitoring and control
* Audible notifications with buzzer
* LED status indication
* Persistent configuration stored in ESP32 NVS
* Power generation and runtime statistics

---

## Hardware

### Distribution box with Controller

* ESP32
* Energy Meter JSY-1050
* Passive buzzer
* 2.42" OLED display 128x64 SSD1309 I2C
* Rotary encoder with push button
* Tomzn Automatic Transfer Switch 2P 125A
* Relay Geye 2P 25A 220V
* Slim SSR DC-AC x3

### Battery with BMS and Inverter

* 6s LTO Yinlong 45Ah
* JK-BMS 4s-8s 100A B1A8S10P
* Foval Pure Sine Wave Inverter 12V->220V 2200W



## Setup:
1. Create secrets.h and define next variables:
```
const String WIFI_SSID = "MY_WIFI";
const String WIFI_PASS = "MY_PASSWORD";
const String TG_BOT_TOKEN = "1234567890:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
const String ADMIN_ID = "123456789";
```

## Project Structure

```
lynk-ats-box.ino       Main application loop
global.h               Global variables and configuration

LynkNVS.h              Persistent configuration and statistics
LynkJkBms.h            JK-BMS communication
LynkJsy1050.h          JSY-1050 communication
LynkTelegramBot.h      Telegram integration
ScreenManager.h        OLED user interface
LynkEncoder.h          Rotary encoder
LynkLED.h              LED control
LynkBuzzer.h           Sound notifications
```

---

## Operating Modes

The controller operates as a state machine.

- GRID

The load is supplied from the electrical grid.
The controller continuously monitors battery state-of-charge and available solar power.

- INV_PREHEAT

The inverter is powered on and allowed to stabilize before transferring the load.

- TO_INV

Switching sequence from Grid to Inverter.

- INV

The load is supplied entirely by the inverter.
The controller monitors battery SOC and output power to determine when it should return to grid operation.

- INV_PLUS

Additional relay enabled while operating on the inverter.

- TO_GRID

Switching sequence back to the electrical grid.

---

## Automatic Switching

The controller decides when to change operating mode based on:

* Battery State of Charge (SOC)
* Solar production
* Load power
* User configured thresholds
* Manual override commands

Several delays are intentionally introduced to prevent rapid switching caused by temporary fluctuations.

---

## Configuration

Configuration is stored in ESP32 Non-Volatile Storage (NVS).

The configuration structure is versioned to allow future firmware updates without loading incompatible data.

Configuration is only written after changes have remained unchanged for 30 seconds, minimizing flash wear.

---

## Statistics

The controller records:

* Solar generation
* Total generated energy
* Inverter operating time
* Grid operating time
* Grid warning time(no output on GRID mode)
* Power failure time(no output on INV mode)

Statistics are periodically saved to NVS.

---

## Telegram

The Telegram bot provides:

* Device status
* Battery information
* Solar production
* Power consumption
* Manual mode switching
* Notifications
* OTA updates

---

## Main Loop

To keep loop latency low, heavier tasks are distributed across multiple iterations.

Typical tasks include:

* JK-BMS polling
* JSY-1050 polling
* Telegram processing
* Configuration saving
* Display updates
* Mode control
* Statistics calculation

---

## Future Improvements

* SD card logging
* Historical energy graphs
* UPS mode(switch to INV if no output on GRID)
