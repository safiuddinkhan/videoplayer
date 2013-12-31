#include "internels.h"

int get_fourcc_code(pixel_format pixformat){
switch(pixformat){
case MP_YV12:
return 0x32315659;

case MP_I420:
return 0x30323449;

case MP_UYVY:
return 0x59565955;

case MP_YUY2:
return 0x32595559;

case MP_RGBA:
return 0x41424752;

case MP_BGRA:
return 0x41524742;

case MP_RGBX:
return 0x58424752;

case MP_BGRX:
return 0x58524742;

default:
return -1;
}



}

pixel_format avpixfmt2pixformat(AVPixelFormat pixelformat){
switch(pixelformat){
//case AV_PIX_FMT_YUV420P:
//return MP_YV12;

case AV_PIX_FMT_YUV420P:
return MP_I420;

case AV_PIX_FMT_UYVY422:
return MP_UYVY;

case AV_PIX_FMT_YUYV422:
return MP_YUY2;

case AV_PIX_FMT_RGB565LE:
return MP_RGB16;

case AV_PIX_FMT_RGB565BE:
return MP_BGR16;

case PIX_FMT_RGB24:
return MP_RGB24;

case PIX_FMT_BGR24:
return MP_BGR24;

case AV_PIX_FMT_RGBA:
return MP_RGBA;

case AV_PIX_FMT_BGRA:
return MP_BGRA;

case AV_PIX_FMT_RGB0:
return MP_RGBX;

case AV_PIX_FMT_BGR0:
return MP_BGRX;

default:
return MP_NONE;
}


}


AVPixelFormat get_pixelformat(pixel_format pixformat){

switch(pixformat){
case MP_YV12:
return AV_PIX_FMT_YUV420P;

case MP_I420:
return AV_PIX_FMT_YUV420P;

case MP_UYVY:
return AV_PIX_FMT_UYVY422;

case MP_YUY2:
return AV_PIX_FMT_YUYV422;

case MP_RGB16:
return AV_PIX_FMT_RGB565LE;

case MP_BGR16:
return AV_PIX_FMT_RGB565BE;

case MP_RGB24:
return PIX_FMT_RGB24;

case MP_BGR24:
return PIX_FMT_BGR24;

case MP_RGBA:
return AV_PIX_FMT_RGBA;

case MP_BGRA:
return AV_PIX_FMT_BGRA;

case MP_RGBX:
return AV_PIX_FMT_RGB0;

case MP_BGRX:
return AV_PIX_FMT_BGR0;

default:
return AV_PIX_FMT_NONE;
}

}


int internel_decoder::findandopencodec(AVCodecContext *pCodecCtx){


 AVCodec *codec;
codec=avcodec_find_decoder(pCodecCtx->codec_id);
cout <<"Codec:"<<codec->name<<endl;
if(codec==NULL) {
cout <<"Error Unsupported Codec..."<<endl;
   // sc->status = MP_ERROR;
   put_status(MP_ERROR,sc); 
  return -1; 
}

//avcodec_get_context_defaults3(pCodecCtx,codec);  

int ret;
pthread_mutex_lock(&sc->codec_open_lock);
ret = avcodec_open2(pCodecCtx, codec,NULL);
pthread_mutex_unlock(&sc->codec_open_lock); 
if(ret < 0){
  cout <<"Error Opening Codec..."<<endl;
 put_status(MP_ERROR,sc); 
  return -1;   
}

//sleep(1);
cout <<"Codec Opened..."<<endl; 
return 0;  
}



stream_type internel_decoder::stream_detector(char *url){
char *protocol;
char *p;
p = strchr(url,':');
if(p == NULL)
return stream_none;

int loc = p - url;
cout <<"location of collen :"<<loc<<endl;
protocol = new char[loc];
memcpy (protocol,url,loc);
protocol[loc] = '\0';
if(strcmp(protocol,"http") == 0 || strcmp(protocol,"https") == 0 || strcmp(protocol,"rtp") == 0 || strcmp(protocol,"rtsp") == 0 || strcmp(protocol,"rtmp") == 0 || strcmp(protocol,"mms") == 0 || strcmp(protocol,"mmsh") == 0 || strcmp(protocol,"ftp") == 0){
  return stream_network;
}else if(strcmp(protocol,"v4l2") == 0 || strcmp(protocol,"v4l") == 0 || strcmp(protocol,"video4linux2") == 0 || strcmp(protocol,"video4linux") == 0){
  return stream_livesource;
}else if(strcmp(protocol,"file") == 0){
  return stream_localfile;
}else{
  return stream_none;
}

}


AVInputFormat * internel_decoder::init_v4l(char *url){
AVInputFormat *format = NULL;
char *p;
//char *path;
p = strchr(url,':');
if(p == NULL)
return NULL;
int len = strlen(url);
int loc = p - url;
char path[len - loc];
if(url[loc + 1] == '/' && url[loc+2] == '/' && url[loc+3] == '/'){

//path = new char[len - loc];
memcpy (path,p+3,len - loc);
path[len - loc] = '\0';
cout <<path<<endl;


while (format = av_iformat_next(format)){
  cout <<" - "<<format->name<<" - "<<endl;
  if(strcmp(format->name,"video4linux2,v4l2")==0)
    break;
}

strcpy(url,path);
}else{
  cout <<"Illegal V4l URL..."<<endl;
  return NULL;
}  

return format;
}


int internel_decoder::get_first_streams(stream_context *streamcontext){
stream_context *sc = streamcontext;
AVInputFormat *format =  sc->pFormatCtx->iformat;


///////////////////////////////////////////////////////////////////

int i;


int vs = 0;
int as = 0;
cout <<"No of Streams:"<<sc->pFormatCtx->nb_streams<<endl;
for(i=0; i<sc->pFormatCtx->nb_streams; i++){
  switch(sc->pFormatCtx->streams[i]->codec->codec_type){
case AVMEDIA_TYPE_VIDEO:
if(vs == 0){
sc->videostream=i;
AVCodec *pCodec;
pCodec=avcodec_find_decoder(sc->pFormatCtx->streams[sc->videostream]->codec->codec_id);

if(strcmp(format->name,"mp3") == 0){
sc->videostream = -1;  /// if file is a mp3 file then skip videostream
}else{

//// workaround to display png images 
if(strcmp(pCodec->name,"png") == 0){

sc->videoctx = avcodec_alloc_context3(NULL);
sc->videoctx->height = sc->pFormatCtx->streams[sc->videostream]->codec->height; 
sc->videoctx->width = sc->pFormatCtx->streams[sc->videostream]->codec->width;
sc->videoctx->pix_fmt = sc->pFormatCtx->streams[sc->videostream]->codec->pix_fmt;
cout <<"Codec:"<<pCodec->name<<endl;
cout <<"bpp:"<<sc->videoctx->bits_per_coded_sample<<endl; 
avcodec_open2(sc->videoctx, pCodec,NULL);

}else{
//// If no png then open codec normally 

sc->videoctx = sc->pFormatCtx->streams[i]->codec; 
cout <<"Original Height:"<<sc->pFormatCtx->streams[sc->videostream]->codec->height<<" - Original Width:"<<sc->videoctx->width<<endl;
/////////////////// Hardware Acceleration ///////////////////
if(sc->display == NULL){
sc->video_accel_mode = 0;
}else{
int ret;
ret = activate_vaapi_decoding(sc->videoctx,(VADisplay *)sc->display);
if(ret == -1){
//sc->vaformat = get_vaformat(sc->videoctx->pix_fmt,(VADisplay*)sc->display);
//if(sc->vaformat != NULL){
//sc->video_accel_mode = 1;
//}else{
sc->video_accel_mode = 0;
//}
}else{
sc->video_accel_mode = 1;
}
}
/////////////////// Hardware Acceleration ///////////////////

cout <<"height before = "<<sc->pFormatCtx->streams[i]->codec->height<<" - "<<sc->pFormatCtx->streams[i]->codec->width<<endl;
findandopencodec(sc->videoctx);
}
}
//sc->pFormatCtx->streams[i]->discard = AVDISCARD_DEFAULT;


}

vs = vs + 1;
break;
case AVMEDIA_TYPE_AUDIO:
if(as == 0){
sc->audiostream=i;
sc->audioctx = sc->pFormatCtx->streams[i]->codec;
findandopencodec(sc->audioctx);
//sc->pFormatCtx->streams[i]->discard = AVDISCARD_DEFAULT;
}
as = as + 1;
break;
  }
    
}

 cout <<"Total No of Audio Stream:"<<as<<" - Total No of Video Stream:"<<vs<<endl;
if(sc->videostream<0){
 cout <<"No Video Stream Found..."<<endl; 
 }else{
 cout <<"Video Stream Found"<<endl;

cout <<"video time base:"<<av_q2d(sc->pFormatCtx->streams[sc->videostream]->time_base)<<endl;
sc->videobasetime = av_q2d(sc->pFormatCtx->streams[sc->videostream]->time_base);

sc->height = sc->pFormatCtx->streams[sc->videostream]->codec->height;
sc->width = sc->pFormatCtx->streams[sc->videostream]->codec->width;
sc->aspect_ratio_num = sc->videoctx->sample_aspect_ratio.num;
sc->aspect_ratio_den = sc->videoctx->sample_aspect_ratio.den;
sc->pixelformat = sc->pFormatCtx->streams[sc->videostream]->codec->pix_fmt;
}

if(sc->audiostream<0){
 cout <<"No Audio Stream Found..."<<endl; 
 }else{
 cout <<"Audio Stream Found"<<endl; 

sc->audiobasetime = av_q2d(sc->pFormatCtx->streams[sc->audiostream]->time_base);
cout <<"audio time base:"<<av_q2d(sc->pFormatCtx->streams[sc->audiostream]->time_base)<<endl;

sc->channels = sc->audioctx->channels;
sc->samplerate = sc->audioctx->sample_rate;
//sc->sampleformat = sc->audioctx->sample_fmt;

}




//////////////////////////////////////////////////////////////////////////



 return 0; 
}

int internel_decoder::get_attached_images(char* url,stream_context *streamcontext){
stream_context *sc = streamcontext;
AVInputFormat *format =  sc->pFormatCtx->iformat;

if(strcmp(format->name,"mp3") == 0){
sc->is_mp3 = 1;
}

/// If attached image of mp3 is not detected by ffmpeg then we use taglib to extract the attached image

if(strcmp(format->name,"mp3") == 0 && sc->videostream == -1){

 TagLib::MPEG::File mp3File(url);
    Tag * mp3Tag;
    FrameList listOfMp3Frames;
    AttachedPictureFrame * pictureFrame;
AVPacket packet;
//av_init_packet(&packet);

    mp3Tag= mp3File.ID3v2Tag();
    if(mp3Tag)
    {
        listOfMp3Frames = mp3Tag->frameListMap()["APIC"];
        if(!listOfMp3Frames.isEmpty())
        {
            FrameList::ConstIterator it= listOfMp3Frames.begin();
         //   for(; it != listOfMp3Frames.end() ; it++)
         //   {
pictureFrame = static_cast<AttachedPictureFrame *> (*it);
//cout <<"mime-type:"<<pictureFrame->mimeType()<<endl;
AVCodec *codec;

const char *mime_attached = pictureFrame->mimeType().toCString(false);
const CodecMime *mime = ff_id3v2_mime_tags;

 while (mime->id != AV_CODEC_ID_NONE) {
  if (!av_strncasecmp(mime->str, mime_attached, sizeof(mime_attached))) {
codec = avcodec_find_decoder(mime->id);
cout <<"mime-type:"<<mime->str<<endl;
  break;
  }
  mime++;
  }
if(mime->id != AV_CODEC_ID_NONE){


av_new_packet (&packet, pictureFrame->picture().size());
memcpy(packet.data,pictureFrame->picture().data(),pictureFrame->picture().size());
    // packet.data = (uint8_t*)pictureFrame->picture().data();
     packet.size = pictureFrame->picture().size();
     packet.flags = AV_PKT_FLAG_KEY;

pthread_mutex_lock(&sc->videolock);
sc->videobuffer.push(packet);
pthread_mutex_unlock(&sc->videolock);


sc->videoctx = avcodec_alloc_context3(NULL);

avcodec_open2(sc->videoctx,codec, NULL);
sc->attachedimage = 1;
AVFrame *frame = avcodec_alloc_frame();
int decode_ok;

int len;
while(true){
  len = avcodec_decode_video2(sc->videoctx, frame, &decode_ok, &packet);
if(decode_ok || len < 0){
break;
}
}


if(len < 0){
sc->videoctx->width = 0;
sc->videoctx->height = 0;
sc->height = 0;
sc->width = 0;
av_free(frame);
return -1;
}else{
cout <<frame->height<<" - "<<frame->width<<endl;
sc->videoctx->width = frame->width;
sc->videoctx->height = frame->height;
sc->height = frame->height;
sc->width = frame->width;
av_free(frame);
}
}else{
return -1;
}
//break;
          //  }
} else{          
return -1;
}

} else {
return -1;
}


}


return 0;
}


int internel_decoder::check_stream_seekable(stream_context *streamcontext){
stream_context *sc = streamcontext;
if(sc->pFormatCtx->pb != NULL){
if(sc->pFormatCtx->pb->seekable == 0){  
return -1;
}else{

int ret = av_seek_frame(sc->pFormatCtx, -1, sc->pFormatCtx->start_time , 0);

if(ret < 0)
return -1;
else  
return 0;
}
}
}


int internel_decoder::load_file(char *filename,stream_context *streamcontext){
/////////////////////////////
char *p;
//char *path;
p = strchr(filename,':');
if(p == NULL)
return -1;
int len = strlen(filename);
int loc = p - filename;
char path[len - loc];

if(filename[loc + 1] == '/' && filename[loc+2] == '/' && filename[loc+3] == '/'){
memcpy (path,p+3,len - loc);
path[len - loc] = '\0';
}else{
  return -1;
}  

/////////////////////////////

stream_context *sc = streamcontext;
AVInputFormat *format = NULL;

AVDictionary *options = NULL;
//av_dict_set(&options, "video_size", "640x480", 0);
//av_dict_set(&options, "pixel_format", "rgb24", 0);

if(avformat_open_input(&sc->pFormatCtx, path, format, &options)!=0){
return -1; 
}

if(avformat_find_stream_info(sc->pFormatCtx,NULL)<0){
return -1; 
}

get_first_streams(sc);
get_attached_images(path,sc);

sc->start_time = (double)(sc->pFormatCtx->start_time / AV_TIME_BASE);
sc->totalduration = (double)(sc->pFormatCtx->duration / AV_TIME_BASE);


// Check if stream/file is seekable 
int seekable = check_stream_seekable(sc);
if(seekable < 0)
sc->is_seekable = 0;
else  
sc->is_seekable = 1;


return 0;
}



int internel_decoder::load_network_stream(char *url,stream_context *streamcontext){
stream_context *sc = streamcontext;
AVInputFormat *format = NULL;

AVDictionary *options = NULL;
//av_dict_set(&options, "video_size", "640x480", 0);
//av_dict_set(&options, "pixel_format", "rgb24", 0);

if(avformat_open_input(&sc->pFormatCtx, url, format, &options)!=0){
return -1; 
}

  sc->pFormatCtx->probesize = 5*1024*1024;
  sc->pFormatCtx->max_analyze_duration = 60*AV_TIME_BASE;

if(avformat_find_stream_info(sc->pFormatCtx,NULL)<0){
return -1; 
}


get_first_streams(sc);
get_attached_images(url,sc);
sc->start_time = (double)(sc->pFormatCtx->start_time / AV_TIME_BASE);
sc->totalduration = (double)(sc->pFormatCtx->duration / AV_TIME_BASE);
// Check if stream/file is seekable 
int seekable = check_stream_seekable(sc);
if(seekable < 0)
sc->is_seekable = 0;
else  
sc->is_seekable = 1;


return 0;
}




int internel_decoder::load_live_videsource(char *url,stream_context *streamcontext){
stream_context *sc = streamcontext;
AVInputFormat *format = NULL;

format = init_v4l(url);
if(format == NULL)
return - 1;


AVDictionary *options = NULL;
//av_dict_set(&options, "video_size", "640x480", 0);
//av_dict_set(&options, "pixel_format", "rgb24", 0);

if(avformat_open_input(&sc->pFormatCtx, url, format, &options)!=0){
return -1; 
}

if(avformat_find_stream_info(sc->pFormatCtx,NULL)<0){
return -1; 
}


get_first_streams(sc);
sc->totalduration = (double)(sc->pFormatCtx->duration / AV_TIME_BASE);
sc->start_time = (double)(sc->pFormatCtx->start_time / AV_TIME_BASE);
sc->is_seekable = 0;

return 0;
}

