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
	  Timer			   Simon Monk		 https://github.com/JChristensen/Timer
	  and to The Arduino Team.
*/
//Libraries headers
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <WiFlySerial.h>
#include <Timer.h>

//Project headers
#include "MemoryFree.h"
#include "Motors.h"
#include "Consts.h"
#include "Types.h"
#include "Utils.h"
#include "Wifi.h"

Timer t;

/* The MAX Resultant force is the sum of the max forces of the two nodes,
 * in the case that one is repulsive and one attractive */
float maxResultant = 0;
short int maxResultantDir = 0; //Randomly choosen

char mac[MAC_BUFFER_SIZE], ip[IP_BUFFER_SIZE], 
	 rssi[RSSI_BUFFER_SIZE], *current;

char chMisc; 
int endRead, startRead, sharps, commas, count, idx;
bool allEmpty = true;


// Arduino Setup routine. TODO: move this in another file.
void setup() {
	Serial.begin(9600);
	
	wifiSetup();
	resetFields();
	/* TODO: structs initialization */
	for(int i = 0; i < N_ENDPOINTS; i++){
		memset(endPoints[i].ip, '\0', IP_BUFFER_SIZE);
		memset(endPoints[i].mac, '\0', MAC_BUFFER_SIZE);
		endPoints[i].rssi = 1;
		endPoints[i].lb = -1;
		endPoints[i].empty = true;
	}

	/* Assuming that my STEM-NODE is in the middle */
	endPoints[0].position = -1; /* Rear */
	endPoints[1].position = +1; /* Front */
	
	/* Setup the motors pins */
	motorSetup();
	
	t.after(1000*N_SECS_CHECK, checkRobot);
	
	Serial << F("ROBOT FIELDS: ") << endl
		<< F("Requested Link Budget: ") << LB_REQ << endl
		<< F("Node Sensitivity: ") << SENSITIVITY << endl
		<< F("Max LB: ") << MAX_LB << F(" Min LB: ") << MIN_LB << endl;
}
	
	
/* TODO: 
	 * Better a tcp connection when a node enters and when a node leaves?
	 * 3) In another task (timer?) handle the bot movement getting there
	 * 		the here client stored informations.
	 *    Or better handling the packets retreival there? Can I handle WiFly Serial IO interrupt?  */ 

/* Set and reset to zero/null the global variables used in the parsing process */
void resetFields(){
	endRead = startRead = sharps = commas = count = 0;
	memset (ip,'\0',IP_BUFFER_SIZE);
	memset (mac,'\0',MAC_BUFFER_SIZE);
	memset (rssi,'\0',RSSI_BUFFER_SIZE);
}	

void loop() {
	t.update(); //TODO: better move this in a timer interrupt as the while below can run for too much time.. 
	
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
			break;
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
		//TODO: check buffers sizes. If they're wrong ignore the packet
		if((idx = findNode(mac)) != -1){ //mac matches
			if( strcmp(endPoints[idx].ip, ip) != 0) //ip doesn't match
				printDebug(F("Ip has changed")); //TODO: what here?
		}else if ((idx = findEmpty()) == -1){
			printDebug(F("No matches and no empty space")); //TODO: What here?;
			return; //Skip the following: error condition
		}
		
		if(endPoints[idx].empty){
			endPoints[idx].empty = false;
			allEmpty = false;
			
			/* Calculate the max resultant according to the number of nodes connected.
			 * The maxForce direction is pointing to a node choosed randomly (the first checked).
			 * It starts summing the attractive maxForce according to the first node direction,
			 * if other nodes with the same direction are connected it sums again 
			 * the attractive maxForce, if the direction is opposite the repulsive maxForce */  
			if(maxResultantDir == 0){
				/* This is computed only once */
				maxResultantDir = endPoints[idx].position;
			}
			if(maxResultantDir == endPoints[idx].position)
				maxResultant += sqrt(((float)LB_REQ) / ((float)MIN_LB)) - 1; //Attractive
			else
				maxResultant += sqrt(((float)MAX_LB) / ((float)LB_REQ)) - 1; //Repulsive
		}
		
			
		/* If it comes here, it should have found the correct idx */
		memcpy(endPoints[idx].ip, ip, strlen(ip)); 
		memcpy(endPoints[idx].mac, mac, strlen(mac)); 
		endPoints[idx].rssi = atoi(rssi);
		
		/* LINK BUDGET calculation */
		if(endPoints[idx].rssi < SENSITIVITY) 
			endPoints[idx].rssi = SENSITIVITY;
		endPoints[idx].lb = endPoints[idx].rssi - SENSITIVITY;
		if (endPoints[idx].lb > MAX_LB) endPoints[idx].lb = MAX_LB;
		if (endPoints[idx].lb < MIN_LB) endPoints[idx].lb = MIN_LB;

		resetFields();
	}
	
}

void checkRobot(){
	float C = 0.00, R = 0.00, RNorm = 0.00, gamma = 0.00, motionProbability = 0.00, random = 0.00;
	if(!allEmpty){
		Serial << F("_______________________________________________") << endl;
		int nNodes = 0;
		for(int i = 0; i < N_ENDPOINTS; i++){
			
			if(endPoints[i].empty) 
				continue; //Skip the empty nodes
			
			float F = calcForce(i);
			R += F;
			C = max(C, criticality(i));
			
			//TODO: print this as a html page over http
			Serial << F("IDX: ") << i 
					//<< F(" IP: ") << endPoints[i].ip 
					//<< F(" MAC: ") << endPoints[i].mac 
					<< F(" RSSI: ") << endPoints[i].rssi
					<< F(" LB: ") << endPoints[i].lb
					<< F(" FORCE: ") << F
					<< F(" CRITICALITY: ") << criticality(i)
					<< F(" POS: ") << endPoints[i].position << endl;
		}
		
		/* The move probability is proportional to the RNorm and decreases with a high criticality */
		RNorm = fabs(R)/maxResultant;
		gamma = ((float)ATTENUATION) / (1 /* Don't consider criticality for now - C */);
		motionProbability = pow(RNorm, gamma); 
		int r = rand() % 100 + 1;
		random = ((float)r) / 100;
		
		Serial << F("Max Resultant: ") << maxResultant /*<< F("Criticality: ") << C */ << F(" R: ") << R << (" RNorm: " ) << RNorm << endl
			<< F ("Gamma: ") << gamma << F(" Motion Probability: ") << motionProbability << endl
			<< F ("Random: ") << random << endl;
		
		float speed = RNorm*250 + 55;
		
		/* TODO: Calculate R_NORM, p with a choosen ATTENUATION, get a random number, if random < p move otherwise don't move */
		if(random < motionProbability){ /* TODO What is the criticality bound at which I should move ? */
		
			if(R > 0 && speed >= 55){
				/* Move forward */
				Serial << F("moving forward with speed ") << speed << endl;
				move(1, speed - 20, 1); /* TODO: check temp motor defect */
				move(2, speed, 1);
			}else if (R < 0 && speed >= 55){
				/* Move backward */
				Serial << F("moving backward with speed ") << speed << endl;
				move(1, speed - 20, 0); /* TODO: check temp motor defect */
				move(2, speed, 0);
			}else{
				Serial << F("Stop not enough speed") << endl;
				stop();
			}
		}else{
			Serial << F("Stop, too less probability") << endl;
			stop();
		}  
		Serial << F("_______________________________________________") << endl << endl;
	}
	t.after(1000*N_SECS_CHECK, checkRobot);
}

float calcForce(int idx){
	short int forceDirection;

	if (endPoints[idx].lb > LB_REQ){
	
		/* The force is REPULSIVE in this case, thus the direction of the force
		 * must be opposite to the node position */
		forceDirection = -(endPoints[idx].position);
	}else if (endPoints[idx].lb < LB_REQ){
		
		/* The force is ATTRACTIVE in this case, thus the direction of the force
		 * must be opposite to the node position */
		forceDirection = endPoints[idx].position;
	}else{
		/* If the endPoints[idx].lb is equal to LB_REQ the force is null */ 
		forceDirection = 0;
	}
	float force = (sqrt(( (float) max(endPoints[idx].lb, LB_REQ) ) / ( (float)min(endPoints[idx].lb, LB_REQ))) -1) * ((float)forceDirection);
	
	return force;
}

float criticality(int idx){
	return 1 - ((float)min(endPoints[idx].lb, LB_REQ))/((float)LB_REQ);
}
