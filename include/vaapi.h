extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/vaapi.h>
}

#include <va/va.h>
#include <va/va_x11.h>

class vaapi{
private:
static enum AVPixelFormat get_format_vaapi(struct AVCodecContext *s, const enum AVPixelFormat *fmt);
static int get_buffer(struct AVCodecContext *avctx, AVFrame *pic,int flags);
static void release_buffer(void *opaque, uint8_t *data);
VADisplay va_dpy;

public:
int activate_vaapi_decoding(struct AVCodecContext *avctx,VADisplay* va_dpy);
int deinit_vaapi(struct AVCodecContext *avctx);

};

VADisplay init_vaapi(Display * display);
VAImageFormat * get_vaformat(enum AVPixelFormat fmt,VADisplay* va_dpy);


/*
class vaapi_upload: public vaapi{
private:
VAImage *image1;
VASurfaceID surfaceid1;
uint8_t *vidbuffer1;
VADisplay* dpy;
int h;
int w;

public:
vaapi_upload(VADisplay* display);
int init(int width,int height,VAImageFormat *vaformat);
VASurfaceID upload(uint8_t ** data);
~vaapi_upload();

};
*/