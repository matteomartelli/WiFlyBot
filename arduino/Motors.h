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
#ifndef MOTORS_H
#define MOTORS_H

/* Set the arduino pins to communicate with the motors controller */
void motorSetup();

/* Move the motors by their id. 
 * Speed and direction (0 clockwise, 1 counter-clockwise) must be specified too. */
void move(int motor, int speed, int direction);

/* Stop the motors */
void stop();

#endif
