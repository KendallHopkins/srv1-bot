#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

typedef struct _SRV1_connection SRV1_connection;

typedef struct _SRV1_Coordinate SRV1_Coordinate;
struct _SRV1_Coordinate {
    double x;
    double y;
};

typedef enum {
    MOTORSPEED_FORWARD_100 = 100,
    MOTORSPEED_FORWARD_75 = 75,
    MOTORSPEED_FORWARD_50 = 50,
    MOTORSPEED_FORWARD_25 = 25, /* not powerful enough to spin bot */
    MOTORSPEED_OFF = 0,
    MOTORSPEED_REVERSE_25 = -25, /* not powerful enough to spin bot */
    MOTORSPEED_REVERSE_50 = -50,
    MOTORSPEED_REVERSE_75 = -75,
    MOTORSPEED_REVERSE_100 = -100
} MotorSpeed;

typedef enum {
    /* IMAGESIZE_80_64 = 1, specs seem not to support this size */
    IMAGESIZE_160_120 = 3, /* 0.03 second */
    IMAGESIZE_320_240 = 5, /* 0.08 second */
    IMAGESIZE_640_480 = 7, /* 0.33 second */
    IMAGESIZE_1280_1024 = 9 /* 1.43 second */
} ImageSize;

typedef enum {
    IMAGEQUALITY_1 = '1',
    IMAGEQUALITY_2 = '2',
    IMAGEQUALITY_3 = '3',
    IMAGEQUALITY_4 = '4',
    IMAGEQUALITY_5 = '5',
    IMAGEQUALITY_6 = '6',
    IMAGEQUALITY_7 = '7',
    IMAGEQUALITY_8 = '8'
} ImageQuality;

typedef enum {
    AUTOVISIONFLAG_AEC = 1 << 0, //1
    AUTOVISIONFLAG_AWB = 1 << 1, //2
    AUTOVISIONFLAG_AGC = 1 << 2  //4
} ImageAutoVisionFlag;

void SRV1_init();
SRV1_connection * SRV1_new( const char * ip );
void SRV1_free( SRV1_connection * connection );
void SRV1_sendRawMove( SRV1_connection * connection, int8_t left_speed, int8_t right_speed, uint8_t time_in_centiseconds ); //0 time is infinity
void SRV1_setLasers( SRV1_connection * connection, bool enabled ); //pew pew pew

//if you use a different size, the robot was to wait a bit for the new image size to be ready
char * SRV1_getRawJPG( SRV1_connection * connection, ImageSize size, ImageQuality quality, ImageAutoVisionFlag flag, uint32_t * _image_size );
SDL_Surface * SRV1_getImage( SRV1_connection * connection, ImageSize size, ImageQuality quality, ImageAutoVisionFlag flag );

//This function is implemented very crappily on the bot and almost never work properly unless very close to a very flat object.
int16_t SRV1_laserMeasureDistance( SRV1_connection * connection );

void SRV1_setBlobColorRange( SRV1_connection * connection, uint8_t color_bin_index, uint8_t red_min, uint8_t red_max, uint8_t green_min, uint8_t green_max, uint8_t blue_min, uint8_t blue_max );
SRV1_Coordinate * SRV1_findBlobs( SRV1_connection * connection, uint8_t color_bin_index, ImageSize image_size );
