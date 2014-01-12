#include "vaapi.h"
#include "pixelformat.h"
#include <iostream>


using namespace std;
///////////////////////// Video Acceleration /////////////////////////
VADisplay init_vaapi(Display * display){
  int major_ver, minor_ver;
VAStatus va_status;
VADisplay va_dpy;
va_dpy = vaGetDisplay(display);
va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);
if(va_status == VA_STATUS_SUCCESS){
  cout <<"VAAPI initialized successfully..."<<endl;
return va_dpy;
}

return NULL;
}


int surf_id;
int vaapi::deinit_vaapi(struct AVCodecContext *avctx){
VASurfaceID *surface_id = (VASurfaceID *)avctx->opaque;

vaapi_context *vc1 = (vaapi_context *)avctx->hwaccel_context;
VADisplay va_dpy = (VADisplay)vc1->display;
VAConfigID config_id = (VAConfigID)vc1->config_id;
VAContextID context_id =(VAContextID)vc1->context_id;

vaDestroySurfaces(va_dpy,(VASurfaceID *)surface_id,16);
vaDestroyConfig(va_dpy,config_id);
vaDestroyContext(va_dpy,context_id);
vaTerminate(va_dpy);

return -1;  
}

int vaapi::activate_vaapi_decoding(struct AVCodecContext *avctx,VADisplay* va_dpy){
surf_id = 0;
int profile;
VASurfaceID *surface_id;
VAEntrypoint entrypoints[5];
int num_entrypoints,vld_entrypoint;
int va_format;
int i;
int major_ver, minor_ver;
VAStatus va_status;


switch (avctx->codec_id) {
case CODEC_ID_MPEG2VIDEO:
profile = VAProfileMPEG2Main;
cout <<"MPEG 2 Profile Found..."<<endl;
break;
case CODEC_ID_MPEG4:
case CODEC_ID_H263:
profile = VAProfileMPEG4AdvancedSimple;
cout <<"MPEG4 Profile Found..."<<endl;
break;
case CODEC_ID_H264:
profile = VAProfileH264High;
cout <<"H264 Profile Found..."<<endl;
break;
case CODEC_ID_WMV3:
profile = VAProfileVC1Main;
cout <<"WMV3 Profile Found..."<<endl;
break;
case CODEC_ID_VC1:
profile = VAProfileVC1Advanced;
cout <<"VC1 Profile Found..."<<endl;
break;
default:
profile = -1;
break;
}

switch(avctx->pix_fmt){
case AV_PIX_FMT_YUV420P:
va_format = VA_RT_FORMAT_YUV420;
cout <<"format found... YUV420"<<endl;
break;

case AV_PIX_FMT_YUV422P:
va_format = VA_RT_FORMAT_YUV422;
cout <<"format found... YUV422"<<endl;
break;

case AV_PIX_FMT_YUV444P:
va_format = VA_RT_FORMAT_YUV444;
cout <<"format found... YUV444"<<endl;

break;

case PIX_FMT_YUV411P:
va_format = VA_RT_FORMAT_YUV411;
cout <<"format found... YUV411"<<endl;
break;


case AV_PIX_FMT_RGB565LE:
va_format = VA_RT_FORMAT_RGB16;
cout <<"format found... RGB16"<<endl;
break;

case AV_PIX_FMT_RGBA:
va_format = VA_RT_FORMAT_RGB32;
cout <<"format found... RGB32"<<endl;
break;

default:
va_format = -1;
break;
}

if (profile >= 0 && va_format >=0) {
//get_display(x11_display);	

//va_dpy = vaGetDisplay(display);

vaapi_context *vc = new vaapi_context();
//va_dpy = vaGetDisplay(x11_display);
//va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);
//if(va_status == VA_STATUS_SUCCESS){
//  cout <<"VAAPI initialized successfully..."<<endl;


va_status = vaQueryConfigEntrypoints(va_dpy, (VAProfile)profile, entrypoints, &num_entrypoints);
cout <<"No of entry points = "<<num_entrypoints<<endl;

if(va_status == VA_STATUS_SUCCESS){
  cout <<"Entry Point Queryed Successfully..."<<endl;

  for (vld_entrypoint = 0; vld_entrypoint < num_entrypoints; vld_entrypoint++) {
        if (entrypoints[vld_entrypoint] == VAEntrypointVLD)
            break;
    }

 if (vld_entrypoint == num_entrypoints) {
      cout <<"VLD entrypoint not found..."<<endl;
      vaTerminate(va_dpy);
     return -1; 

    }else{
      cout <<"VLD entrypoint located at:"<<vld_entrypoint<<endl;
/////////////////////////////////////////////////////////////////////
    VAConfigAttrib attrib;
    VAConfigID config_id;
    VAContextID context_id;
    //VABufferID pic_param_buf,iqmatrix_buf,slice_param_buf,slice_data_buf;

    
    int putsurface=0;

attrib.type = VAConfigAttribRTFormat;
    vaGetConfigAttributes(va_dpy, (VAProfile)profile, VAEntrypointVLD,
                          &attrib, 1);


 if ((attrib.value & VA_RT_FORMAT_YUV420) == 0) {
        cout << "no desired YUV420 RT format found..."<<endl;
        return -1;  
    }
    


 va_status = vaCreateConfig(va_dpy, (VAProfile)profile, VAEntrypointVLD,
                              &attrib, 1,&config_id);

if(va_status == VA_STATUS_SUCCESS){
  cout <<"Config ID found..."<<endl;
}

surface_id = (VASurfaceID *)malloc(sizeof(VASurfaceID)*20);


va_status = vaCreateSurfaces(va_dpy,va_format, avctx->width,avctx->height,surface_id, 20,NULL, 0);


if(va_status == VA_STATUS_SUCCESS){
  cout <<"VA surface successfully created..."<<endl;
}else{
  cout <<"VA surface not created successfully..."<<endl;
  return -1;  
}


va_status = vaCreateContext(va_dpy, config_id,avctx->coded_width,avctx->coded_height,VA_PROGRESSIVE,
                               surface_id,
                               20,
                               &context_id);

if(va_status == VA_STATUS_SUCCESS){
  cout <<"VA context successfully created..."<<endl;
}else{
  cout <<"VA context not created successfully..."<<endl;
  return -1;  
}

vc->display = va_dpy;
vc->config_id = config_id;
vc->context_id = context_id;

avctx->opaque = (void *)surface_id;
avctx->hwaccel_context = vc;
cout <<"display = "<<vc->display<<endl;
cout <<"config_id = "<<config_id<<endl;
cout <<"context_id = "<<context_id<<endl;
cout <<"------------------------------------------------------"<<endl;
/////////////////////////////////////////////////////////////////////

avctx->get_format =  get_format_vaapi;
avctx->get_buffer2 = get_buffer;
avctx->draw_horiz_band = NULL;
avctx->slice_flags = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;
return 0;
}




}
//}
}

return -1;	
}





enum AVPixelFormat vaapi::get_format_vaapi(struct AVCodecContext *s, const enum AVPixelFormat *fmt){

cout <<"--------------------------------------------------------------------"<<endl;
cout <<"Negotiating the pixelFormat..."<<endl;
cout <<"--------------------------------------------------------------------"<<endl;

return PIX_FMT_VAAPI_VLD;
}

//////////////////////////////////////
void vaapi::release_buffer(void *opaque, uint8_t *data)
{
VASurfaceID *surface = (VASurfaceID *)data;
printf("- Released Surface ID: %04x\n",surface);
}


int vaapi::get_buffer(struct AVCodecContext *avctx, AVFrame *pic,int flags)
{

//VASurfaceID *surface_id2 = surface_id1;
VASurfaceID *surface_id = (VASurfaceID *)avctx->opaque;
void *surface = (void *)(uintptr_t)surface_id[surf_id];
surf_id = surf_id + 1;
if(surf_id > 19){
surf_id = 0;
}


cout <<"Surface ID:"<<surf_id<<endl;
printf("- Created Surface ID: %04x\n",surface);

    pic->data[0]        = (uint8_t *)surface;
    pic->data[1]        = NULL;
    pic->data[2]        = NULL;
    pic->data[3]        = (uint8_t *)surface;

    VAStatus va_status;

    pic->buf[0] = av_buffer_create((uint8_t *)surface,sizeof((uint8_t *)surface),release_buffer,NULL,AV_BUFFER_FLAG_READONLY);
    return 0;
}



VAImageFormat * get_vaformat(enum AVPixelFormat fmt,VADisplay* va_dpy){

VAStatus va_status;
pixel_format pixformat = avpixfmt2pixformat(fmt);
int fourcc_code = get_fourcc_code(pixformat);
int x;
VAImageFormat *vaformat;
int maxformats = vaMaxNumImageFormats (va_dpy);
int totalformats;
cout <<"Maximum No of Formats Supported:"<<maxformats<<endl;
VAImageFormat *formatlist = (VAImageFormat *)malloc(sizeof(VAImageFormat) * maxformats);
vaQueryImageFormats (va_dpy,formatlist,&totalformats);
cout <<"Total No of Formats:"<<totalformats<<endl;

if(fourcc_code != -1){
for(x = 0;x < totalformats;x ++){
vaformat = &formatlist[x];
if(vaformat->fourcc == fourcc_code){
printf("FourCC Code: %04x\n",vaformat->fourcc);
return vaformat;
}
}



}else{
cout <<"FourCC Not Code Found..."<<endl;
}

return NULL; 
}


/*
vaapi_upload::vaapi_upload(VADisplay* display){
dpy = display;
}

vaapi_upload::~vaapi_upload(){
VAStatus va_status; 
//free((void *)vidbuffer1);

va_status = vaUnmapBuffer (dpy,image1->buf);  

if(va_status == VA_STATUS_SUCCESS){
va_status = vaDestroyImage (dpy,image1->image_id);
if(va_status == VA_STATUS_SUCCESS){
va_status = vaDestroySurfaces(dpy,&surfaceid1,1);
if(va_status == VA_STATUS_SUCCESS){
}else{
cout <<"Error Unloading VAAPI upload class..."<<endl;  
}
}
}
}

int vaapi_upload::init(int width,int height,VAImageFormat *vaformat){
VAStatus va_status;  
h = height;
w = width;
va_status = vaCreateSurfaces(dpy,VA_RT_FORMAT_YUV420,width,height,&surfaceid1, 1,NULL, 0);

if(va_status == VA_STATUS_SUCCESS){
  cout <<"VA surface successfully created..."<<endl;
image1 = (VAImage *)malloc(sizeof(VAImage)); 
va_status = vaCreateImage (dpy,vaformat,width,height,image1);
if(va_status == VA_STATUS_SUCCESS){
  cout <<"VA Image created..."<<endl;

va_status = vaMapBuffer (dpy,image1->buf,(void **)&vidbuffer1);
if(va_status == VA_STATUS_SUCCESS)
return 0;
else
return -1;
}

}

return -1;
}


VASurfaceID vaapi_upload::upload(uint8_t ** data){
VAStatus va_status;  

cout <<w<<" - "<<h<<" - "<<image1->offsets[0]<<" - "<<image1->offsets[1]<<" - "<<image1->offsets[2]<<"  - "<<image1->data_size<<endl;
memcpy(vidbuffer1,data[0],image1->offsets[0]);
memcpy(vidbuffer1+image1->offsets[1],data[1],image1->offsets[2] - image1->offsets[1]);
memcpy(vidbuffer1+image1->offsets[2],data[2],image1->data_size - image1->offsets[2]);

va_status = vaPutImage (dpy,surfaceid1,image1->image_id,0,0,w,h,0,0,w,h);
if(va_status == VA_STATUS_SUCCESS)
return surfaceid1;
else
return (VASurfaceID)NULL;

}
*/




///////////////////////// Video Acceleration /////////////////////////




