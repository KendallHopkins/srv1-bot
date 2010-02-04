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

SRV1_Coordinate * newCoordinate( double x, double y ) {
	SRV1_Coordinate * coor = (SRV1_Coordinate *) malloc( sizeof( SRV1_Coordinate ) );
	coor->x = x;
	coor->y = y;
	return coor;
}

int8_t approacher( double y ) {
	int8_t speed = ( pow( pow( y, 3.0 ), .1 ) + 2 ) * 10;
	return  speed > 100 ? 100:speed;	
}

void goFidoo( SRV1_connection * connection, SRV1_Coordinate * point ) {
	if( point ) {
		//double angleFromY = lineAngleFromY( point );
		if( fabs( point->x ) > 35 ) {
			if( point->x > 0 ) {
				SRV1_sendRawMove( connection, 50, -50, 10 );
			} else {
				SRV1_sendRawMove( connection, -50, 50, 10 );
			}
		} else {
			if( point->y > PROXIMITY_THRESHOLD_Y )
				SRV1_sendRawMove( connection, approacher( point->y ), approacher( point->y ), 10);
		}
	} 
}

