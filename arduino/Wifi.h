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
/* Wifi end point data are stored here. */
#ifndef WIFLY_H
#define WIFLY_H

#include <WiFlySerial.h>

#include "Types.h"
#include "Consts.h"


extern WiFiNode endPoints[N_ENDPOINTS]; //An array of WiFiNodes
extern WiFlySerial wifi;

extern char buffer[BUFFER_SIZE];
 
int findNode(char *mac);
int findEmpty();
void wifiSetup();
void sendCmd(WiFlySerial *wifi, String cmd);
void wifiSetup();

#endif
