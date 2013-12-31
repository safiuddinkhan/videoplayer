#include "common.h"
#include "vaapi.h"

using namespace std;
using namespace TagLib::ID3v2;



void init_all();


////// Internet Structure not to be used by the user
struct stream_context{
AVFormatContext *pFormatCtx = NULL;
AVCodecContext *videoctx;
AVCodecContext *audioctx;
int audiostream;
int videostream;
int attachedimage;
std::queue <AVPacket> audiobuffer;
std::queue <AVPacket> videobuffer;
double videobasetime;
double audiobasetime;
int endthread;
int pausetoggle;
int audiopause;
int videopause;
int demuxpause;
//int demux_block;
media_clock *masterclock;


double videopts;
double lastvideopts;
double audiopts;
double lastaudiopts;
pthread_t audiothread;
pthread_t videothread;
pthread_t demuxerthread;


pthread_mutex_t audiolock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t videolock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t seeklock;


pthread_mutex_t demuxlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t demuxcond;

pthread_mutex_t demuxpauselock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t demuxpausecond;
int demuxpausetoggle;

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
//double seekdelay;
//double new_audio_seektime;
//double new_video_seektime;


int stop;
double totalduration;
double oldcurrenttime;

mediaplayer_status status;


double start_time;
int is_seekable;
AVPixelFormat pixelformat;

int videoseek;
int audioseek;

int networkstream;
stream_type streamtype;

////////////////////////
int video_flag;
int audio_flag;
////////////////////////

////////////////////////
int end_audiothread;
int end_videothread;
////////////////////////


AVDictionaryEntry *tag;
int video_accel_mode;


///////////////////////////
SwsContext * convert_ctx;
AVFrame *vidframe1;
///////////////////////////

int fc;
//AVFrame *vidframe;
void * opaque;
void (*video_callback)(void * ,void ** ,int *, double , void *);
void (*audio_callback)(uint8_t *,int , double , void *);

void *display;

/////////////////
VAImageFormat *vaformat;
///////////////


int height;
int width;
int samplerate;
int channels;
int aspect_ratio_num;
int aspect_ratio_den;
int is_mp3;


};


class internel_decoder: public vaapi{
protected:
stream_context *sc;
static void put_status(mediaplayer_status status,stream_context *sc);


// Seeking functions
static int seek_stream(double seektime,stream_context *sc);
int check_stream_seekable(stream_context *streamcontext);

// Video / Audio decoding functions
static int getdecodedvideoframe(stream_context *sc,AVFrame *vidframe);
static int getdecodedaudioframe(stream_context *sc,AVFrame *audioframe);

// Audio / Video playback threads
static void *audioplayback(void *arg);
static void *videoplayback(void *arg);

// Demux Thread and related functions
static void *demuxer(void *arg);
static void demux_pause(stream_context *streamcontext);

// Buffering functions
static int init_buffering(int min_videobuffersize,int min_audiobuffersize,stream_context *streamcontext);
static int detect_pause(stream_context *streamcontext);
static int end_buffering(int videobuffersize,int audiobuffersize,stream_context *streamcontext);
static int buffer_limit(int videobuffersize,int audiobuffersize,stream_context *streamcontext);
static void empty_buffers(stream_context *sc);

// I/O and loading functions
int findandopencodec(AVCodecContext *pCodecCtx);
int get_first_streams(stream_context *streamcontext);
int get_attached_images(char* url,stream_context *streamcontext);
int load_file(char *filename,stream_context *streamcontext);
int load_network_stream(char *url,stream_context *streamcontext);
int load_live_videsource(char *device,stream_context *streamcontext);

// MRL parsing functions
stream_type stream_detector(char *url);

// V4L function
AVInputFormat *init_v4l(char *url);
};

//////////////////////////////////////////////////
// Copied from FFMPEG Source Code
 typedef struct CodecMime{
  char str[32];
  enum AVCodecID id; 
} CodecMime;

const CodecMime ff_id3v2_mime_tags[] = {
  { "image/gif", AV_CODEC_ID_GIF },
  { "image/jpeg", AV_CODEC_ID_MJPEG },
  { "image/jpg", AV_CODEC_ID_MJPEG },
  { "image/png", AV_CODEC_ID_PNG },
  { "image/tiff", AV_CODEC_ID_TIFF },
  { "image/bmp", AV_CODEC_ID_BMP },
  { "JPG", AV_CODEC_ID_MJPEG }, /* ID3v2.2 */
  { "PNG", AV_CODEC_ID_PNG }, /* ID3v2.2 */
  { "", AV_CODEC_ID_NONE },
 };

 int av_strncasecmp(const char *a, const char *b, size_t n);
//////////////////////////////////////////////////