# DPS310 Usage Example

## Unit Testing

```cpp
// -*- coding:utf-8-unix -*-
/**
 * @file   Unit_DPS310.cpp
 * @brief  Using Infineon DPS310 barometric pressure sensor
 *
 * @copyright (C) 2024 Mono Wireless Inc. All Rights Reserved.
 * Released under MW-OSSLA-*J,*E (MONO WIRELESS OPEN SOURCE SOFTWARE LICENSE AGREEMENT).
 */

#include <TWELITE>
#include "Act_props/DPS310.hpp"

const float sea_level_pressure = 1023.3f;

DPS310 dps310;

void setup() {
    dps310.setup(
        DPS310::Address::PRIMARY,
        DPS310::Settings(DPS310::Settings::Presets::LOW_POWER_WEATHER_STATION));
    Serial << "DPS310 Unit sample (press m to measure, 1-3 to apply preset)"
           << mwx::crlf;
}

void begin() {
    dps310.begin();
}

void loop() {
    dps310.update();

    if (Serial.available()) {
        int c = Serial.read();
        if (c >= 0) {
            Serial << crlf << '[' << char_t(c) << '/' << int(millis() & 0xFFFF) << "] ";
            switch (c) {
            case 'm': {
                if (not dps310.request()) {
                    Serial << dps310.getErrorMessage();
                } else {
                    Serial << "Requested";
                }
                break;
            }
            case '1': {
                dps310.end();
                dps310.setSettings(DPS310::Settings(
                    DPS310::Settings::Presets::LOW_POWER_WEATHER_STATION));
                dps310.begin();
                Serial << "Applied preset: Weather Station (Low power)";
                break;
            }
            case '2': {
                dps310.end();
                dps310.setSettings(DPS310::Settings(
                    DPS310::Settings::Presets::STANDARD_PRECISION_INDOOR_NAVIGATION));
                dps310.begin();
                Serial << "Applied preset: Indoor navigation (Standard precision)";
                break;
            }
            case '3': {
                dps310.end();
                dps310.setSettings(
                    DPS310::Settings(DPS310::Settings::Presets::HIGH_PRECISION_SPORTS));
                dps310.begin();
                Serial << "Applied preset: Sports (High precision, high rate)";
                break;
            }
            default: Serial << "Unknown command"; break;
            }
        }
    }

    if (dps310.available()) {
        float temperature, pressure;
        if (not dps310.read(&temperature, &pressure)) {
            Serial << crlf << '[' << int(millis() & 0xFFFF) << "] ";
            Serial << dps310.getErrorMessage();
        } else {
            float altitude = dps310.calcAltitude(sea_level_pressure);
            Serial << crlf << '[' << int(millis() & 0xFFFF) << "] ";
            Serial << "Read";
            Serial
                << crlf
                << format("%.2f°C, %.2fhPa (%.2fm)", temperature, pressure, altitude);
        }
    }
}
```