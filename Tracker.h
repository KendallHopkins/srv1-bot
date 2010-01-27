/*
 *  Tracker.h
 *  srv1-bot
 *
 *  Created by soulh4x on 1/17/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "srv1.h"

typedef struct _SRV1_Coordinate SRV1_Coordinate;

static bool isInMotion = FALSE;

double lineLength( SRV1_Coordinate * c );

double lineAngleFromY( SRV1_Coordinate * c );

double lineAngleFromX( SRV1_Coordinate * c );

void goFidoo( SRV1_Coordinate point_loc );

