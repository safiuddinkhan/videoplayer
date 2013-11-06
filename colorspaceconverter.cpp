#include "colorspace_converter.h"
#include "fourcc.h"
#include <iostream>
using namespace std;
colorspace_converter::colorspace_converter(int h,int w,char *srcfourcc,char *dstfourcc){
vidframe = avcodec_alloc_frame();
video_out = new video();
height = h;
width = w;
AVPixelFormat dstfmt;
dstfmt = get_avpixelformat(dstfourcc);

if(dstfmt == AV_PIX_FMT_NONE){
 std::cout <<"Error:Could Not Recognize FourCC Code"<<std::endl;
  error = 1;
}else{
  error = 0;


numBytes=avpicture_get_size(dstfmt ,width,height);
vidbuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
avpicture_fill((AVPicture *)vidframe, vidbuffer, dstfmt ,width, height);

//mm = mp;

convert_ctx = sws_getContext(width, height, get_avpixelformat(srcfourcc),
                                   width, height,dstfmt,
                                   SWS_BICUBIC, NULL, NULL, NULL );


}
}


video *colorspace_converter::convert(video * video_in){
if(error == 0){
sws_scale(convert_ctx,video_in->data,video_in->linesize,0,height,vidframe->data,vidframe->linesize);
video_out->data = vidframe->data;
video_out->linesize = vidframe->linesize;
video_out->pts = video_in->pts;
return video_out;
}
}

colorspace_converter::~colorspace_converter(){
 if(error == 0){ 
sws_freeContext (convert_ctx);
av_free(vidframe);
av_freep(&vidbuffer); 
}
}
