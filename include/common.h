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
#include <libavcodec/vaapi.h>
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
#include "pixelformat.h"
#include "clock.h"

enum stream_type{
stream_network,
stream_livesource,
stream_localfile,
stream_none
};

 
enum mediaplayer_status{
MP_STOP,
MP_PLAYING,
MP_PAUSE,
MP_BUFFERING,
MP_SEEKING,
MP_ERROR
 };


struct metadata_entry {
char *key;
char *value;
};
