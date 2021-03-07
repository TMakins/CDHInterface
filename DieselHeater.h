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
    IGNITION_RETRY,
    RAMPING_UP,
    RUNNING,
    RAMPING_DOWN,
    STOPPING,
    COOLDOWN,
	PREHEAT,
	UKNOWN_RUN_STATE,  // should always be last
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
    TEMP_SENS_FAILURE,
	IGNITION_FAILURE, //heaters standard error codes end here
    DISCONNECTED,
	PUMP_RUNAWAY,
	UNKNOWN_ERROR, // should always be last
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
        void setMinFanSpeed(uint16_t minRPM);
        void setMaxFanSpeed(uint16_t maxRPM);
        void setOpVoltage12V();
        void setOpVoltage24V();
        void setFanMagnets1();
        void setFanMagnets2();
        void setGlowPlugPower(uint8_t power); // range 1-6
		
        uint8_t getRequestedState();
        float getMinPumpHz();
        float getMaxPumpHz();
        uint16_t getMinFanSpeed();
        uint16_t getMaxFanSpeed();
		uint16_t getAltitude();
        float getOpVoltage();
        uint8_t getFanMagnets();
        uint8_t getGlowPlugPower();
        
        void disableSettingsUpdates();
        void enableSettingsUpdates(); // Note: default on power up

        // Read methods
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
        error_state_t getLastErrorState();
        const char *getLastErrorDesc();
		
        uint8_t getHtrState();
		uint8_t getMode();
        
        // Other methods
        uint8_t interfaceReady();
		uint8_t getLastResetReason();
    
    protected:
        TwoWire *_twi;
        void _writeTwiRegU8(uint8_t addr, uint8_t data);
        void _writeTwiRegU16(uint8_t addr, uint16_t data);
        uint8_t _readTwiRegU8(uint8_t addr);
        uint16_t _readTwiRegU16(uint8_t addr);
        int16_t _readTwiRegS16(uint8_t addr);
};

#endif // DIESELHEATER_H