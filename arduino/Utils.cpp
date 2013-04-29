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

#include <Streaming.h>
#include "Consts.h"

void errorPanic(__FlashStringHelper *err){
	Serial.print(F("ERROR PANIC :"));
	Serial.println(err);
	while(1);
}

void printDebug(__FlashStringHelper *msg){
	Serial.print(F("DEBUG: "));
	Serial.println(msg);
}

/* Check if the ip string is correctly formatted */
bool checkIP(char *ip){
	short int i, dots = 0;
	int len;
	
	if((len = strlen(ip)) < MIN_IP_STR_LEN)
		return false;

	for(i = 0; i < len; i++){
		if (ip[i] == '.')
			/* If the ith character is a dot between two non-dot characters, increment dots counter,
			 * otherwise there's something wrong with the string. The error is checked below */ 
			if(i > 0 && i < len - 1 && ip[i - 1] != '.' && ip[i + 1] != '.')
				dots++;
	}
	if(dots != IP_NDOTS)
		return false;
	else
		return true;
}
