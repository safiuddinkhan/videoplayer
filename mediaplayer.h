
#include "common.h"
#include <SDL/SDL.h>





using namespace std;



enum stream_type{
stream_network,
stream_v4l,
stream_localfile,
stream_none
};

 

 enum mediaplayer_status{
MP_STOP,
MP_PLAYING,
MP_PAUSE,
MP_SEEKING,
MP_ERROR
 };


struct meta_data{
char *key;
char *value;
meta_data *next;
};


void init_all();


////// Internet Structure not to be used by the user
struct stream_context{
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *videoctx;
AVCodecContext *audioctx;
int audiostream;
int videostream;
std::queue <AVPacket> audiobuffer;
std::queue <AVPacket> videobuffer;
double videobasetime;
double audiobasetime;
int endthread ;
int pausetoggle;
int audiopause;
int videopause;
int demuxpause;
int demux_block;
media_clock *masterclock;


double videopts;
double lastvideopts;
double audiopts;
double lastaudiopts;
pthread_t audiothread;
pthread_t videothread;
pthread_t demuxerthread;
pthread_t audioplaybackthread;


pthread_mutex_t audiolock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t videolock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t seeklock;


pthread_mutex_t demuxlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t demuxcond;


pthread_mutex_t decodelock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t decodecond;

pthread_mutex_t decodelock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t decodecond1;

pthread_mutex_t waitlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t audio_waitcond;
pthread_cond_t video_waitcond;
pthread_cond_t demux_waitcond;


pthread_mutex_t pauselock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pausecond;

pthread_mutex_t videoframelock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t videodatalock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t videoframeupdate;

pthread_mutex_t audioframelock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t audiodatalock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t audioframeupdate;

pthread_mutex_t codec_open_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t end_status_lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_status_lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t end_thread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t status_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t video_seek_status_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t audio_seek_status_lock = PTHREAD_MUTEX_INITIALIZER;

int seek;
//int audioseek;
double seektime;
double seekdelay;
double new_audio_seektime;
double new_video_seektime;


int seekback;
int stop;
double totalduration;
double oldcurrenttime;
mediaplayer_status status;
uint8_t **videodata;
int *linesize;
double start_time;
int is_seekable;
//uint8_t *audiodata;
//int audiopacketsize;
audio *aout;
video *vout;
//video *vout1;

AVPixelFormat pixelformat;
//AVFrame *vidframe;
//colorspace_converter *cc;
SDL_Surface *screen;

int videoseek;
int audioseek;

int networkstream;
int v4l_device;
stream_type streamtype;

int video_flag;
int audio_flag;
int end_audiothread;
int end_videothread;

meta_data *mdata_init;
meta_data *mdata;
meta_data *mdata_curr;

char *pixel_format;


};


class mediaplayer{
private:


void audio_pause_callback(int toggle);
static void put_status(mediaplayer_status status,stream_context *sc);
static void empty_buffers(stream_context *sc);
static int seek_stream(double seektime,stream_context *sc);
static int getdecodedvideoframe(stream_context *sc,AVFrame *vidframe);
static int getdecodedaudioframe(stream_context *sc,AVFrame *audioframe);
static void *audioplayback(void *arg);
static void *videoplayback(void *arg);
static void *demuxer(void *arg);

int findandopencodec(AVCodecContext *pCodecCtx, int stream_index);
int loadfile(char *url,stream_context *streamcontext);
stream_type stream_detector(char *url);


public:
stream_context *sc;	
int height;
int width;
int samplerate;
int channels;
stream_type streamtype;
mediaplayer(char *file);
void set_pixelformat(char *fourcc_code);
video *get_playback_videoframe();
audio *get_playback_audioframe();


video *next_videoframe();
audio *next_audioframe();

int play();
int stop();
int pause();
void seek(double timestamp);
double getpos();
double getduration();
//char * get_pixelformat();
mediaplayer_status getstatus();
void get_metadata(char *key,char *value,int flag);
~mediaplayer();
};




