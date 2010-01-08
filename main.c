#include "srv1.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    //init the environment
    SRV1_init();
    
    //create a new bot connection
    SRV1_connection * bot_connection = SRV1_new( "169.254.0.10" );
    
    //spin for until we tell it stop
    SRV1_sendRawMove( bot_connection, MOTORSPEED_FORWARD_50, MOTORSPEED_REVERSE_50, 0 );
    
    //wait a bit, keep spinning
    sleep(1);
    
    //grab an image while still spinning
    uint32_t image_size;
    char * image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, &image_size );
    
    //write image to file
    FILE * image_file = fopen( "/tmp/image.jpg", "w" );
    fwrite( image_data, image_size, 1, image_file );
    fclose( image_file );
    
    //tell bot to stop
    SRV1_sendRawMove( bot_connection, MOTORSPEED_OFF, MOTORSPEED_OFF, 0 );
    
    SRV1_free(bot_connection);
    
	return 0;
}