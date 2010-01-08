#include <unistd.h>
#include <assert.h>
#include "srv1.h"
#include <SDL_net.h>
#include <SDL_image.h>
#include <time.h>
#include <stdlib.h>

#define TIMEOUT 10000

SDLNet_SocketSet generalSocketSet;

struct _SRV1_connection {
    TCPsocket tcpsock;
    ImageSize current_image_size;
    ImageQuality current_image_quality;
    bool laser_enabled;
};

void _SRV1_setResolution( SRV1_connection * connection, ImageSize size );
void _SRV1_setImageQuality( SRV1_connection * connection, ImageQuality quality );

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
    new_connection->laser_enabled = 0;
    assert( new_connection->tcpsock );
    return new_connection;
}

void SRV1_free( SRV1_connection * connection )
{
    SDLNet_TCP_Close( connection->tcpsock );
    free( connection );
}

int _SRV1_sendBasicCommand( SRV1_connection * connection, char * message, int message_length, char * responce, int respone_length )
{
    SDLNet_TCP_AddSocket( generalSocketSet, connection->tcpsock );
    assert( SDLNet_TCP_Send( connection->tcpsock, message, message_length ) > 0 );
    assert( SDLNet_CheckSockets( generalSocketSet, TIMEOUT ) );
    int actual_responce_length = SDLNet_TCP_Recv( connection->tcpsock, responce, respone_length );
    SDLNet_TCP_DelSocket( generalSocketSet, connection->tcpsock );
    return actual_responce_length;
}

void SRV1_sendRawMove( SRV1_connection * connection, int8_t left_speed, int8_t right_speed, uint8_t time_in_centiseconds )
{
    char message[4] = { 'M', left_speed, right_speed, time_in_centiseconds };
    char responce[2];
    assert( _SRV1_sendBasicCommand( connection, message, 4, responce, 2 ) == 2 );
    assert( memcmp( "#M", responce, 2 ) == 0 );
}

void SRV1_turn( SRV1_connection * connection, int angle )
{
    
}

void SRV1_setLasers( SRV1_connection * connection, bool enabled )
{
    char message[1];
    message[0] = enabled ? 'l' : 'L';
    char responce[2];
    assert( _SRV1_sendBasicCommand( connection, message, 1, responce, 2 ) == 2 );
    assert( ( responce[0] == '#' ) && ( responce[1] == message[0] ) );
    connection->laser_enabled = enabled;
}

SDL_Surface * SRV1_getImage( SRV1_connection * connection, ImageSize size, ImageQuality quality )
{
    if( connection->current_image_size != size )
        _SRV1_setResolution( connection, size );
    
    if( connection->current_image_quality != quality )
        _SRV1_setImageQuality( connection, quality );
    
    SDLNet_TCP_AddSocket( generalSocketSet, connection->tcpsock );
    
    char message[4] = { 'I' };
    assert( SDLNet_TCP_Send( connection->tcpsock, message, 1 ) > 0 );
    
    int responce = SDLNet_CheckSockets( generalSocketSet, TIMEOUT );
    assert( responce > 0 );
    
    unsigned char buffer[10];
    int result = SDLNet_TCP_Recv( connection->tcpsock, buffer, 10 );
    assert( result == 10 );
    assert( memcmp( "##IMJ", buffer, strlen( "##IMJ" ) ) == 0 );
    uint32_t image_size = ( (uint32_t)buffer[6] ) + ( ((uint32_t)buffer[7])*256 ) + ( ((uint32_t)buffer[8])*65536 ) + ( ((uint32_t)buffer[9])*16777216 );
    
    char * image_buffer = malloc( image_size );
    int index = 0;
    while( index < image_size ) {
        assert( SDLNet_CheckSockets( generalSocketSet, TIMEOUT ) );
        index += SDLNet_TCP_Recv( connection->tcpsock, image_buffer + index, image_size - index );
    }
    
    FILE * image_file = fopen( "/tmp/file.jpg", "w" );
    assert( image_file );
    fwrite( image_buffer, image_size, 1, image_file );
    fclose( image_file );
    
    SDL_RWops * rwop = SDL_RWFromMem( image_buffer, image_size );
    SDL_Surface * image = IMG_LoadJPG_RW( rwop );
    SDLNet_TCP_DelSocket( generalSocketSet, connection->tcpsock );
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
    char responce[2];
    assert( _SRV1_sendBasicCommand( connection, message, 1, responce, 2 ) == 2 );
    assert( ( responce[0] == '#' ) && ( responce[1] == message[0] ) );
    connection->current_image_size = size;
    usleep( 250000 );
}

void _SRV1_setImageQuality( SRV1_connection * connection, ImageQuality quality )
{
    assert( quality >= '1' && quality <= '8' );
    char message[2] = { 'q', quality };
    char responce[13];
    assert( _SRV1_sendBasicCommand( connection, message, 2, responce, 13 ) == 13 );
    assert( memcmp( "##quality", responce, strlen( "##quality" ) ) == 0 );
    connection->current_image_quality = quality;
    usleep( 250000 );
}

//void SRV1_enableColorSegmentation( SRV1_connection * connection )
//{
//    char responce[4];
//    assert( _SRV1_sendBasicCommand( connection, "g1", 2, responce, 4 ) == 4 );
//    assert( memcmp( "##g1", responce, 4 ) == 0 );
//}

int16_t SRV1_laserMeasureDistance( SRV1_connection * connection )
{
    char responce[200];
    _SRV1_sendBasicCommand( connection, "R", 1, responce, 200 );
    if( memcmp( "##Range(cm) = ", responce, 14 ) != 0 ) return -1;
    
    char number_string[6] = {0};
    int i = 0;
    while( i < 5 && responce[i+14] >= '0' && responce[i+14] <= '9' ) {
        number_string[i] = responce[i+14];
        i++;
    }
    return atoi( number_string );
}