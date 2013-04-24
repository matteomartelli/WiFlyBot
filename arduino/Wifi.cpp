/* 
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
*/

#include <WiFlySerial.h>
#include <Streaming.h>

#include "MemoryFree.h"
#include "Types.h"
#include "Consts.h"
#include "Utils.h"

String ssid = "ARDUINOS";
String localIp = "10.42.1.11";
String localPort = "5005";
String netMask = "255.255.0.0";

WiFlySerial wifi(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

WiFiNode endPoints[N_ENDPOINTS];

char buffer[BUFFER_SIZE];

int findNode(char *mac){
	for(int i = 0; i < N_ENDPOINTS; i++){
		if(strcmp(endPoints[i].mac, mac) == 0) 
			return i;
	}
	return -1;
}
	
int findEmpty(){
	for(int i = 0; i < N_ENDPOINTS; i++){
		if(endPoints[i].empty) 
			return i;
	}
	return -1;
}

/* This functions just calls the respective SendCommand function of 
 * the WiFly Serial library. It is needed for using the string objects
 * in a more confortable way. */
void sendCmd(WiFlySerial *wifi, String cmd){
	wifi->SendCommand(&(cmd[0]), ">",buffer, BUFFER_SIZE);
}

void wifiSetup(){
	Serial << F("Arduino mobile wifi") << endl
		<< F("Arduino Rx Pin (connect to WiFly Tx):") << ARDUINO_RX_PIN << endl
		<< F("Arduino Tx Pin (connect to WiFly Rx):") << ARDUINO_TX_PIN << endl
		<< F("RAM: ") << freeMemory();

	wifi.begin();

	Serial << F("Started WiFly") << endl
		<< F("WiFly Lib Version: ") << wifi.getLibraryVersion(buffer, BUFFER_SIZE) << endl
		<< F("Wifi MAC: ") << wifi.getMAC(buffer, BUFFER_SIZE) << endl;

	/* Create the ad-hoc connection */
	//sendCmd(&wifi, "scan");
	sendCmd(&wifi, "set wlan join 4"); //Ad-hoc mode (change to 1 for joining an existing network)
	sendCmd(&wifi, "set wlan ssid "+ssid);
	//sendCmd(&wifi, "set join "+ssid); //For join an existing network
	sendCmd(&wifi, "set wlan chan 1");
	sendCmd(&wifi, "set ip dhcp 0");
	sendCmd(&wifi, "set ip address "+localIp);
	
	sendCmd(&wifi, "set ip netmask "+netMask);
	
	sendCmd(&wifi, "set ip proto 3");
	sendCmd(&wifi, "set ip local "+localPort);
	sendCmd(&wifi, "save");
	sendCmd(&wifi, "reboot");

	Serial << F("Initial WiFi Settings :") << endl  
		<< F("IP: ") << wifi.getIP(buffer, BUFFER_SIZE) << endl
		<< F("Netmask: ") << wifi.getNetMask(buffer, BUFFER_SIZE) << endl
		<< F("Gateway: ") << wifi.getGateway(buffer, BUFFER_SIZE) << endl
		<< F("DNS: ") << wifi.getDNS(buffer, BUFFER_SIZE) << endl
		<< F("battery: ") <<  wifi.getBattery(buffer, BUFFER_SIZE) << endl;
	memset (buffer,'\0',BUFFER_SIZE);

	// close any open connections
	wifi.closeConnection();
	
	Serial.println(F("Command mode exit"));
	
	wifi.exitCommandMode();
	
	if(wifi.isInCommandMode())
		errorPanic(F("Can't exit from command mode"));
	
	Serial << F("Listening on port ") << localPort << endl;
}
