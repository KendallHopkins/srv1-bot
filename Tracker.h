/*
 *  Tracker.h
 *  srv1-bot
 *
 *  Created by soulh4x on 1/17/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "srv1.h"

#define PROXIMITY_THRESHOLD_Y 50
#define PROXIMITY_THRESHOLD_X 50

SRV1_Coordinate * newCoordinate( double x, double y );

double lineLength( SRV1_Coordinate * c );

double lineAngleFromY( SRV1_Coordinate * c );

double lineAngleFromX( SRV1_Coordinate * c );

void goFidoo( SRV1_connection * c, SRV1_Coordinate * point_loc );

