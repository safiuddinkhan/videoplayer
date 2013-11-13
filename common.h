#include "fourcc.h"
#include "av_struct.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include "libavutil/common.h"
#include "libavdevice/avdevice.h"
#include "libavutil/avstring.h"	
}

#include <sys/time.h>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <unistd.h>

#include <taglib/id3v2tag.h>
#include <taglib/tbytevector.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>


//#include <va/va.h>


class media_clock{
private:
int64_t itime;
double curtime;
double stime;

public:  
media_clock();
void reset();
double gettime();
void settime(double time);
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
