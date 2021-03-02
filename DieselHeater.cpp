/*
 *  DieselHeater.cpp
 *  ----------------
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

#include "dieselheater.h"

#define HEATER_TWI_ADDR 0x32U

// Write registers
#define STATE_REG_ADDR              0
#define MODE_REG_ADDR               1
#define CURRENT_TEMP_REG_ADDR       2
#define DESIRED_TEMP_REG_ADDR       3
#define DESIRED_HZ_REG_ADDR         4
#define ALTITUDE_REG_ADDR           5  // 5, 6
#define MIN_PUMP_HZ_REG_ADDR        7
#define MAX_PUMP_HZ_REG_ADDR        8
#define MIN_FAN_SPEED_REG_ADDR      9  // 9, 10
#define MAX_FAN_SPEED_REG_ADDR      11 // 11, 12
#define OP_VOLTAGE_REG_ADDR         13
#define FAN_MAGS_REG_ADDR           14
#define GP_POWER_REG_ADDR           15
// Read registers
#define HTR_STATE_REG_ADDR          16
#define RUN_STATE_REG_ADDR          17
#define SUPPLY_V_REG_ADDR           18 // 18, 19
#define FAN_SPEED_REG_ADDR          20 // 20, 21
#define FAN_V_REG_ADDR              22 // 22, 23
#define BODY_TEMP_REG_ADDR          24 // 24, 25
#define CURRENT_GP_V_REG_ADDR       26 // 26, 27
#define CURRENT_GP_C_REG_ADDR       28 // 28, 29
#define CURRENT_PUMP_HZ_REG_ADDR    30
#define REQUESTED_PUMP_HZ_REG_ADDR  31
#define ERROR_CODE_REG_ADDR         32
#define LAST_ERROR_REG_ADDR         33
#define CONFIG_A_REG_ADDR           34
#define STATUS_A_REG_ADDR           35

// Status A register, currently just has a ready bit
#define STATUS_A_READY  0 // bit position for ready bit in status register

// Config A register
#define CONFIG_A_UPDATE_SETTINGS    0 // bit position for enabling/disabling setting config (see method below)

const char* run_state_strs[] = {
    "Stopped",
    "Starting",
    "Igniting",
    "Waiting",
    "Ramping up",
    "Running",
    "Ramping down",
    "Stopping",
    "Cooldown"
};

const char* error_state_strs[] = {
    "No error",
    "Supply voltage too low",
    "Supply voltage too high",
    "Glow plug failure",
    "Pump failure",
    "Overheated",
    "Motor failure",
    "Controller comms. error", 
    "Flame out",
    "Temperature sensor failure",
    "Disconnected"
};

DieselHeater::DieselHeater()
{
}

void DieselHeater::init(TwoWire *wire)
{
    // Wait long enough to be certain the interface board is running
    delay(100);
    // Init I2C
    _twi = wire;
    _twi->begin();
    // Wait for status bit just to make sure
    while(!interfaceReady());
}

/*
 *  Control methods
 */

void DieselHeater::turnOn()
{
    _writeTwiRegU8(STATE_REG_ADDR, 1); // 1 = on
}

void DieselHeater::turnOff()
{
    _writeTwiRegU8(STATE_REG_ADDR, 0); // 0 = off
}

void DieselHeater::startPrime()
{
    _writeTwiRegU8(STATE_REG_ADDR, 2); // 2 = prime
}

void DieselHeater::stopPrime()
{
    turnOff(); // stops priming
}

void DieselHeater::usePumpHzMode()
{
    _writeTwiRegU8(MODE_REG_ADDR, 0); // 0 = hz mode
}

void DieselHeater::useThermostatMode()
{
    _writeTwiRegU8(MODE_REG_ADDR, 1); // 1 = thermostat mode
}

void DieselHeater::setDesiredPumpHz(float hz)
{
    _writeTwiRegU8(DESIRED_HZ_REG_ADDR, (uint8_t)(hz * 10)); // 0.1hz / digit
}

void DieselHeater::setDesiredTemp(uint8_t tempC)
{
    _writeTwiRegU8(DESIRED_TEMP_REG_ADDR, tempC);
}

void DieselHeater::setCurrentTemp(uint8_t tempC)
{
    _writeTwiRegU8(CURRENT_TEMP_REG_ADDR, tempC);
}

// Note: most (all?) heaters ignore this, it's here for completeness
void DieselHeater::setAltitude(uint16_t altitudeM)
{
    _writeTwiRegU16(ALTITUDE_REG_ADDR, altitudeM);
}

/*
 * Config methods
 */         
    
void DieselHeater::setMinPumpHz(float minHz)
{
    _writeTwiRegU8(MIN_PUMP_HZ_REG_ADDR, (uint8_t)(minHz * 10)); // 0.1hz / digit
}

void DieselHeater::setMaxPumpHz(float maxHz)
{
    _writeTwiRegU8(MAX_PUMP_HZ_REG_ADDR, (uint8_t)(maxHz * 10)); // 0.1hz / digit
}

void DieselHeater::setMinFanSpeed(uint8_t minRPM)
{
    _writeTwiRegU16(MIN_FAN_SPEED_REG_ADDR, minRPM);
}

void DieselHeater::setMaxFanSpeed(uint8_t maxRPM)
{
    _writeTwiRegU16(MAX_FAN_SPEED_REG_ADDR, maxRPM);
}

void DieselHeater::setOpVoltage12V()
{
    _writeTwiRegU8(OP_VOLTAGE_REG_ADDR, 120); // 0.1V / digit
}

void DieselHeater::setOpVoltage24V()
{
    _writeTwiRegU8(OP_VOLTAGE_REG_ADDR, 240); // 0.1V / digit
}

void DieselHeater::setFanMagnets1()
{
    _writeTwiRegU8(FAN_MAGS_REG_ADDR, 1);
}

void DieselHeater::setFanMagnets2()
{
    _writeTwiRegU8(FAN_MAGS_REG_ADDR, 2);
}

void DieselHeater::setGlowPlugPower(uint8_t power)
{
    // Correct limits, should be 1 - 6
    if(power > 6)
        power = 6;
    if(power < 1)
        power = 1;
        
    
    _writeTwiRegU8(GP_POWER_REG_ADDR, power);
}

uint8_t DieselHeater::getRequestedState() 
{
    return _readTwiRegU16(STATE_REG_ADDR);
}

float DieselHeater::getMinPumpHz()
{
    uint8_t res = _readTwiRegU8(MIN_PUMP_HZ_REG_ADDR);
    return res / 10.0; // 0.1Hz / digit
}

float DieselHeater::getMaxPumpHz()
{
    uint8_t res = _readTwiRegU8(MAX_PUMP_HZ_REG_ADDR);
    return res / 10.0; // 0.1Hz / digit
}

uint8_t DieselHeater::getMinFanSpeed()
{
    return _readTwiRegU16(MIN_FAN_SPEED_REG_ADDR);
}

uint8_t DieselHeater::getMaxFanSpeed()
{
    return _readTwiRegU16(MAX_FAN_SPEED_REG_ADDR);
}

float DieselHeater::getOpVoltage()
{
    uint8_t res = _readTwiRegU8(OP_VOLTAGE_REG_ADDR);
    return res / 10.0; // 0.1V / digit
}

uint8_t DieselHeater::getFanMagnets()
{
    return _readTwiRegU8(FAN_MAGS_REG_ADDR);
}

uint8_t DieselHeater::getGlowPlugPower()
{
    return _readTwiRegU8(GP_POWER_REG_ADDR);
}

// Causes the CDH interface to mimic a rotary controller, and hence any settings (pump and fan) are ignored
// This is handy if you don't want to temporarily use the CDH interface without overwriting your current settings
void DieselHeater::disableSettingsUpdates()
{
    uint8_t currentReg = _readTwiRegU8(CONFIG_A_REG_ADDR);
    _writeTwiRegU16(CONFIG_A_REG_ADDR, currentReg | (1 << CONFIG_A_UPDATE_SETTINGS));
}

// Causes the CDH interface to mimic a rotary controller, and hence any settings (pump and fan) are ignored
// This is handy if you don't want to temporarily use the CDH interface without overwriting your current settings
void DieselHeater::enableSettingsUpdates()
{
    uint8_t currentReg = _readTwiRegU8(CONFIG_A_REG_ADDR);
    _writeTwiRegU16(CONFIG_A_REG_ADDR, currentReg & ~(1 << CONFIG_A_UPDATE_SETTINGS));
}

/*
 * Read methods
 */
 
uint8_t DieselHeater::getHtrState()
 {
    return _readTwiRegU8(HTR_STATE_REG_ADDR);
 }

bool DieselHeater::isOn()
{
    return (getHtrState() == 2);
}
 
bool DieselHeater::isConnected()
{
    return (getHtrState() != 0);
}

run_state_t DieselHeater::getRunState()
{
    uint8_t res = _readTwiRegU8(RUN_STATE_REG_ADDR);
    return (run_state_t)res;
}

const char *DieselHeater::getRunStateDesc()
{
    run_state_t state = getRunState();
    // Shouldn't be more than 8, but just in case
    if(state > COOLDOWN)
        state = COOLDOWN;
    
    return(run_state_strs[state]);
}

float DieselHeater::getSupplyVoltage()
{
    uint16_t res = _readTwiRegU16(SUPPLY_V_REG_ADDR);
    return res / 10.0; // 0.1V / digit
}

float DieselHeater::getFanVoltage()
{
    uint16_t res = _readTwiRegU16(FAN_V_REG_ADDR);
    return res / 10.0; // 0.1V / digit
}

uint16_t DieselHeater::getFanSpeed()
{
    return _readTwiRegU16(FAN_SPEED_REG_ADDR);
}

int16_t DieselHeater::getHeatExchangerTemp()
{
    return _readTwiRegS16(BODY_TEMP_REG_ADDR);
}

float DieselHeater::getGlowPlugVoltage()
{
    uint16_t res = _readTwiRegU16(CURRENT_GP_V_REG_ADDR);
    return res / 10.0; // 0.1V / digit
}

float DieselHeater::getGlowPlugCurrent()
{
    uint16_t res = _readTwiRegU16(CURRENT_GP_C_REG_ADDR);
    return res / 100.0; // A (10mA / digit)
}

float DieselHeater::getCurrentPumpHz()
{
    uint8_t res = _readTwiRegU8(CURRENT_PUMP_HZ_REG_ADDR);
    return res / 10.0; // 0.1Hz / digit
}

float DieselHeater::getRequestedPumpHz()
{
    uint8_t res = _readTwiRegU8(REQUESTED_PUMP_HZ_REG_ADDR);
    return res / 10.0; // 0.1Hz / digit
}

bool DieselHeater::hasError()
{
    return (getErrorState() != NO_ERROR);
}

error_state_t DieselHeater::getErrorState()
{
    if(!isConnected())
        return DISCONNECTED; // additional state for when we can't communicate with the heater
        
    uint8_t res = _readTwiRegU8(ERROR_CODE_REG_ADDR);
    
    // only return integer if error state (0 and 1 are running states)
    if(res < 2)
        return NO_ERROR;
    
    return (error_state_t)(res - 1);
}

const char *DieselHeater::getErrorDesc()
{
    error_state_t state = getErrorState();
    // Shouldn't be more than 10, but just in case
    if(state > DISCONNECTED)
        state = DISCONNECTED;
    
    return(error_state_strs[state]);
}

/*
 * Other methods
 */
 
uint8_t DieselHeater::interfaceReady()
{
    return _readTwiRegU8(STATUS_A_REG_ADDR) & (1 << STATUS_A_READY);
}

/*
 *  Helper methods
 */

void DieselHeater::_writeTwiRegU8(uint8_t addr, uint8_t data)
{
    _twi->beginTransmission((uint8_t)HEATER_TWI_ADDR);
    _twi->write(addr);
    _twi->write(data);
    _twi->endTransmission();
}

void DieselHeater::_writeTwiRegU16(uint8_t addr, uint16_t data)
{
    _twi->beginTransmission((uint8_t)HEATER_TWI_ADDR);
    _twi->write(addr);
    _twi->write(data & 0xFF);
    _twi->write((data >> 8) & 0xFF);
    _twi->endTransmission();
}

uint8_t DieselHeater::_readTwiRegU8(uint8_t addr)
{
    _twi->beginTransmission((uint8_t)HEATER_TWI_ADDR);
    _twi->write(addr);
    _twi->endTransmission(false); // repeated start
    _twi->requestFrom((uint8_t)HEATER_TWI_ADDR, (uint8_t)1, (uint8_t)true); // request + stop
    
    return _twi->read();
}

uint16_t DieselHeater::_readTwiRegU16(uint8_t addr)
{
    _twi->beginTransmission((uint8_t)HEATER_TWI_ADDR);
    _twi->write(addr);
    _twi->endTransmission(false); // repeated start
    _twi->requestFrom((uint8_t)HEATER_TWI_ADDR, (uint8_t)2, (uint8_t)true); // request + stop
    uint8_t lsb = _twi->read();
    uint8_t msb = _twi->read();
    return (uint16_t)(msb << 8) | lsb;
}

int16_t DieselHeater::_readTwiRegS16(uint8_t addr)
{
    _twi->beginTransmission((uint8_t)HEATER_TWI_ADDR);
    _twi->write(addr);
    _twi->endTransmission(false); // repeated start
    _twi->requestFrom((uint8_t)HEATER_TWI_ADDR, (uint8_t)2, (uint8_t)true); // request + stop
    uint8_t lsb = _twi->read();
    uint8_t msb = _twi->read();
    return (int16_t)(msb << 8) | lsb;
}