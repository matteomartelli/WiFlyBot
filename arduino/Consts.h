#ifndef CONSTS_H
#define CONSTS_H

/*
arduino		0		1		2		3		4		5		6		7		8		9		10		11		12		13
bees        tx2		rx2		tx1		rx1		4		5		6	 	7		8		9		10		11		12		13
driver										4		1		3		2		5		7		6		
driver										stby 	pwma	ain1	ain2	bin1	pwmb	bin2
*/

/* WIFLY PINS */
#define ARDUINO_RX_PIN 	2
#define ARDUINO_TX_PIN 	3

/* MOTORS DRIVER PINS */
#define DRIVER_STBY	4 //standby
//Motor A
#define DRIVER_PWMA	5 //Speed control 
#define DRIVER_AIN1	6 //Direction
#define DRIVER_AIN2	7 //Direction

//Motor B
#define DRIVER_PWMB	9 //Speed control
#define DRIVER_BIN1	8 //Direction
#define DRIVER_BIN2	10 //Direction

#define BUFFER_SIZE 80
#define IP_BUFFER_SIZE 16
#define MAC_BUFFER_SIZE 16
#define RSSI_BUFFER_SIZE 8

#define MAC_STR_LEN	12  //a0b1c2d3e4f5 (no colons format)
#define MIN_IP_STR_LEN 7 //0.0.0.0 (considering just ipv4)
#define IP_NDOTS	3

#define N_ENDPOINTS 2

/* STEM-NODE FIELDS */
#define N_SECS_HTML_REFRESH 3
#define N_SECS_ROBOT_CHECK 2
#define LB_REQ		50
#define SENSITIVITY 	-83 //from wifly rn171 manual
#define MAX_RSSI 		-10
#define MIN_RSSI		-75
#define MAX_LB  		MAX_RSSI - SENSITIVITY
#define MIN_LB 		MIN_RSSI - SENSITIVITY
#define ATTENUATION 	1




#endif
