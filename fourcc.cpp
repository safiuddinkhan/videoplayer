#include "fourcc.h"
/// Modified from /ffmpeg/tools/fourcc2pixfmt.c in the source code


char * get_fourcc_code(AVPixelFormat pix_fmt)
{
    int i;
    char *buf = new char(32);

    for (i = 0; ff_raw_pix_fmt_tags[i].pix_fmt != AV_PIX_FMT_NONE; i++) {
        if (ff_raw_pix_fmt_tags[i].pix_fmt == pix_fmt) {
            
            av_get_codec_tag_string(buf, sizeof(buf), ff_raw_pix_fmt_tags[i].fourcc);
            return buf;
        }
    }


}


AVPixelFormat get_avpixelformat(char * fourcc){
int i;
    char buf[32];
    for (i = 0; ff_raw_pix_fmt_tags[i].pix_fmt != AV_PIX_FMT_NONE; i++) {
av_get_codec_tag_string(buf, sizeof(buf), ff_raw_pix_fmt_tags[i].fourcc);
if(strcmp(buf,fourcc) == 0){
	return ff_raw_pix_fmt_tags[i].pix_fmt;
}

}

return (AVPixelFormat)AV_PIX_FMT_NONE;
}
