#include "mediaplayer.h"
#include <ao/ao.h>

/// Pause Audio Callback
void mediaplayer::audio_pause_callback(int toggle){

}

int mediaplayer::getdecodedaudioframe(stream_context *sc,AVFrame *audioframe){
AVPacket packet;
int audioframefinished;
int len;
//while(true){
//pthread_mutex_lock(&sc->demuxlock);
//pthread_mutex_unlock(&sc->demuxlock);

pthread_cond_signal(&sc->demuxcond);

if(sc->stop == 1){
cout <<"Stop Signal Received by Audio Thread..."<<endl;
pthread_cond_broadcast(&sc->demuxcond);
  return -1;
}

if(sc->endthread == 1){
  cout <<" Audio Decoding Ended..."<<endl;
 return -1;
}



if(sc->audiobuffer.size() == 0){

sc->audio_flag = 1;

if(sc->endthread == 1){
  cout <<" Audio Decoding Ended..."<<endl;
 return -1;
}

  cout << "start..."<<endl;
pthread_mutex_lock(&sc->decodelock);
pthread_cond_wait(&sc->decodecond, &sc->decodelock);
pthread_mutex_unlock(&sc->decodelock);
pthread_cond_signal(&sc->demuxcond);
  cout <<"end..."<<endl;

if(sc->stop == 1){
cout <<"Stop Signal Received by Audio Thread..."<<endl;
pthread_cond_broadcast(&sc->demuxcond);
  return -1;
}


if(sc->endthread == 1){
  cout <<" Audio Decoding Ended..."<<endl;
 return -1;
}

if(sc->pausetoggle == 1)
return -2;

}


pthread_mutex_lock(&sc->audiolock);
packet = sc->audiobuffer.front();
sc->audiobuffer.pop();
pthread_mutex_unlock(&sc->audiolock); 


len = avcodec_decode_audio4(sc->audioctx,audioframe,&audioframefinished ,&packet);
av_free_packet(&packet);
if(len < 0){
return -3;  
}




if(audioframefinished){
return 0;
}


return 1;
}
//audio thread
void *mediaplayer::audioplayback(void *arg){
stream_context *sc = (stream_context *)arg;
  


//AVPacket packet;
int audioframefinished;
AVFrame *audioframe = avcodec_alloc_frame(); 

int curr_ch = 2;
int default_sample_size = 4096;
/// Initializing Audio Sampler
struct SwrContext *swr_ctx;
cout << "Channel Layout:"<<sc->audioctx->channel_layout<<" - "<<av_get_default_channel_layout(sc->audioctx->channels)<<" - "<<AV_CH_LAYOUT_MONO<<endl;
cout << "Channels:"<<sc->audioctx->channels<<endl;
cout << "Sample Rate:"<<sc->audioctx->sample_rate<<endl;
swr_ctx = swr_alloc_set_opts(NULL,av_get_default_channel_layout(curr_ch),AV_SAMPLE_FMT_S16,sc->audioctx->sample_rate,av_get_default_channel_layout(sc->audioctx->channels)
  ,sc->audioctx->sample_fmt,sc->audioctx->sample_rate,0,NULL);
if(!swr_ctx){
 // sc->status = MP_ERROR;
  put_status(MP_ERROR,sc);
  pthread_exit(NULL);  
}

int dst_nb_samples;
int ret;
 if ((ret = swr_init(swr_ctx)) < 0) {
 // sc->status = MP_ERROR;
 put_status(MP_ERROR,sc);
  pthread_exit(NULL);  
  }
///
//void *exit;
int64_t delay = 0;
uint8_t *dst_data;
//audioarg aa;

av_samples_alloc(&dst_data, NULL, curr_ch, default_sample_size, AV_SAMPLE_FMT_S16, 0);


int fc =0;
int endaudiothread = 0;
////////////////////////////////////////////////////
audio *out;
int default_driver;
ao_device *device;
ao_initialize();
default_driver = ao_default_driver_id();
ao_sample_format *format = new ao_sample_format();

format->bits = 16;
format->channels = curr_ch;
format->rate = sc->audioctx->sample_rate;
format->byte_format = AO_FMT_NATIVE;
//format.matrix = (char *)"L,R";

device = ao_open_live(default_driver, format, NULL);
  if (device == NULL) {
  cout <<"Sound Error..."<<endl;
    }

////////////////////////////////////////////////////
    int fc1 = 0;
double duration;
double audio_decode_delay = 0;
int ts_diff;
while(true){
audiopos:
pthread_mutex_lock(&sc->pauselock);
if(sc->pausetoggle == 1){ 
//sc->status = MP_PAUSE;   
sc->audiopause = 1;
pthread_cond_broadcast(&sc->audio_waitcond);  
pthread_cond_wait(&sc->pausecond, &sc->pauselock);
sc->audiopause = 0;
//sc->status = MP_PLAYING; 
}
pthread_mutex_unlock(&sc->pauselock);



ret = getdecodedaudioframe(sc,audioframe);
if(ret == -1){
break;
}else if(ret == -2){
goto audiopos;
}else if(ret == 0){
//timediff = av_gettime() - timediff;

//if(fc == 2){
//sc->aseek_ok = 0;
//fc = 0;
//}
//fc = fc + 1;

sc->audiopts = ((double)av_frame_get_best_effort_timestamp(audioframe) * (double)sc->audiobasetime);
duration = ((double)av_frame_get_pkt_duration(audioframe) * (double)sc->audiobasetime)* AV_TIME_BASE;

///////////////////////////////////////////////////////////////////

if(sc->networkstream){
if(sc->audio_flag == 1){
sc->masterclock->settime(sc->audiopts - 0.04); 
sc->masterclock->reset();
sc->audio_flag = 0;
}
}

///////////////////////////////////////////////////////////////////

if(fc == 0){
if(sc->audiopts < 0){
sc->masterclock->settime(sc->start_time); 
}else{  
sc->masterclock->settime(sc->audiopts); 
}
sc->masterclock->reset();
 fc = 1;
ts_diff = 0;
}else{
ts_diff = av_compare_ts(av_frame_get_best_effort_timestamp(audioframe),sc->pFormatCtx->streams[sc->audiostream]->time_base,sc->masterclock->gettime() * AV_TIME_BASE,AV_TIME_BASE_Q);
}


delay = (sc->audiopts - sc->masterclock->gettime()) * AV_TIME_BASE;

///////////////////////////////////////////////////////////////////

if(sc->networkstream){
if(delay > 1*AV_TIME_BASE && duration < 1*AV_TIME_BASE){
if(sc->videostream == -1){  
sc->masterclock->settime(sc->audiopts);
sc->masterclock->reset();
cout <<"Syncing Master Clock with Audio PTS..."<<endl;
}
//delay = 0;
ts_diff = -1;
//fc = 1;
}
}

///////////////////////////////////////////////////////////////////


if(ts_diff == -1){
//delay = 0;
pthread_yield();
cout <<"Audio Frame Dropped..."<<endl;
}else{


if(sc->videostream == -1){
  cout <<"Video PTS:"<<sc->videopts<<" - Audio PTS:"<<sc->audiopts<<" - Master Clock:"<<sc->masterclock->gettime()<<" | "<<sc->audiobuffer.size()<<" - "<<sc->videobuffer.size()<<endl;  
}





dst_nb_samples = av_rescale_rnd(audioframe->nb_samples, sc->audioctx->sample_rate, sc->audioctx->sample_rate, AV_ROUND_UP);
if(dst_nb_samples > default_sample_size){
av_freep(&dst_data); 
av_samples_alloc(&dst_data, NULL, curr_ch, dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
default_sample_size = dst_nb_samples;
}

ret = swr_convert(swr_ctx,&dst_data, dst_nb_samples, (const uint8_t **)audioframe->extended_data, audioframe->nb_samples);
if(ret < 0){
  //sc->status = MP_ERROR;
 put_status(MP_ERROR,sc);
  pthread_exit(NULL);  
}




if(delay < 0)
delay = 0;
av_usleep(delay);
 
//sc->status = MP_PLAYING;
ao_play(device, (char *)dst_data, ret*curr_ch*(format->bits/8));

pthread_mutex_lock(&sc->audio_seek_status_lock);
if(sc->audioseek == 1){
  sc->audioseek = 0;
}
pthread_mutex_unlock(&sc->audio_seek_status_lock);

put_status(MP_PLAYING,sc);




///
}
///



}else{
  
  cout <<"Releasing CPU time for audio thread..."<<pthread_yield()<<endl;
}


//pthread_mutex_lock(&sc->audioframelock);
//pthread_cond_broadcast(&sc->audioframeupdate);
//pthread_mutex_unlock(&sc->audioframelock);

//
}

pthread_mutex_lock(&sc->end_status_lock1);
sc->end_audiothread = 1;
pthread_mutex_unlock(&sc->end_status_lock1);

pthread_cond_broadcast(&sc->decodecond1);
av_freep(&dst_data); 
swr_free (&swr_ctx);
cout <<"audio loop break"<<endl;
av_free(audioframe);
//sc->stop = 1;
ao_close(device);
//free(format);
//sc->audioctx->skip_frame = AVDISCARD_ALL ;
avcodec_flush_buffers(sc->audioctx);
 ao_shutdown();
//sc->status = MP_STOP; 
pthread_exit(NULL);  
}

/*
audio *mediaplayer::get_playback_audioframe(){
pthread_mutex_lock(&sc->audioframelock);
pthread_cond_wait(&sc->audioframeupdate, &sc->audioframelock);
pthread_mutex_unlock(&sc->audioframelock);
return sc->aout;
}
*/
