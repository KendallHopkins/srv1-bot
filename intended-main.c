#include "Tracker.h"
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    //init the environment
    SRV1_init();
    
    SRV1_connection * bot_connection = SRV1_new( "169.254.0.10" );
	SRV1_setBlobColorRange( bot_connection, 1, 200, 255, 50, 150, 50, 150);
	
	//SRV1_sendRawMove( bot_connection, -40, 54, 80 );
	
	while( 1 ) {
		SRV1_Coordinate * point = SRV1_findBlobs( bot_connection, 1, IMAGESIZE_640_480 );
		goFidoo( bot_connection, point );
	}
	
    SRV1_sendRawMove( bot_connection, MOTORSPEED_OFF, MOTORSPEED_OFF, 0 );
	
	SRV1_free( bot_connection );
	return 0;
}
