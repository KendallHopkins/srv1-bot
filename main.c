#include "srv1.h"

#include <unistd.h>

void saveData( const char * file_path, char * data, uint32_t size )
{
    FILE * file = fopen( file_path, "w" );
    fwrite( data, size, 1, file );
    fclose( file );
}

int main(int argc, char* argv[])
{
    //init the environment
    SRV1_init();
    
    //create a new bot connection
    SRV1_connection * bot_connection = SRV1_new( "169.254.0.10" );
    
    //image process test
    char * image_data;
    uint32_t image_size;
    
    image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, 0, &image_size );
    saveData( "/tmp/image_none.jpg", image_data, image_size );
    
    image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, AUTOVISIONFLAG_AEC, &image_size );
    saveData( "/tmp/image_aec.jpg", image_data, image_size );
    
    image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, AUTOVISIONFLAG_AGC, &image_size );
    saveData( "/tmp/image_agc.jpg", image_data, image_size );
    
    image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, AUTOVISIONFLAG_AWB, &image_size );
    saveData( "/tmp/image_awb.jpg", image_data, image_size );
    
    image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, AUTOVISIONFLAG_AWB | AUTOVISIONFLAG_AEC | AUTOVISIONFLAG_AGC, &image_size );
    saveData( "/tmp/image_all.jpg", image_data, image_size );
    
    //spin for until we tell it stop
    SRV1_sendRawMove( bot_connection, MOTORSPEED_FORWARD_50, MOTORSPEED_REVERSE_50, 0 );
    
    //wait a bit, keep spinning
    sleep(1);
    
    //grab an image while still spinning
    image_data = SRV1_getRawJPG( bot_connection, IMAGESIZE_1280_1024, IMAGEQUALITY_8, 0, &image_size );
    
    //write image to file
    FILE * image_file = fopen( "/tmp/image.jpg", "w" );
    fwrite( image_data, image_size, 1, image_file );
    fclose( image_file );
    
    //tell bot to stop
    SRV1_sendRawMove( bot_connection, MOTORSPEED_OFF, MOTORSPEED_OFF, 0 );
    
    SRV1_free(bot_connection);
    
	return 0;
}