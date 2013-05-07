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
#ifndef TYPES_H
#define TYPES_H

#include "Consts.h"

typedef struct WiFiNode{
	char ip[IP_BUFFER_SIZE]; 
	char mac[MAC_BUFFER_SIZE];
	short int rssi;
	short int lb;
	short int position; /* This will be replaced with GPS coordinates */
	float force;
	bool empty;
}WiFiNode;

#endif
