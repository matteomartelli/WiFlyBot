/* 
    WiFlyBot is a software for the arduino shield configured as a mobile robot with a wifly module. 
    WiFlyBot allows the robot to find the optimal position between two (or more?) wifi points.
    Copyright (C) 2013 Matteo Martelli.

    This file is part of WiFlyBot.

    WiFlyBot is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    WiFlyBot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WiFlyBot.  If not, see <http://www.gnu.org/licenses/>.
    
    Credits:
	  SoftwareSerial   Mikal Hart        http://arduiniana.org/
	  Time             Michael Margolis  http://www.arduino.cc/playground/uploads/Code/Time.zip
	  WiFly            Roving Networks   www.rovingnetworks.com
	  WiFlySerial	   Tom Waldock		 https://github.com/perezd/arduino-wifly-serial
	  and to The Arduino Team.
*/

#include <Arduino.h>
#include <Time.h>
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <PString.h>
#include <WiFlySerial.h>
#include "MemoryFree.h"

#define ARDUINO_RX_PIN 7
#define ARDUINO_TX_PIN 8
#define BUFFER_SIZE 80

String ssid = "ARDUINOS";
String localIp = "10.42.1.11";
String localPort = "5005";
String netMask = "255.255.0.0";

char buffer[BUFFER_SIZE];

/* The current character.
 * TODO: Is this needed global? */
char chMisc; 

WiFlySerial wifi(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

// Arduino Setup routine. TODO: move this in another file
void setup() {
	Serial.begin(9600);
	Serial << F("Arduino mobile wifi") << endl
		<< F("Arduino Rx Pin (connect to WiFly Tx):") << ARDUINO_RX_PIN << endl
		<< F("Arduino Tx Pin (connect to WiFly Rx):") << ARDUINO_TX_PIN << endl
		<< F("RAM: ") << freeMemory();

	wifi.begin();

	Serial << F("Started WiFly") << endl
		<< F("WiFly Lib Version: ") << wifi.getLibraryVersion(buffer, BUFFER_SIZE) << endl
		<< F("Wifi MAC: ") << wifi.getMAC(buffer, BUFFER_SIZE) << endl;

	/* Create the ad-hoc connection */
	//sendCommandString(&wifi, "scan");
	sendCommandString(&wifi, "set wlan join 4"); //Ad-hoc mode (change to 1 for joining an existing network)
	sendCommandString(&wifi, "set wlan ssid "+ssid);
	//sendCommandString(&wifi, "set join "+ssid); //For join an existing network
	sendCommandString(&wifi, "set wlan chan 1");
	sendCommandString(&wifi, "set ip dhcp 0");
	sendCommandString(&wifi, "set ip address "+localIp);
	
	sendCommandString(&wifi, "set ip netmask "+netMask);
	
	sendCommandString(&wifi, "set ip proto 3");
	sendCommandString(&wifi, "set ip local "+localPort);
	sendCommandString(&wifi, "save");
	sendCommandString(&wifi, "reboot");

	Serial << F("Initial WiFi Settings :") << endl  
		<< F("IP: ") << wifi.getIP(buffer, BUFFER_SIZE) << endl
		<< F("Netmask: ") << wifi.getNetMask(buffer, BUFFER_SIZE) << endl
		<< F("Gateway: ") << wifi.getGateway(buffer, BUFFER_SIZE) << endl
		<< F("DNS: ") << wifi.getDNS(buffer, BUFFER_SIZE) << endl
		<< F("RSSI: ") << wifi.getRSSI(buffer, BUFFER_SIZE) << endl
		<< F("battery: ") <<  wifi.getBattery(buffer, BUFFER_SIZE) << endl;
	memset (buffer,'\0',BUFFER_SIZE);

	// close any open connections
	wifi.closeConnection();
	
	Serial.println(F("Command mode exit"));
	
	wifi.exitCommandMode();
	
	if(wifi.isInCommandMode())
		errorPanic("Can't exit from command mode");
	
	Serial << F("Listening on port ") << localPort << endl;
}

void loop() {
	/*Serial << F("RSSI: ") << wifi.getRSSI(buffer, BUFFER_SIZE) << endl;
	memset (buffer,'\0',BUFFER_SIZE);
	wifi.exitCommandMode();*/
  
	while ((chMisc = wifi.read()) > -1) {
		Serial.print(chMisc);
		/* TODO: 
		 * 1) Verify that multiple inputs don't come in concurrency.
		 * 		How are packets send and read? 
		 * 2) Parse the input string. 
		 * 		Store in some structs the clients IPs, MACs and RSSIs. 
		 * 3) In another task (timer?) handle the bot movement getting there
		 * 		the here client stored informations. */ 
		if(chMisc == '}')
			Serial.print('\n');
	}

}

/* This functions just maps call the respective SendCommand function of 
 * the WiFly Serial library. It is needed just for using the string objects
 * in a more confortable way. */
void sendCommandString(WiFlySerial *wifi, String cmd){
	wifi->SendCommand(&(cmd[0]), ">",buffer, BUFFER_SIZE);
}

void errorPanic(char *err){
	Serial.print(F("ERROR PANIC :"));
	Serial.println(err);
	while(1);
}
