# Secure GSM-Based Thermal Monitoring and Set-Point Control System using LPC2148

## Project Overview

This project implements a Secure GSM-Based Thermal Monitoring and Set-Point Control System using the LPC2148 ARM7 microcontroller. The system continuously monitors temperature and humidity using the DHT11 sensor, compares the values with configurable set points, stores configuration data in EEPROM, and sends GSM SMS alerts with RTC timestamps when abnormal conditions occur. Password-protected SMS commands and keypad-based configuration provide secure local and remote control.

## Features

- Real-time temperature and humidity monitoring
- GSM SMS alert notification
- Password-protected SMS commands
- RTC-based timestamp logging
- EEPROM storage for configuration
- UART interrupt-based GSM communication
- LCD display for live monitoring
- Keypad-based local configuration
- Secure remote monitoring and control

## Hardware Used

- LPC2148 ARM7 Microcontroller
- DHT11 Temperature & Humidity Sensor
- GSM Module
- AT24C256 EEPROM
- 16×2 LCD
- Matrix Keypad
- LEDs

## Software Used

- Embedded C
- Keil uVision
- Flash Magic

## Project Modules

- GSM
- UART
- RTC
- EEPROM
- DHT11
- LCD
- Keypad
- Interrupts
- Password Authentication
- SMS Communication
