#include "mediaplayer.h"
#include <SDL/SDL.h>


// Video Threasd


int mediaplayer::getdecodedvideoframe(stream_context *sc,AVFrame *vidframe){
int vidframefinished;
AVPacket packet;
av_init_packet(&packet);
int len = 0;
//while(true){
//cout <<"New Loop..."<<endl;
//pthread_mutex_lock(&sc->demuxlock);
//pthread_mutex_unlock(&sc->demuxlock);
pthread_cond_signal(&sc->demuxcond);


if(sc->stop == 1){
cout <<"Stop Signal Received by Video Thread..."<<endl;
pthread_cond_broadcast(&sc->demuxcond);
  return -1;
}

/*
if(sc->endthread == 1){
  cout <<" Video Decoding Ended..."<<endl;
return -1;
}
*/

//pthread_mutex_lock(&sc->videolock);
if(sc->videobuffer.size() == 0){

sc->video_flag = 1;

if(sc->endthread == 1){
  cout <<" Video Decoding Ended..."<<endl;
return -1;
}

cout <<"I am here.."<<endl;
pthread_mutex_lock(&sc->decodelock1);
pthread_cond_wait(&sc->decodecond1, &sc->decodelock1);
pthread_mutex_unlock(&sc->decodelock1);
pthread_cond_signal(&sc->demuxcond);
cout <<"I have crossed this..."<<endl;

if(sc->stop == 1){
cout <<"Stop Signal Received by Video Thread..."<<endl;
pthread_cond_broadcast(&sc->demuxcond);
  return -1;
}

if(sc->endthread == 1){
  cout <<" Video Decoding Ended..."<<endl;
return -1;
}

if(sc->pausetoggle == 1)
return -2;	


}

//pthread_mutex_unlock(&sc->videolock);

pthread_mutex_lock(&sc->videolock);
packet = sc->videobuffer.front();
sc->videobuffer.pop();
pthread_mutex_unlock(&sc->videolock); 

//packet.flags = AV_PKT_FLAG_KEY;


len = avcodec_decode_video2(sc->videoctx, vidframe, &vidframefinished,&packet);
av_free_packet(&packet);


if(len < 0){
return -3;
}



if(vidframefinished){
return 0;
}

// if (packet.data) {
//  packet.size -= len;
//  packet.data += len;
//  }

//pthread_mutex_lock(&sc->demuxlock);
//pthread_cond_broadcast(&sc->demuxcond);
//pthread_mutex_unlock(&sc->demuxlock);

//}

return 1;
}


void *mediaplayer::videoplayback(void *arg){
stream_context *sc = (stream_context *)arg;  
AVFrame *vidframe = avcodec_alloc_frame(); 
avcodec_get_frame_defaults (vidframe);
////////////////////////////////////////////////////////////////

SDL_Overlay *surf;
SDL_Rect rect; 
cout <<"line1"<<endl;
cout <<sc->videoctx->width<<" - "<<sc->videoctx->height<<endl;
surf = SDL_CreateYUVOverlay(sc->videoctx->width, sc->videoctx->height, SDL_YV12_OVERLAY, sc->screen);
cout <<"line2"<<endl;
   rect.x = 0;
   rect.y = 0;
  rect.w = sc->videoctx->width;
  rect.h = sc->videoctx->height;
////////////////////////////////////////////////////////////////

double delay;
int fc=0;
int ret;
double duration;
double video_decode_delay = 0;
int ts_diff;
////////////////////////////////////////////////////////////////
int numBytes;
uint8_t *vidbuffer;
AVFrame *vidframe1;
int height;
int width;
SwsContext * convert_ctx;

AVPixelFormat dstfmt = get_avpixelformat(sc->pixel_format);
if(dstfmt == AV_PIX_FMT_NONE){
  dstfmt = AV_PIX_FMT_RGB24;
  cout <<"Cannot Find your FourCC code falling to RGB24 pixel format"<<endl;
}

cout <<sc->pixel_format<<endl;

vidframe1 = avcodec_alloc_frame();
height = sc->videoctx->height;
width = sc->videoctx->width;

numBytes=avpicture_get_size(dstfmt ,width,height);
vidbuffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
avpicture_fill((AVPicture *)vidframe1, vidbuffer, dstfmt ,width, height);

//mm = mp;

convert_ctx = sws_getContext(width, height, sc->videoctx->pix_fmt,
                                   width, height,dstfmt,
                                   SWS_BICUBIC, NULL, NULL, NULL );


////////////////////////////////////////////////////////////////
SDL_LockYUVOverlay(surf);

vidframe1->data[0] = surf->pixels[0];
vidframe1->data[1] = surf->pixels[2];
vidframe1->data[2] = surf->pixels[1];

vidframe1->linesize[0] = surf->pitches[0];
vidframe1->linesize[1] = surf->pitches[2];
vidframe1->linesize[2] = surf->pitches[1];


SDL_UnlockYUVOverlay(surf);
////////////////////////////////////////////////////////////////



while(true){

videopause:
pthread_mutex_lock(&sc->pauselock);
if(sc->pausetoggle == 1){
  cout <<"Video Thread Paused..."<<endl;
//sc->status = MP_PAUSE;   
sc->videopause = 1;
pthread_cond_broadcast(&sc->video_waitcond);
pthread_cond_wait(&sc->pausecond, &sc->pauselock);
sc->videopause = 0;
//sc->status = MP_PLAYING;
cout <<"Video Thread unPaused..."<<endl;
}
pthread_mutex_unlock(&sc->pauselock);



ret = getdecodedvideoframe(sc,vidframe);
cout <<"Video Decoder Status:"<<ret<<endl;

if(ret == -1){
break;
}else if(ret == -2){
goto videopause;
}else if(ret == 0){
sws_scale(convert_ctx,vidframe->data,vidframe->linesize,0,height,vidframe1->data,vidframe1->linesize);

cout <<"Hello..."<<endl;
sc->videopts = (double)av_frame_get_best_effort_timestamp(vidframe) * (double)sc->videobasetime;
duration = ((double)av_frame_get_pkt_duration(vidframe) * (double)sc->videobasetime)* AV_TIME_BASE;

///////////////////////////////////////////////////////

if(sc->networkstream){
if(sc->video_flag == 1){
sc->masterclock->settime(sc->videopts - 0.04);	
sc->masterclock->reset();
sc->video_flag = 0;
}
}

///////////////////////////////////////////////////////





if(fc == 0){
if(sc->videopts < 0){
sc->masterclock->settime(sc->start_time);	
}else{	
sc->masterclock->settime(sc->videopts);	
cout <<"videopts:"<<sc->videopts<<endl;
}
sc->masterclock->reset();
fc = 1;
ts_diff = 0;
}else{
ts_diff = av_compare_ts(av_frame_get_best_effort_timestamp(vidframe),sc->pFormatCtx->streams[sc->videostream]->time_base,sc->masterclock->gettime() * AV_TIME_BASE,AV_TIME_BASE_Q);
}


cout <<"Video PTS:"<<sc->videopts<<" - Audio PTS:"<<sc->audiopts<<" - Master Clock:"<<sc->masterclock->gettime()<<" | "<<sc->audiobuffer.size()<<" - "<<sc->videobuffer.size()<<endl;  
//cout <<"ts_diff ="<<ts_diff<<endl;

delay = (sc->videopts - sc->masterclock->gettime()) * AV_TIME_BASE; 

///////////////////////////////////////////////////////

if(sc->networkstream){
if(delay > 1*AV_TIME_BASE && duration < 1*AV_TIME_BASE){
sc->masterclock->settime(sc->videopts);
sc->masterclock->reset();
cout <<"Syncing Master Clock with Video PTS..."<<endl;
delay = 0;
ts_diff = -1;
//fc = 1;
}
}
///////////////////////////////////////////////////////

if(ts_diff == -1){
cout <<"Video Frame Dropped..."<<endl;
 pthread_yield();
}else{



if(delay < 0)
delay = 0;
av_usleep(delay);
//cout <<"Delay:"<<delay<<" - Duration:"<<duration<<endl;

/*
sc->vout1->data = vidframe->data;
sc->vout1->linesize = vidframe->linesize; 
sc->vout1->pts = sc->videopts;
if(sc->cc == NULL)
sc->vout = sc->vout1;
else
sc->vout = sc->cc->convert(sc->vout1);
*/




SDL_DisplayYUVOverlay(surf, &rect);

pthread_mutex_lock(&sc->video_seek_status_lock);
if(sc->videoseek == 1){
  sc->videoseek = 0;
}
pthread_mutex_unlock(&sc->video_seek_status_lock);

//sc->status = MP_PLAYING;
put_status(MP_PLAYING,sc);


}
//pthread_mutex_lock(&sc->videoframelock);
//pthread_cond_broadcast(&sc->videoframeupdate);
//pthread_mutex_unlock(&sc->videoframelock);
//}

//}
}else{

  cout <<"Releasing CPU time for video thread..."<<pthread_yield()<<endl;	
}

//
}

pthread_mutex_lock(&sc->end_status_lock2);
sc->end_videothread = 1;
pthread_mutex_unlock(&sc->end_status_lock2);

pthread_cond_broadcast(&sc->decodecond);
av_free(vidframe);
av_free(vidframe1);
sws_freeContext (convert_ctx);
av_freep(&vidbuffer); 
cout <<"loop break"<<endl;
//sc->status = MP_STOP; 
//sc->stop = 1;
pthread_exit(NULL);  
}

video *mediaplayer::get_playback_videoframe(){
pthread_mutex_lock(&sc->videoframelock);
pthread_cond_wait(&sc->videoframeupdate, &sc->videoframelock);
pthread_mutex_unlock(&sc->videoframelock);
//sc->videoctx->skip_frame = AVDISCARD_ALL ;
avcodec_flush_buffers(sc->videoctx);

return sc->vout;
}

