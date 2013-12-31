
#include "internels.h"





class mediaplayer: public internel_decoder{
public:
int height;
int width;
int samplerate;
int channels;
int aspect_ratio_num;
int aspect_ratio_den;
stream_type streamtype;
VAImage *va_image;

mediaplayer(char *url , void *display);
int set_pixel_format(pixel_format pixformat);
void set_callbacks(void (*video_callback)(void *,void ** ,int *, double , void *),void (*audio_callback)(uint8_t *,int , double , void *),void * opaque);


int play();
int stop();
int pause();
void seek(double timestamp);
double getpos();
double getduration();
//char * get_pixelformat();
mediaplayer_status getstatus();
metadata_entry *get_metadata();
~mediaplayer();
};




