#include "mediaplayer.h"
uint8_t *vidbuffer;



void init_all(){
 
}


metadata_entry *mediaplayer::get_metadata(){

if(getstatus() == MP_ERROR)
return NULL;

metadata_entry *entry = new metadata_entry();

sc->tag = av_dict_get(sc->pFormatCtx->metadata, "", sc->tag, AV_DICT_IGNORE_SUFFIX);
if(sc->tag == NULL){
return NULL;
}
entry->key = sc->tag->key;
entry->value = sc->tag->value;
return entry;

}









mediaplayer::mediaplayer(char *url , void *display){
//char *file,char *fourcc_code,Display *display


//cout <<avdevice_configuration ()<<endl;

avcodec_register_all();
avdevice_register_all();
avformat_network_init(); 
av_register_all(); 

height = 0;
width = 0;

sc = new stream_context();
///////////////////////////////////////////////
//sc->x11_dpy = (Display *)display;

sc->display = display;
sc->is_mp3 = 0;
put_status(MP_STOP,sc);

pthread_cond_init (&sc->demuxcond, NULL);
pthread_cond_init (&sc->pausecond, NULL);
pthread_cond_init (&sc->audio_waitcond, NULL);
pthread_cond_init (&sc->video_waitcond, NULL);
pthread_cond_init (&sc->demux_waitcond, NULL);
pthread_cond_init (&sc->demuxpausecond, NULL);
pthread_cond_init (&sc->decodecond, NULL);
pthread_cond_init (&sc->decodecond1, NULL);
///////////////////////////////////////////////
sc->tag = NULL;
sc->pFormatCtx = avformat_alloc_context();
sc->attachedimage = 0;
sc->videostream =-1;
sc->audiostream =-1;
sc->masterclock = new media_clock(); 
sc->networkstream = 0;
//sc->v4l_device = 0;
sc->streamtype = stream_none;
sc->end_audiothread = 1;
sc->end_videothread = 1;
sc->endthread = 1;
sc->stop = 0;
sc->video_accel_mode = 0;


stream_type st = stream_detector(url);
sc->streamtype = st;  
streamtype = sc->streamtype;

int ret;

if(st == stream_network){  
ret = load_network_stream(url,sc);
sc->networkstream = 1; 
}else if(st == stream_localfile){
ret = load_file(url,sc);
}else if(st == stream_livesource){
ret = load_live_videsource(url,sc);
}else if(st == stream_none){
ret = -1;
}

if(ret < 0){
put_status(MP_ERROR,sc);
streamtype = stream_none;
height = 0;
width = 0;
samplerate = 0;
channels = 0;
aspect_ratio_num = 0;
aspect_ratio_den = 0;
}else{
height = sc->height;
width = sc->width;
samplerate = sc->samplerate;
channels = sc->samplerate;
aspect_ratio_num = sc->samplerate;
aspect_ratio_den = sc->samplerate;
}




}



int mediaplayer::set_pixel_format(pixel_format pixformat){
if(getstatus() == MP_ERROR)
return -1;



////////////////////////////////////////////////////////////////



if(sc->videostream != -1 || sc->attachedimage == 1){
////////////////////////////////////////////////////////////////
cout <<"Hardware Acceleration Mode:"<<sc->video_accel_mode<<endl;


if(sc->video_accel_mode == 0){

//sc->pixel_format = fourcc_code;
AVPixelFormat dstfmt = get_pixelformat(pixformat);
if(dstfmt == AV_PIX_FMT_NONE){
  dstfmt = AV_PIX_FMT_RGB24;
  cout <<"Cannot Find your FourCC code falling to RGB24 pixel format"<<endl;
}
sc->vidframe1 = avcodec_alloc_frame();
////////////////////////////////////////////////////////////////////////
if(sc->videoctx->pix_fmt == AV_PIX_FMT_NONE){
cout <<"Pixel Format Not Found..."<<endl; 
put_status(MP_ERROR,sc); 
}else{
int numBytes;
cout <<"Source Pixel Format:"<<sc->videoctx->pix_fmt<<endl;
cout <<"Destination Pixel Format:"<<dstfmt<<endl;
numBytes=avpicture_get_size(dstfmt ,width,height);
cout <<"Size of Video Buffer:"<<numBytes<<endl;
vidbuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
//VAStatus va_status;
//va_status = vaMapBuffer ((VADisplay *)sc->display,va_image->buf,(void **)&vidbuffer);

avpicture_fill((AVPicture *)sc->vidframe1, vidbuffer, dstfmt ,width, height);
cout <<" ---- "<<sc->videoctx->pix_fmt<<" - "<<sc->videoctx->height<<" - "<<sc->videoctx->width<<endl;
sc->convert_ctx = sws_getContext(width, height, sc->videoctx->pix_fmt,
                                   width, height,dstfmt,
                                   SWS_BICUBIC, NULL, NULL, NULL );

}
////////////////////////////////////////////////////////////////
}

return 0;
}

return -1;
}


void mediaplayer::set_callbacks(void (*video_callback)(void *,void **,int *,double , void *),void (*audio_callback)(uint8_t *,int , double , void *),void * opaque){

cout <<"Callback function..."<<endl;

cout <<"---------------------------------------"<<endl;
switch(getstatus()){
case MP_PLAYING:
cout <<"Current Status: Playing..."<<endl;
break;
case MP_PAUSE:
cout <<"Current Status: Pause..."<<endl;
break;
case MP_SEEKING:
cout <<"Current Status: Seek..."<<endl;
break;
case MP_ERROR:
cout <<"Current Status: Error..."<<endl;
break;
case MP_STOP:
cout <<"Current Status: Stop..."<<endl;
break;
}
cout <<"---------------------------------------"<<endl;

cout <<sc->videostream<<" - "<<sc->attachedimage<<endl;

sc->opaque = opaque;
if(sc->audiostream != -1){
  sc->audio_callback = audio_callback;
}


if((sc->videostream != -1 || sc->attachedimage == 1) && getstatus() != MP_ERROR){
cout <<"executed.."<<endl;
sc->video_callback = video_callback;
//init_video(va_dpy,(void**)sc->vidframe1->data,sc->vidframe1->linesize,opaque);

}
cout <<"not exexuted..."<<endl;

}

int mediaplayer::play(){
////////// Video Acceleration ////////// 
//sc->surface_id = surface_id;
////////// Video Acceleration //////////

if(getstatus() == MP_ERROR)
return -1;

if(getstatus() == MP_STOP){

cout <<"---------------------------------------"<<endl;
switch(getstatus()){
case MP_PLAYING:
cout <<"Current Status: Playing..."<<endl;
break;
case MP_PAUSE:
cout <<"Current Status: Pause..."<<endl;
break;
case MP_SEEKING:
cout <<"Current Status: Seek..."<<endl;
break;
case MP_ERROR:
cout <<"Current Status: Error..."<<endl;
break;
case MP_STOP:
cout <<"Current Status: Stop..."<<endl;
break;
}
cout <<"---------------------------------------"<<endl;

/*
while(true){
  cout <<"Waiting For All Threads to End..."<<endl;
  if(sc->end_audiothread == 1 && sc->end_videothread == 1 && sc->endthread == 1)
    break;
}
*/
sc->endthread = 0;
sc->pausetoggle = 0;

sc->audiopause = 0;
sc->videopause = 0;
sc->demuxpause = 0;

sc->audiopts = 0;
sc->videopts = 0;
sc->seek = 0;

sc->seektime = 0;
sc->videopts = 0;
sc->audiopts = 0;
sc->videoseek = 0;
sc->audioseek = 0;

sc->video_flag = 0;
sc->audio_flag = 0;

sc->end_audiothread = 0;
sc->end_videothread = 0;
sc->demuxpausetoggle = 0;
sc->fc = 0;

//sc->masterclock->settime(sc->start_time); 
//sc->masterclock->reset();
//if(sc->status == MP_STOP)
//sc->stop = 1;


sc->stop = 0;
//sc->status = MP_PLAYING;  
put_status(MP_PLAYING,sc);
  empty_buffers(sc);
if(sc->videostream != -1)
avcodec_flush_buffers(sc->videoctx);
if(sc->audiostream != -1)
avcodec_flush_buffers(sc->audioctx);


if(sc->is_seekable){


// Rewind Video to Start
  cout <<"start time:"<<sc->pFormatCtx->start_time<<endl;
int ret = av_seek_frame(sc->pFormatCtx, -1, sc->pFormatCtx->start_time , AVSEEK_FLAG_ANY);
if(ret < 0){
//sc->masterclock->settime(sc->start_time);
//sc->masterclock->reset();
cout <<"Seeking not supported for this video..."<<endl;
}
}


if(sc->videostream != -1 || sc->attachedimage == 1)
  pthread_create(&sc->videothread,NULL,internel_decoder::videoplayback,sc);  
 
if(sc->audiostream != -1)
  pthread_create(&sc->audiothread,NULL,internel_decoder::audioplayback,sc); 

  pthread_create(&sc->demuxerthread,NULL,internel_decoder::demuxer,sc);



return 0;
}

return -1;
}

int mediaplayer::stop(){
//endthread == 1;
if(getstatus() == MP_STOP){
return -1;
}
  void *exit;
  sc->stop = 1;

pthread_cond_broadcast(&sc->demuxcond);  
pthread_cond_broadcast(&sc->video_waitcond);
pthread_cond_broadcast(&sc->audio_waitcond);
pthread_cond_broadcast(&sc->pausecond);
pthread_cond_broadcast(&sc->demuxpausecond);
pthread_cond_broadcast(&sc->decodecond1);
pthread_cond_broadcast(&sc->decodecond);


pthread_join(sc->demuxerthread,&exit);


//pthread_cond_broadcast(&sc->demuxcond);
//pthread_cond_broadcast(&sc->video_waitcond);
//pthread_cond_broadcast(&sc->audio_waitcond);
pthread_cond_broadcast(&sc->pausecond);
pthread_cond_broadcast(&sc->decodecond1);
pthread_cond_broadcast(&sc->decodecond);


if(sc->audiostream != -1)
pthread_join(sc->audiothread,&exit);  

if(sc->videostream != -1 || sc->attachedimage == 1)
pthread_join(sc->videothread,&exit);  

//sc->status = MP_STOP;
put_status(MP_STOP,sc);

cout <<"Media Stopped"<<endl;
//pthread_cond_broadcast(&sc->videoframeupdate); 
 // sc->stop = 0;

//avcodec_close(sc->videoctx);
//avcodec_close(sc->audioctx);
//avformat_close_input(&sc->pFormatCtx); 
return 0;

}

int mediaplayer::pause(){
if(getstatus() == MP_PAUSE || getstatus() == MP_PLAYING){  
if(sc->pausetoggle == 1){
// Send single to all threads to unpause
pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 0;
pthread_mutex_unlock(&sc->pauselock);
pthread_cond_broadcast(&sc->pausecond);
// reset master clock to the stored time before pause
sc->masterclock->reset();
//sc->status = MP_PLAYING;
put_status(MP_PLAYING,sc);
}else{

// Send pause signal to all threads
pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 1;
pthread_mutex_unlock(&sc->pauselock);



//////////////////////////////////////////////
/*
int64_t timediff;
timediff = av_gettime();

if(sc->audiostream != -1){
pthread_cond_broadcast(&sc->decodecond);
pthread_mutex_lock(&sc->pauselock);
if(sc->audiopause == 0) pthread_cond_wait(&sc->audio_waitcond, &sc->pauselock);
pthread_mutex_unlock(&sc->pauselock);
}

timediff = av_gettime() - timediff;

cout << "Pass - 1 - "<<timediff<<endl;

timediff = av_gettime();

if(sc->videostream != -1){
pthread_cond_broadcast(&sc->decodecond1);  
pthread_mutex_lock(&sc->pauselock);
if(sc->videopause == 0) pthread_cond_wait(&sc->video_waitcond, &sc->pauselock);
pthread_mutex_unlock(&sc->pauselock);
}

timediff = av_gettime() - timediff;

cout << "Pass - 2 - "<<timediff<<endl;

cout <<sc->audiopause<<" - "<<sc->videopause<<endl;
*/
//////////////////////////////////////////////

// Store current time of master clock 
sc->oldcurrenttime = sc->masterclock->gettime();
sc->masterclock->settime(sc->masterclock->gettime());
//sc->status = MP_PAUSE; 
put_status(MP_PAUSE,sc);

}
return 0;
}else{
return -1;  
}


}



void mediaplayer::seek(double timestamp){
if(sc->is_seekable == 1){  
if(getstatus() == MP_PLAYING){
int mode;
int mode1;
if(sc->videostream == -1){
  pthread_mutex_lock(&sc->audio_seek_status_lock);
  mode = sc->audioseek == 0;
  pthread_mutex_unlock(&sc->audio_seek_status_lock);
}

if(sc->audiostream == -1){
  pthread_mutex_lock(&sc->video_seek_status_lock);
  mode = sc->videoseek == 0;
  pthread_mutex_unlock(&sc->video_seek_status_lock);

}

if(sc->audioseek != -1 && sc->videoseek != -1){
pthread_mutex_lock(&sc->video_seek_status_lock);
mode1 = sc->videoseek == 0;
pthread_mutex_unlock(&sc->video_seek_status_lock);


pthread_mutex_lock(&sc->audio_seek_status_lock);
mode = mode1 && sc->audioseek == 0;
pthread_mutex_unlock(&sc->audio_seek_status_lock);
}

if(sc->seek == 0 && mode){// && sc->videoseek == 0 && sc->audioseek == 0){ 
sc->seektime = timestamp;
sc->seek = 1;
seek_stream(sc->seektime,sc);
//////////////////////////////////////////////
cout <<"Seeking Over...."<<endl;
//////////////////////////////////////////////
put_status(MP_PLAYING,sc);
sc->seek = 0;
}


}
}else{
  cout <<"Seeking Not Supported For This Stream..."<<endl;
}

}



double mediaplayer::getpos(){ 
if(getstatus() == MP_STOP)
return 0;

if(getstatus() == MP_PAUSE)
return sc->oldcurrenttime;
  

return sc->masterclock->gettime();
}

double mediaplayer::getduration(){
  return sc->totalduration;
}

mediaplayer_status mediaplayer::getstatus(){
mediaplayer_status s;
pthread_mutex_lock(&sc->status_lock);
s = sc->status;
pthread_mutex_unlock(&sc->status_lock);

  return s;
}

void internel_decoder::put_status(mediaplayer_status status,stream_context *sc){
pthread_mutex_lock(&sc->status_lock);
sc->status = status;
pthread_mutex_unlock(&sc->status_lock);
}


//char * mediaplayer::get_pixelformat(){
// return get_fourcc_code(sc->pixelformat);
//}

mediaplayer::~mediaplayer(){

if(sc->videoctx->hwaccel_context != NULL){
deinit_vaapi(sc->videoctx);
}

delete(sc->masterclock);
sws_freeContext (sc->convert_ctx);
av_free(sc->vidframe1);
//av_free(sc->vidframe);
av_freep(&vidbuffer); 
//delete(sc->vout1);
//delete(sc->cc);
//av_free(sc->vidframe);
avcodec_close(sc->videoctx);
avcodec_close(sc->audioctx);
avformat_close_input(&sc->pFormatCtx); 



}


//////////////////////////////////////////////////////////////////
/// Function Copied From FFMPEG Source Code
 int av_strncasecmp(const char *a, const char *b, size_t n)
 {
  const char *end = a + n;
  uint8_t c1, c2;
  do {
  c1 = av_tolower(*a++);
  c2 = av_tolower(*b++);
  } while (a < end && c1 && c1 == c2);
  return c1 - c2;
 }
//////////////////////////////////////////////////////////////////







