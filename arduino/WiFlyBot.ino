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
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <WiFlySerial.h>
#include "MemoryFree.h"

#define ARDUINO_RX_PIN 7
#define ARDUINO_TX_PIN 8
#define BUFFER_SIZE 80

#define IP_BUFFER_SIZE 16
#define MAC_BUFFER_SIZE 16
#define RSSI_BUFFER_SIZE 8

#define N_ENDPOINTS 2

String ssid = "ARDUINOS";
String localIp = "10.42.1.11";
String localPort = "5005";
String netMask = "255.255.0.0";

char buffer[BUFFER_SIZE],mac[MAC_BUFFER_SIZE], ip[IP_BUFFER_SIZE], 
	 rssi[RSSI_BUFFER_SIZE], *current;

char chMisc; 
int endRead, startRead, sharps, commas, count, idx;
int distance = -1;

/* Data of a wifi end point are stored here. */
struct WiFiNode{
	char ip[IP_BUFFER_SIZE]; 
	char mac[MAC_BUFFER_SIZE];
	int rssi;
	bool empty;
};

WiFiNode endPoints[N_ENDPOINTS]; //An array of WiFiNodes

WiFlySerial wifi(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

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

/* Set and reset to zero/null the global variables used in the parsing process */
void resetFields(){
	endRead = startRead = sharps = commas = count = 0;
	memset (ip,'\0',IP_BUFFER_SIZE);
	memset (mac,'\0',MAC_BUFFER_SIZE);
	memset (rssi,'\0',RSSI_BUFFER_SIZE);
}	

/* This functions just calls the respective SendCommand function of 
 * the WiFly Serial library. It is needed for using the string objects
 * in a more confortable way. */
void sendCmd(WiFlySerial *wifi, String cmd){
	wifi->SendCommand(&(cmd[0]), ">",buffer, BUFFER_SIZE);
}

/*void updateWiFiNode(struct WiFiNode *node){
	Serial << F("IP: ") << node->ip << F(" MAC: ") << node->mac << F(" RSSI: ") << node->rssi << endl;
}*/

void errorPanic(__FlashStringHelper *err){
	Serial.print(F("ERROR PANIC :"));
	Serial.println(err);
	while(1);
}

void printDebug(__FlashStringHelper *msg){
	Serial.print(F("DEBUG: "));
	Serial.println(msg);
}

int string2bytes(char *str, unsigned char *bytes, int nbytes){
	char *pos = str;
	int count = 0;
	for(count = 0; count < nbytes; count++) {
		sscanf(pos, "%2hhx", &bytes[count]);
		pos += 2;
	}
	return 0;
}

// Arduino Setup routine. TODO: move this in another file.
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
		<< F("RSSI: ") << wifi.getRSSI(buffer, BUFFER_SIZE) << endl
		<< F("battery: ") <<  wifi.getBattery(buffer, BUFFER_SIZE) << endl;
	memset (buffer,'\0',BUFFER_SIZE);

	// close any open connections
	wifi.closeConnection();
	
	Serial.println(F("Command mode exit"));
	
	wifi.exitCommandMode();
	
	if(wifi.isInCommandMode())
		errorPanic(F("Can't exit from command mode"));
	
	Serial << F("Listening on port ") << localPort << endl;
	
	/* TODO: structs initialization */
	for(int i = 0; i < N_ENDPOINTS; i++){
		memset(endPoints[i].ip, '\0', IP_BUFFER_SIZE);
		memset(endPoints[i].mac, '\0', MAC_BUFFER_SIZE);
		endPoints[i].rssi = -1;
		endPoints[i].empty = true;
	}

	resetFields();
}
	
	
/* TODO: 
	 * Better a tcp connection when a node enters and when a node leaves?
	 * 3) In another task (timer?) handle the bot movement getting there
	 * 		the here client stored informations.
	 *    Or better handling the packets retreival there? Can I handle WiFly Serial IO interrupt?  */ 

	
void loop() {
	while (wifi.available() && ((chMisc = wifi.read()) > -1)) {
		
		/* PKT FORMAT: ###IP,MAC,RSSI; */
		
		if(sharps == 3){
			startRead = 1;
			sharps = 0;
		}
		
		if(chMisc == '#')
			sharps++;
		else sharps = 0;
		
		if(chMisc == ';'){
			endRead = 1;
			startRead = 0;
		}
		
		if(startRead == 1){
			if(chMisc == ','){
				commas++;
				if(count){
					current[count] = '\0'; /* Null terminate the string */
					count = 0;
				}
				continue;
			}
			if(commas == 0){
				/* IP CASE */
				if(count >= IP_BUFFER_SIZE)
					continue;
				current = ip;
				ip[count++] = chMisc;
			}
			if(commas == 1){
				/* MAC CASE */
				if(count >= MAC_BUFFER_SIZE)
					continue;
				current = mac;
				mac[count++] = chMisc;
			}
			if(commas == 2){
				/* RSSI CASE */
				if(count >= RSSI_BUFFER_SIZE)
					continue;
				current = rssi;
				rssi[count++] = chMisc;
			}
		}		
	}
	if(endRead){
				
		if((idx = findNode(mac)) != -1){ //mac matches
			if( strcmp(endPoints[idx].ip, ip) != 0) //ip doesn't match
				errorPanic(F("Ip has changed")); //TODO: what here?
		}else if ((idx = findEmpty()) == -1)
			printDebug(F("No matches and no empty space")); //TODO: What here?;
			
		/* If it comes here, it should have found the correct idx */
		memcpy(endPoints[idx].ip, ip, strlen(ip)); 
		memcpy(endPoints[idx].mac, mac, strlen(mac)); 
		endPoints[idx].rssi = -(atoi(rssi));
		endPoints[idx].empty = false;
		
		printDebug(F("STATISTICS"));
		for(int i = 0; i < N_ENDPOINTS; i++){
			Serial << F("IDX: ") << i << F(" IP: ") 
				<< endPoints[i].ip << F(" MAC: ") 
				<< endPoints[i].mac << F(" RSSI: ") 
				<< endPoints[i].rssi << endl;
		}
		
		/* Considering only the two nodes case */
		if(!endPoints[0].empty && !endPoints[1].empty){
			distance = abs(endPoints[0].rssi - endPoints[1].rssi); /* TODO: check this */
			Serial << F("Absolute distance : ") << distance << endl;
		}
		
		resetFields();
	}
	
	/* TODO: move the robot trying to decrease the avarage distance */
	
}


