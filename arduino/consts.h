#ifndef CONSTS_H
#define CONSTS_H

#define ARDUINO_RX_PIN 6
#define ARDUINO_TX_PIN 7
#define BUFFER_SIZE 80

#define IP_BUFFER_SIZE 16
#define MAC_BUFFER_SIZE 16
#define RSSI_BUFFER_SIZE 8

#define N_ENDPOINTS 2

/* STEM-NODE FIELDS */
#define N_SECS_CHECK 2
#define LB_REQ		50
#define SENSITIVITY 	-83 //from wifly rn171 manual
#define MAX_RSSI 		-5
#define MIN_RSSI		-75
#define MAX_LB  		MAX_RSSI - SENSITIVITY
#define MIN_LB 		MIN_RSSI - SENSITIVITY
#define ATTENUATION 	1

#endif
