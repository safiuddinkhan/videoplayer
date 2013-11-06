extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#include "av_struct.h"

class colorspace_converter{
private:
int numBytes;
uint8_t *vidbuffer;
AVFrame *vidframe;
int height;
int width;
SwsContext * convert_ctx;
video *video_out;
int error;
public:
colorspace_converter(int h,int w,char *srcfourcc,char *dstfourcc);
video *convert(video * video_in);
~colorspace_converter();
};