#include "mediaplayer.h"
#include <SDL/SDL.h>


// Video Threasd


int mediaplayer::getdecodedvideoframe(stream_context *sc,AVFrame *vidframe){
int vidframefinished;
AVPacket packet;
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


if(sc->endthread == 1){
  cout <<" Video Decoding Ended..."<<endl;
return -1;
}

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

len = avcodec_decode_video2(sc->videoctx, vidframe, &vidframefinished,&packet);
av_free_packet(&packet);

cout <<"len:"<<len<<endl;
if(len < 0){
return -3;
}



if(vidframefinished){
return 0;
}

 //if (packet.data) {
 // packet.size -= len;
 // packet.data += len;
 // }

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
surf = SDL_CreateYUVOverlay(sc->videoctx->width, sc->videoctx->height, SDL_YV12_OVERLAY, sc->screen);
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


sc->vout1->data = vidframe->data;
sc->vout1->linesize = vidframe->linesize; 
sc->vout1->pts = sc->videopts;
if(sc->cc == NULL)
sc->vout = sc->vout1;
else
sc->vout = sc->cc->convert(sc->vout1);

SDL_LockYUVOverlay(surf);


//memcpy(surf->pixels[1],sc->vout->data[2],(sc->videoctx->height * surf->pitches[1]));

//   memcpy(data,out->data[0], out->linesize[0]*mp->height);
//   memcpy(data+(out->linesize[0]*mp->height),out->data[1], out->linesize[1]*mp->height);

//sc->vout->data[0] = surf->pixels[0];
//sc->vout->data[1] = surf->pixels[2];
//sc->vout->data[2] = surf->pixels[1];

memcpy(surf->pixels[0],sc->vout->data[0],(sc->videoctx->height * sc->videoctx->width));
memcpy(surf->pixels[1],sc->vout->data[2],(sc->videoctx->height * sc->videoctx->width)/4);
memcpy(surf->pixels[2],sc->vout->data[1],(sc->videoctx->height * sc->videoctx->width)/4);


sc->vout->linesize[0] = surf->pitches[0];
sc->vout->linesize[1] = surf->pitches[2];
sc->vout->linesize[2] = surf->pitches[1];


SDL_UnlockYUVOverlay(surf);
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

