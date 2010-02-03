#include <unistd.h>
#include <assert.h>
#include "srv1.h"
#include <SDL_net.h>
#include <SDL_image.h>
#include <time.h>
#include <stdlib.h>

//wait for bot to return data
#define TIMEOUT 10000

//wait for the camera to reset for new settings
#define CAMERA_SWAP_WAIT 500000

SDLNet_SocketSet generalSocketSet;

struct _SRV1_connection {
    TCPsocket tcpsock;
    ImageSize current_image_size;
    ImageQuality current_image_quality;
    ImageAutoVisionFlag current_flag;
    bool laser_enabled;
};

void _SRV1_setResolution( SRV1_connection * connection, ImageSize size );
void _SRV1_setImageQuality( SRV1_connection * connection, ImageQuality quality );
void _SRV1_autoVision( SRV1_connection * connection, ImageAutoVisionFlag flag );

void SRV1_init()
{
    assert( SDL_Init(SDL_INIT_VIDEO) == 0 );
    assert( SDLNet_Init() == 0 );
    generalSocketSet = SDLNet_AllocSocketSet(1);    
}

SRV1_connection * SRV1_new( const char * ip_address )
{
    SRV1_connection * new_connection = (SRV1_connection *) malloc( sizeof( SRV1_connection ) );
    IPaddress ipaddress;
    
    SDLNet_ResolveHost( &ipaddress, ip_address, 10001 );
    new_connection->tcpsock = SDLNet_TCP_Open( &ipaddress );
    new_connection->current_image_size = 0;
    new_connection->current_image_quality = 0;
    new_connection->current_flag = 0;
    new_connection->laser_enabled = 0;
    assert( new_connection->tcpsock );
    return new_connection;
}

void SRV1_free( SRV1_connection * connection )
{
    SDLNet_TCP_Close( connection->tcpsock );
    free( connection );
}

int _SRV1_sendBasicCommand( SRV1_connection * connection, char * message, int message_length, char * response, int respone_length )
{
    SDLNet_TCP_AddSocket( generalSocketSet, connection->tcpsock );
    assert( SDLNet_TCP_Send( connection->tcpsock, message, message_length ) > 0 );
    assert( SDLNet_CheckSockets( generalSocketSet, TIMEOUT ) );
    int actual_response_length = SDLNet_TCP_Recv( connection->tcpsock, response, respone_length );
    SDLNet_TCP_DelSocket( generalSocketSet, connection->tcpsock );
    return actual_response_length;
}

void SRV1_sendRawMove( SRV1_connection * connection, int8_t left_speed, int8_t right_speed, uint8_t time_in_centiseconds )
{
    char message[4] = { 'M', left_speed, right_speed, time_in_centiseconds };
    char response[2];
    assert( _SRV1_sendBasicCommand( connection, message, 4, response, 2 ) == 2 );
    assert( memcmp( "#M", response, 2 ) == 0 );
}

void SRV1_setLasers( SRV1_connection * connection, bool enabled )
{
    char message[1];
    message[0] = enabled ? 'l' : 'L';
    char response[5];
    assert( _SRV1_sendBasicCommand( connection, message, 1, response, 5 ) == 2 );
    assert( ( response[0] == '#' ) && ( response[1] == message[0] ) );
    connection->laser_enabled = enabled;
}

char * SRV1_getRawJPG( SRV1_connection * connection, ImageSize size, ImageQuality quality, ImageAutoVisionFlag flag, uint32_t * _image_size )
{
    if( connection->current_flag != flag )
        _SRV1_autoVision( connection, flag );
        
    if( connection->current_image_size != size )
        _SRV1_setResolution( connection, size );
    
    if( connection->current_image_quality != quality )
        _SRV1_setImageQuality( connection, quality );
    
    SDLNet_TCP_AddSocket( generalSocketSet, connection->tcpsock );
    
    char message[4] = { 'I' };
    assert( SDLNet_TCP_Send( connection->tcpsock, message, 1 ) > 0 );
    
    int response = SDLNet_CheckSockets( generalSocketSet, TIMEOUT );
    assert( response > 0 );
    
    unsigned char buffer[10];
    int result = SDLNet_TCP_Recv( connection->tcpsock, buffer, 10 );
    assert( result == 10 );
    assert( memcmp( "##IMJ", buffer, strlen( "##IMJ" ) ) == 0 );
    uint32_t image_size = ( (uint32_t)buffer[6] ) + ( ((uint32_t)buffer[7])<<8 ) + ( ((uint32_t)buffer[8])<<16 ) + ( ((uint32_t)buffer[9])*24 );
    
    char * image_buffer = malloc( image_size );
    int index = 0;
    while( index < image_size ) {
        assert( SDLNet_CheckSockets( generalSocketSet, TIMEOUT ) );
        index += SDLNet_TCP_Recv( connection->tcpsock, image_buffer + index, image_size - index );
    }
    
    SDLNet_TCP_DelSocket( generalSocketSet, connection->tcpsock );
    
    *_image_size = image_size;
    return image_buffer;
}

SDL_Surface * SRV1_getImage( SRV1_connection * connection, ImageSize size, ImageQuality quality, ImageAutoVisionFlag flag )
{
    uint32_t image_size;
    char * image_buffer = SRV1_getRawJPG( connection, size, quality, flag, &image_size );
    SDL_RWops * rwop = SDL_RWFromMem( image_buffer, image_size );
    SDL_Surface * image = IMG_LoadJPG_RW( rwop );
    return image;
}


void _SRV1_setResolution( SRV1_connection * connection, ImageSize size )
{
    char message[1];
    switch( size ) {
        case IMAGESIZE_160_120: message[0] = 'a'; break;
        case IMAGESIZE_320_240: message[0] = 'b'; break;
        case IMAGESIZE_640_480: message[0] = 'c'; break;
        case IMAGESIZE_1280_1024: message[0] = 'A'; break;
        default: assert( 0 );
    }
    char response[2];
    assert( _SRV1_sendBasicCommand( connection, message, 1, response, 2 ) == 2 );
    assert( ( response[0] == '#' ) && ( response[1] == message[0] ) );
    connection->current_image_size = size;
    usleep( CAMERA_SWAP_WAIT );
}

void _SRV1_setImageQuality( SRV1_connection * connection, ImageQuality quality )
{
    assert( quality >= '1' && quality <= '8' );
    char message[2] = { 'q', quality };
    char response[15];
    assert( _SRV1_sendBasicCommand( connection, message, 2, response, 15 ) == 15 );
    assert( memcmp( "##quality", response, strlen( "##quality" ) ) == 0 );
    connection->current_image_quality = quality;
    usleep( CAMERA_SWAP_WAIT );
}

int16_t SRV1_laserMeasureDistance( SRV1_connection * connection )
{
    char response[200];
    _SRV1_sendBasicCommand( connection, "R", 1, response, 200 );
    if( memcmp( "##Range(cm) = ", response, 14 ) != 0 ) return -1;
    
    char number_string[6] = {0};
    int i = 0;
    while( i < 5 && response[i+14] >= '0' && response[i+14] <= '9' ) {
        number_string[i] = response[i+14];
        i++;
    }
    return atoi( number_string );
}

void _SRV1_autoVision( SRV1_connection * connection, ImageAutoVisionFlag flag )
{
    char flag_data = flag + '0';
    char message[3] = { 'v', 'a', flag_data };
    char response[7];

    _SRV1_sendBasicCommand( connection, message, 3, response, 7 );
    assert( memcmp( "##va", response, 4 ) == 0 );
    assert( response[4] == flag_data );
    assert( memcmp( "\r\n", response + 5, 2 ) == 0 );
    connection->current_flag = flag;
    usleep( CAMERA_SWAP_WAIT );
}

void SRV1_setBlobColorRange( SRV1_connection * connection, uint8_t color_bin_index, uint8_t red_min, uint8_t red_max, uint8_t green_min, uint8_t green_max, uint8_t blue_min, uint8_t blue_max )
{
    char response[200];
    char message[25];
    int message_size = snprintf( message, 25, "vc%01d%03d%03d%03d%03d%03d%03d", color_bin_index, red_min, red_max, green_min, green_max, blue_min, blue_max );
    _SRV1_sendBasicCommand( connection, message, message_size, response, 200 );
    //TODO: program checks
}

SRV1_Coordinate * SRV1_findBlobs( SRV1_connection * connection, uint8_t color_bin_index, ImageSize image_size )
{
    if( connection->current_image_size != image_size )
        _SRV1_setResolution( connection, image_size );
    
    char response[1024] = {0};
    char message[25];
    int message_size = snprintf( message, 25, "vb%01d", color_bin_index );
    int responce_size = _SRV1_sendBasicCommand( connection, message, message_size, response, 1024 );
    
    int i = 0;
    while( response[i] != '\r' && response[i+1] != '\n' ){
        if( i >= responce_size ) return NULL;
        i++;
    }
    i += 2;
    
    printf("%s",response);
    
    int size;
    int x_min, x_max, y_min, y_max;
    
    int result = sscanf( response + i, " %d - %d %d %d %d  \r\n", &size, &x_min, &x_max, &y_min, &y_max );
    if( result != 5 ) return NULL;
        
    SRV1_Coordinate * new_coordinate = (SRV1_Coordinate*)malloc( sizeof( SRV1_Coordinate ) );
    new_coordinate->x = (x_min+x_max)/2;
    new_coordinate->y = (y_min+y_max)/2;
    
    switch( connection->current_image_size ) {
        case IMAGESIZE_160_120:
            new_coordinate->y -= 120;
            new_coordinate->y = fabs( new_coordinate->y );
            new_coordinate->x -= 80;
            break;
        case IMAGESIZE_320_240:
            new_coordinate->y -= 240;
            new_coordinate->y = fabs( new_coordinate->y );
            new_coordinate->x -= 160;
            break;
        case IMAGESIZE_640_480:
            new_coordinate->y -= 480;
            new_coordinate->y = fabs( new_coordinate->y );
            new_coordinate->x -= 320;
            break;
        case IMAGESIZE_1280_1024:
            new_coordinate->y -= 1024;
            new_coordinate->y = fabs( new_coordinate->y );
            new_coordinate->x -= 640;
            break;
        default:
            break;
    }
    
    return new_coordinate;
}
