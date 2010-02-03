/*
 *  Tracker.c
 *  srv1-bot
 *
 *  Created by soulh4x on 1/17/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdint.h>
#include <math.h>
#include "Tracker.h"


#define PI 3.14159265

//struct _SRV1_Coordinate {
//	double x;
//	double y;
//};

SRV1_Coordinate * newCoordinate( double x, double y ) {
	SRV1_Coordinate * coor = (SRV1_Coordinate *) malloc( sizeof( SRV1_Coordinate ) );
	coor->x = x;
	coor->y = y;
	return coor;
}

double lineLength( SRV1_Coordinate * c ) {
	return sqrt( ( pow( c->x, 2.0 ) + pow( c->y, 2.0 ) ) );
}

double lineAngleFromY( SRV1_Coordinate * c ) {
	return ( atan2( c->x, c->y )*( 180/PI ) );
}

double lineAngleFromX( SRV1_Coordinate * c ) {
	return 90 - ( atan2( c->x, c->y )*( 180/PI ) );
}

void goFidoo( SRV1_connection * connection, SRV1_Coordinate * location ) {
	
	double angleFromY = lineAngleFromY( location );
	
	// The dot is in quadrant II so angle is towards the left
	// to turn 20 degrees to the left: 0x4D40C014 = "M64,-64,20"
	// we will vary the time based on the estimation that at this speed it will turn 1 degree per millisecond
	if( location->x < 0 ) {
		SRV1_sendRawMove( connection, (int8_t)(64), (int8_t)(-64), (uint8_t) angleFromY );
	} 
	// The dot is quadrant I so angle is towards the right
	// to turn 20 degrees to the right: 0x4DC04014 = "M-64,64,20"
	// we will vary the time based on the estimation that at this speed it will turn 1 degree per millisecond
	else {
		SRV1_sendRawMove( connection, (int8_t)(-64), (int8_t)(64), (uint8_t) angleFromY );
	}
}

