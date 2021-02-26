/*
 *  DieselHeater.h
 *  --------------
 *  Copyright(C) 2021 Toby Makins
 *  toby@makins.org.uk
 * 
 *  Arduino library for interfacing with Chinese diesel heaters via an I2C device.
 *  For more info, see the GitHub repository here: https://github.com/TMakins/CDHInterface
 *
 *  This file is part of the CDHInterface library. 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DIESELHEATER_H
#define DIESELHEATER_H

#include <Arduino.h>
#include <Wire.h>

typedef enum run_state {
    OFF = 0,
    STARTING,
    IGNITING,
    RETRY_WAIT,
    RAMPING_UP,
    RUNNING,
    RAMPING_DOWN,
    STOPPING,
    COOLDOWN
} run_state_t;

typedef enum error_state {
    NO_ERROR = 0,
    VOLTAGE_LOW,
    VOLTAGE_HIGH,
    GLOW_PLUG_FAILURE,
    PUMP_FAILURE,
    OVERHEAT,
    MOTOR_FAILURE,
    CONNECTION_LOST,
    FLAME_OUT,
    TEMP_SENS_FAILURE, //heaters error codes end here
    DISCONNECTED,
} error_state_t;

class DieselHeater
{
    public:
        DieselHeater();
        void init(TwoWire *wire = &Wire);
        
        // Control methods
        void turnOn();
        void turnOff();
        void startPrime();
        void stopPrime();
        void usePumpHzMode();
        void useThermostatMode();
        void setDesiredPumpHz(float hz);
        void setDesiredTemp(uint8_t tempC);
        void setCurrentTemp(uint8_t tempC);
        void setAltitude(uint16_t altitudeM); // Note: most (all?) heaters ignore this, it's here for completeness

        // Config methods
        void setMinPumpHz(float minHz);
        void setMaxPumpHz(float maxHz);
        void setMinFanSpeed(uint8_t minRPM);
        void setMaxFanSpeed(uint8_t maxRPM);
        void setOpVoltage12V();
        void setOpVoltage24V();
        void setFanMagnets1();
        void setFanMagnets2();
        void setGlowPlugPower(uint8_t power);
        
        float getMinPumpHz();
        float getMaxPumpHz();
        uint8_t getMinFanSpeed();
        uint8_t getMaxFanSpeed();
        float getOpVoltage();
        uint8_t getFanMagnets();
        uint8_t getGlowPlugPower();
        
        void disableSettingsUpdates();
        void enableSettingsUpdates(); // Note: default on power up

        // Read methods
        uint8_t getHtrState();
        bool isOn();
        bool isConnected();
        run_state_t getRunState();
        const char *getRunStateDesc();
        float getSupplyVoltage();
        float getFanVoltage();
        uint16_t getFanSpeed();
        int16_t getHeatExchangerTemp();
        float getGlowPlugVoltage();
        float getGlowPlugCurrent();
        float getCurrentPumpHz();
        float getRequestedPumpHz();
        bool hasError();
        error_state_t getErrorState();
        const char *getErrorDesc();
        
        // Other methods
        uint8_t interfaceReady();
    
    protected:
        TwoWire *_twi;
        void _writeTwiRegU8(uint8_t addr, uint8_t data);
        void _writeTwiRegU16(uint8_t addr, uint16_t data);
        uint8_t _readTwiRegU8(uint8_t addr);
        uint16_t _readTwiRegU16(uint8_t addr);
        int16_t _readTwiRegS16(uint8_t addr);
};

#endif // DIESELHEATER_H