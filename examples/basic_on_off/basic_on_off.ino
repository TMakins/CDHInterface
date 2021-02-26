/*
 *	basic_on_off.ino
 *	----------------
 *	Copyright(C) 2021 Toby Makins
 *	toby@makins.org.uk
 *
 *	Intended platform: ESP32 
 * 
 * 	Example sketch demonstrating a very simple on/off style control for a Chinese 
 *	Diesel Heater.
 *
 *	For more info, see the GitHub repository here: https://github.com/TMakins/CDHInterface
 *
 *	This file is part of the CDHInterface library. 
 *	
 * 	This program is free software: you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation, either version 3 of the License, or
 * 	(at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 * 
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <DieselHeater.h>

DieselHeater heater;

void setup() {
  heater.init();
  // IMPORTANT - THESE MAY NOT BE CORRECT FOR YOUR HEATER
  // Change these to your current heater settings
  heater.setMinPumpHz(1.6);
  heater.setMaxPumpHz(5.5);
  heater.setMinFanSpeed(1680);
  heater.setMaxFanSpeed(3500);

  heater.usePumpHzMode();
  heater.setDesiredPumpHz(2.2);
  
  Serial.begin(115200);
  Serial.print("Connecting to heater... ");
  while(!heater.isConnected());
  Serial.println("connected.");
  
  Serial.println("Enter '1' to turn the heater on, and '0' to turn it off");
}

void loop() {
  if(!heater.isConnected()) {
    Serial.println("Disconnected.");
    Serial.print("Attempting to reconnect... ");
    while(!heater.isConnected());
    Serial.println("connected.");
  }
  
  if(Serial.available()) {
    char cmd = Serial.read();
    if(cmd == '1') {
      Serial.println("Turning on");
      heater.turnOn();
    }
    else if(cmd == '0') {
      Serial.println("Turning off");
      heater.turnOff();
    }
  }
}