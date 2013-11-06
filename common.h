#include "fourcc.h"
#include "colorspace_converter.h"
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
}

#include <sys/time.h>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <unistd.h>
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


