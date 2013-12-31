#include "internels.h"
//#include <SDL/SDL.h>

// Video Threasd


int internel_decoder::getdecodedvideoframe(stream_context *sc,AVFrame *vidframe){
int vidframefinished;
AVPacket packet;
av_init_packet(&packet);
int len = 0;
//while(true){
//cout <<"New Loop..."<<endl;
//pthread_mutex_lock(&sc->demuxlock);
//pthread_mutex_unlock(&sc->demuxlock);
//if(sc->pausetoggle == 1)
//return -2;  

//if(sc->pausetoggle == 1){
//return -2;  
//}

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


if(sc->endthread == 1 && sc->videobuffer.size() == 0){
  cout <<" Video Decoding Ended..."<<endl;
return -1;
}

if(sc->pausetoggle == 1){
return -2;  
}


//cout <<"I am here.."<<endl;
if(sc->endthread != 1){
pthread_mutex_lock(&sc->decodelock1);
pthread_cond_wait(&sc->decodecond1, &sc->decodelock1);
pthread_mutex_unlock(&sc->decodelock1);
pthread_cond_signal(&sc->demuxcond);
}
//cout <<"I have crossed this..."<<endl;


if(sc->endthread == 1 && sc->videobuffer.size() == 0){
  cout <<" Video Decoding Ended..."<<endl;
return -1;
}  

if(sc->stop == 1){
cout <<"Stop Signal Received by Video Thread..."<<endl;
pthread_cond_broadcast(&sc->demuxcond);
  return -1;
}

if(sc->pausetoggle == 1){
return -2;	
}


}


//pthread_mutex_unlock(&sc->videolock);

pthread_mutex_lock(&sc->videolock);
packet = sc->videobuffer.front();
sc->videobuffer.pop();
pthread_mutex_unlock(&sc->videolock); 

//packet.flags = AV_PKT_FLAG_KEY;

/*
if(sc->videoctx->hwaccel == NULL){
  cout <<"hardware accelerator not set..."<<endl;
}else{
  cout <<"hardware accelerator - "<<sc->videoctx->hwaccel->name<<" - "<<sc->videoctx->hwaccel->pix_fmt<<" - "<<AV_PIX_FMT_VAAPI_VLD<<endl;
}

if(sc->videoctx->hwaccel_context == NULL){
  cout <<"VAPPI context not found..."<<endl;
}else{
  cout <<"VAAPI context found..."<<endl;
vaapi_context * vc = (vaapi_context *)sc->videoctx->hwaccel_context;
cout <<"display = "<<vc->display<<endl;

}
*/

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


void *internel_decoder::videoplayback(void *arg){
stream_context *sc = (stream_context *)arg;  
AVFrame *vidframe = avcodec_alloc_frame(); 
//avcodec_get_frame_defaults (vidframe);


////////////////////////////////////////////////////////////////
/*
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
*/  
////////////////////////////////////////////////////////////////

double delay;
int fc=0;
int ret;
double duration;
double video_decode_delay = 0;
int ts_diff;

/*
SDL_LockYUVOverlay(surf);

sc->vout->data[0] = surf->pixels[0];
sc->vout->data[1] = surf->pixels[2];
sc->vout->data[2] = surf->pixels[1];

sc->vout->linesize[0] = surf->pitches[0];
sc->vout->linesize[1] = surf->pitches[2];
sc->vout->linesize[2] = surf->pitches[1];


SDL_UnlockYUVOverlay(surf);
*/
////////////////////////////////////////////////////////////////

//pthread_cond_broadcast(&sc->videoframeupdate);
VAImage *image1;
VASurfaceID surfaceid1;
uint8_t *vidbuffer1;
//vaapi_upload *va = new vaapi_upload((VADisplay*)sc->display);

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
//cout <<"Video Decoder Status:"<<ret<<endl;


if(ret == -1){
break;
}else if(ret == -2){
goto videopause;
}else if(ret == 0){


//cout <<"Hello..."<<endl;
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


//if(sc->video_accel_mode == 1 && fc == 0){
//if(va->init(vidframe->width,vidframe->height,sc->vaformat) == -1)
//break;
//}


if(fc == 0){
//if(sc->fc == 0){
if(sc->videopts < 0){
sc->masterclock->settime(sc->start_time);	
}else{	
sc->masterclock->settime(sc->videopts);	
cout <<"videopts:"<<sc->videopts<<endl;
}
sc->masterclock->reset();
//}

fc = 1;
//sc->fc = 1;
ts_diff = 0;
}else{
ts_diff = av_compare_ts(av_frame_get_best_effort_timestamp(vidframe),sc->pFormatCtx->streams[sc->videostream]->time_base,sc->masterclock->gettime() * AV_TIME_BASE,AV_TIME_BASE_Q);
}


//cout <<"Video PTS:"<<sc->videopts<<" - Audio PTS:"<<sc->audiopts<<" - Master Clock:"<<sc->masterclock->gettime()<<" | "<<sc->audiobuffer.size()<<" - "<<sc->videobuffer.size()<<endl;  
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

if(sc->streamtype == stream_livesource){
sc->masterclock->settime(sc->videopts);
sc->masterclock->reset();
}

 pthread_yield();
}else{



if(delay < 0)
delay = 0;

av_usleep(delay);
//////////////// Video Acceleration //////////////// 

//////////////// Video Acceleration ////////////////


if(sc->video_accel_mode == 0){
sws_scale(sc->convert_ctx,vidframe->data,vidframe->linesize,0,vidframe->height,sc->vidframe1->data,sc->vidframe1->linesize);
sc->video_callback(sc->videoctx->hwaccel_context,(void**)sc->vidframe1->data,sc->vidframe1->linesize,sc->videopts,sc->opaque);
cout <<"coded width:"<<vidframe->linesize[0]<<" - coded height:"<<sc->videoctx->coded_height<<endl;

//}else if(sc->video_accel_mode == 1){

//surfaceid1 = va->upload(vidframe->extended_data);
//vaapi_context *vc = new vaapi_context();
//vc->display = sc->display;
//printf("- Surface ID1: %04x\n",sc->surfaceid1);
//printf("- VA Display1: %04x\n",sc->va_dpy);

//sc->video_callback(vc,(void**)&surfaceid1,sc->vidframe1->linesize,sc->videopts,sc->opaque);

}else if(sc->video_accel_mode == 1){

vaapi_context *vc = (vaapi_context*)sc->videoctx->hwaccel_context;
sc->video_callback(sc->videoctx->hwaccel_context,(void**)vidframe->data,sc->vidframe1->linesize,sc->videopts,sc->opaque);

}




//pthread_mutex_lock(&sc->video_seek_status_lock);
if(sc->videoseek == 1){
  sc->videoseek = 0;
}
//pthread_mutex_unlock(&sc->video_seek_status_lock);

//sc->status = MP_PLAYING;
//put_status(MP_PLAYING,sc);


}
//}

//}
}else{

  cout <<"Releasing CPU time for video thread..."<<pthread_yield()<<endl;	
}

//
}



pthread_cond_broadcast(&sc->decodecond);

pthread_mutex_lock(&sc->demuxpauselock);
sc->demuxpausetoggle = 0;
pthread_mutex_unlock(&sc->demuxpauselock);
pthread_cond_broadcast(&sc->demuxpausecond); 

pthread_mutex_lock(&sc->end_status_lock2);
sc->end_videothread = 1;
pthread_mutex_unlock(&sc->end_status_lock2);

pthread_mutex_lock(&sc->video_seek_status_lock);
sc->videoseek = 1;
pthread_mutex_unlock(&sc->video_seek_status_lock);

av_free(vidframe);
//if(sc->video_accel_mode == 1){
//delete(va);
//}



cout <<"loop break"<<endl;
//sc->status = MP_STOP; 
//sc->stop = 1;
pthread_exit(NULL);  
}

