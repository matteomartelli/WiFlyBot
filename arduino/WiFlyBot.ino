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
float C = 0.00, R = 0.00, RNorm = 0.00, gamma = 0.00, motionProbability = 0.00, randNum = 0.00, speed = 0.00;
short int lastMove = -1;
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
		endPoints[i].force = 0;
		endPoints[i].empty = true;
	}

	/* Assuming that my STEM-NODE is in the middle */
	endPoints[0].position = -1; /* Rear */
	endPoints[1].position = +1; /* Front */
	
	/* Setup the motors pins */
	motorSetup();
	
	t.after(1000*N_SECS_ROBOT_CHECK, checkRobot);
	
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

int read_line( char *line ){
  int pos = 0;
  char c = '\0';
  while( c != '\n' ){
    if( wifi.available() ){
      c = wifi.read();
      line[pos++] = c;
    }
  }
  line[pos] = '\0';
  return pos;
}


char *cmdOpen = "OPEN*";
char *cmdClose = "CLOS*";
bool enterParse = false;
char str[8];
void loop() {
	t.update(); //TODO: better move this in a timer interrupt as the while below can run for too much time.. 
	int i = 0;
	memset(str, '\0', 8);
	while (wifi.available() && ((chMisc = wifi.read()) > -1)) {
		
		/* PKT FORMAT: ###IP,MAC,RSSI; */
		//Serial << chMisc;
		if(enterParse){
			if(i < 5){
				Serial << F("B") << i;
				str[i++] = chMisc;
				if (i == 5){Serial << endl; break;}
				else continue;
			}
		}

		if(chMisc == '*'){
			Serial << F("E") << endl;
			enterParse = true;
			continue;
		}

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
	#if 1
	if(enterParse){
		enterParse = false;
		Serial << F("STR: x") << str << F("x ")<<endl;
		if(strcmp(str, cmdOpen) == 0){
			//Open
			sendCmd(&wifi, "set ip proto 2");
			sendCmd(&wifi, "set comm idle "+N_SECS_ROBOT_CHECK);
			wifi.exitCommandMode();
			
			Serial.println("Connection opened");
			
			//char cmd[128], line[128];
			
			//while(wifi.available());
			//read_line( cmd );
			//Serial.println( cmd );
			/*while( read_line( line ) > 1 ){
				Serial.print( line );
				if( line[0] == '\r' )
					break;
			}*/
			/*char *data = "<html><body>welcome!</body></html>\n";*/
			wifi.print( "\r\nHTTP/1.1 200 OK\r\n");
			wifi.print( "Content-Type: text/html\r\n" );
			wifi.print( "Content-Length: " );
			int len = 100+(300*N_ENDPOINTS);
			
			wifi.print( 680 ); //TODO: how much here?
			wifi.print("\r\n");
			wifi.print( "Connection: Close\r\n" );
			wifi.print( "\r\n" );
			
			//wifi << F("<HTML>ciao");
			
			wifi << F("<HTML><META HTTP-EQUIV=\"REFRESH\" CONTENT=\"") << N_SECS_HTML_REFRESH << F("\">")
					<< F("<P>LB Required: ") << LB_REQ
					<< F(" | Sensitivity: ") << SENSITIVITY
					<< F(" | MAX_LB: ") << MAX_LB
					<< F(" | MIN_LB: ") << MIN_LB
					<< F("</P>") << endl;

			/*char forceStr[5]; TODO: store the force in the WiflyNode
			itoa(F, forceStr, 5);*/
			//if (!allEmpty){
				for(int i = 0; i < N_ENDPOINTS; i++){
					/*if(endPoints[i].empty) 
						continue;*/
					
					wifi << F("<TABLE BORDER=\"1\"><TR><TH>ID</TH><TH>IP</TH><TH>MAC</TH><TH>RSSI</TH><TH>LB</TH><TH>FORCE</TH><TH>POS</TH></TR>")
							<< F("<TR><TD>") << i 
							<< F("</TD><TD>") << endPoints[i].ip
							<< F("</TD><TD>") << endPoints[i].mac
							<< F("</TD><TD>") << endPoints[i].rssi 
							<< F("</TD><TD>") << endPoints[i].lb
							<< F("</TD><TD>") << endPoints[i].force
							<< F("</TD><TD>") << endPoints[i].position
							<< F("</TR></TABLE>") << endl;
				}
				#if 1
				wifi << F("<P> Max Resultant: ") << maxResultant /*<< F("Criticality: ") << C */ << F(" | R: ") << R << (" | RNorm: " ) << RNorm 
					<< F ("</P><P>Gamma: ") << gamma << F(" | Motion Probability: ") << motionProbability << endl
					<< F (" | Random: ") << randNum;
				if(lastMove > -1){
					if(lastMove == 1)
						wifi << F(" | Last Move: FORWARD");
					if(lastMove == 0)
						wifi << F(" | Last Move: BACKWARD");
					else wifi << F("MOVE ERROR");
					
					wifi << F(" | speed: ") << speed;
				}else
					wifi << F(" | Last Move: STATIONARY");
				wifi << F("</P>"); 
				#endif
			//}
			
			wifi << "</HTML>";
			/*for(int i = 0; i < 100; i++){ //TODO: Very ugly workaround...find a better way!!!
				wifi.write( " " );
				wifi.write(byte(0));
			}*/
			
			wifi.write(byte(0));
			
			sendCmd(&wifi, "close");
			sendCmd(&wifi, "set ip proto 3");
			wifi.exitCommandMode();
		}
		else if(strcmp(str, cmdClose) == 0){
			//Close
			Serial.println("Connection closed");
			
		} 
	}
	#endif
	
	if(endRead){
		short int rssint; //integer rssi 
		if( (strlen(mac) == MAC_STR_LEN) && (checkIP(ip)) && ((rssint = atoi(rssi)) < 1) ){
		
			if((idx = findNode(mac)) != -1){ //mac matches
				if( strcmp(endPoints[idx].ip, ip) != 0) //ip doesn't match
					printDebug(F("Ip has changed")); //TODO: what here?
			}else if ((idx = findEmpty()) == -1){
				printDebug(F("No matches and no empty space")); //TODO: What here?;
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
			
			
			Serial << F("IP: ") << ip << F(" | RSSI: ") << rssi << endl;
				
			/* If it comes here, it should have found the correct idx */
			memcpy(endPoints[idx].ip, ip, strlen(ip)); 
			memcpy(endPoints[idx].mac, mac, strlen(mac)); 
			endPoints[idx].rssi = rssint;
			
			/* LINK BUDGET calculation */
			if(endPoints[idx].rssi < SENSITIVITY) 
				endPoints[idx].rssi = SENSITIVITY;
			endPoints[idx].lb = endPoints[idx].rssi - SENSITIVITY;
			if (endPoints[idx].lb > MAX_LB) endPoints[idx].lb = MAX_LB;
			if (endPoints[idx].lb < MIN_LB) endPoints[idx].lb = MIN_LB;

			resetFields();
		}
	}
	
}

//float C = 0.00, R = 0.00, RNorm = 0.00, gamma = 0.00, motionProbability = 0.00, randNum = 0.00, speed = 0.00;
//short int lastMove = -1;
void checkRobot(){ //TODO: Move this in a timer interrupt routine
	C = 0.00, R = 0.00, RNorm = 0.00, gamma = 0.00, motionProbability = 0.00, randNum = 0.00, speed = 0.00;
	if(!allEmpty){
		Serial << F("_______________________________________________") << endl;
		
		int nNodes = 0;
		for(int i = 0; i < N_ENDPOINTS; i++){
			
			if(endPoints[i].empty) 
				continue; //Skip the empty nodes
			
			endPoints[i].force = calcForce(i);
			R += endPoints[i].force;
			//C = max(C, criticality(i)); TODO: not used for now
			
			Serial << F("IP: ") << endPoints[i].ip << F(" | RSSI: ") << endPoints[i].rssi << endl;
			
			
		}
		/* The move probability is proportional to the RNorm and decreases with a high criticality */
		RNorm = fabs(R)/maxResultant;
		gamma = ((float)ATTENUATION) / (1 /* Don't consider criticality for now - C */);
		motionProbability = pow(RNorm, gamma); 
		int r = rand() % 100 + 1;
		randNum = ((float)r) / 100;
		
		Serial << F("Max Resultant: ") << maxResultant /*<< F("Criticality: ") << C */ << F(" R: ") << R << (" RNorm: " ) << RNorm << endl
			<< F ("Gamma: ") << gamma << F(" Motion Probability: ") << motionProbability << endl
			<< F ("Random: ") << randNum << endl;
		
		speed = RNorm*200 + 55;
		
		/* TODO: Calculate R_NORM, p with a choosen ATTENUATION, get a random number, if random < p move otherwise don't move */
		if(randNum < motionProbability){ /* TODO What is the criticality bound at which I should move ? */
		
			if(R > 0 && speed > 55){
				/* Move forward */
				lastMove = 1;
				Serial << F("moving forward with speed ") << speed << endl;
				move(1, speed, 1); /* TODO: check temp motor defect */
				move(2, speed, 1);
			}else if (R < 0 && speed > 55){
				/* Move backward */
				lastMove = 0;
				Serial << F("moving backward with speed ") << speed << endl;
				move(1, speed, 0); /* TODO: check temp motor defect */
				move(2, speed, 0);
			}else{
				Serial << F("Stop not enough speed") << endl;
				stop();
			}
		}else{
			Serial << F("Stop, too less probability") << endl;
			lastMove = -1;
			stop();
		}  
		
		
		Serial << F("_______________________________________________") << endl << endl;
	}
	t.after(1000*N_SECS_ROBOT_CHECK, checkRobot);
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
