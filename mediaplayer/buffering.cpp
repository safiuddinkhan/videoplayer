#include "internels.h"

void internel_decoder::empty_buffers(stream_context *sc){
//cout <<"yes"<<endl;  
AVPacket packet;


if(sc->videostream != -1){
while(true){


if(sc->videobuffer.size() == 0 || sc->videobuffer.empty())
break;


//usleep(300);
pthread_mutex_lock(&sc->videolock);
packet = sc->videobuffer.front();
sc->videobuffer.pop();
pthread_mutex_unlock(&sc->videolock);
av_free_packet(&packet);
}
}

if(sc->audiostream != -1){

while(true){

  if(sc->audiobuffer.size() == 0 || sc->audiobuffer.empty())
    break;
//usleep(300);
  pthread_mutex_lock(&sc->audiolock);
  packet = sc->audiobuffer.front();
  sc->audiobuffer.pop();
  pthread_mutex_unlock(&sc->audiolock);
av_free_packet(&packet);

}
}


}



int internel_decoder::init_buffering(int min_videobuffersize,int min_audiobuffersize,stream_context *streamcontext){
stream_context *sc = streamcontext;
int mode;

if(sc->videostream != -1 && sc->audiostream != -1)
mode = (sc->audiobuffer.size() < min_videobuffersize && sc->videobuffer.size() < min_audiobuffersize);  

 if(sc->videostream == -1)
mode = (sc->audiobuffer.size() < min_audiobuffersize); 

if(sc->audiostream == -1)
mode = (sc->videobuffer.size() < min_videobuffersize); 

if(sc->is_mp3){
  mode = (sc->audiobuffer.size() < min_audiobuffersize);
}

if(mode){

cout <<"---------------------"<<endl; 
cout <<"Buffer Low...."<<endl;
cout <<"---------------------"<<endl; 
//sc->status = MP_PAUSE;
put_status(MP_PAUSE,sc);
// Store current time of master clock 
sc->oldcurrenttime = sc->masterclock->gettime();
sc->masterclock->settime(sc->masterclock->gettime());
// Send pause signal to all threads
pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 1;
pthread_mutex_unlock(&sc->pauselock);

if(sc->stop == 1){
  return -1;
}

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

/////////////////////////////
if(sc->stop == 1){
  return -1;
}
/////////////////////////////


timediff = av_gettime();

if(sc->videostream != -1){
pthread_cond_broadcast(&sc->decodecond1);  
pthread_mutex_lock(&sc->pauselock);
if(sc->videopause == 0) pthread_cond_wait(&sc->video_waitcond, &sc->pauselock);
pthread_mutex_unlock(&sc->pauselock);
}

timediff = av_gettime() - timediff;
cout << "Pass - 2 - "<<timediff<<endl;

/////////////////////////////
if(sc->stop == 1){
  return -1;
}
/////////////////////////////

cout <<sc->audiopause<<" - "<<sc->videopause<<endl;
//////////////////////////////////////////////
//pause_over = 1;
return 1;
}else{
return 0;
}


}


int internel_decoder::detect_pause(stream_context *streamcontext){
stream_context *sc = streamcontext;


mediaplayer_status status;
pthread_mutex_lock(&sc->status_lock);
status = sc->status;
pthread_mutex_unlock(&sc->status_lock);

if(status == MP_PLAYING){
return 0;
}

if(status == MP_PAUSE){
return 1;
}

return -1;
}


int internel_decoder::end_buffering(int videobuffersize,int audiobuffersize,stream_context *streamcontext){
stream_context *sc = streamcontext;
int mode1;

if(sc->videostream != -1 && sc->audiostream != -1)
mode1 = (sc->audiobuffer.size() > audiobuffersize || sc->videobuffer.size() > videobuffersize); 

if(sc->videostream == -1)
mode1 = (sc->audiobuffer.size() > audiobuffersize); 

if(sc->audiostream == -1)
mode1 = (sc->videobuffer.size() > videobuffersize); 

if(mode1){

// Send single to all threads to unpause
pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 0;
pthread_mutex_unlock(&sc->pauselock);
pthread_cond_broadcast(&sc->pausecond);
// reset master clock to the stored time before pause

while(true){
if(sc->audiopause == 0 && sc->videopause == 0)
  break;
av_usleep(1);
}

cout <<"------------------"<<endl;
cout <<"Buffering Done...."<<endl;
cout <<"------------------"<<endl;

sc->masterclock->reset();
//sc->status = MP_PLAYING;
put_status(MP_PLAYING,sc);
return 1;
}


return 0;
}


int internel_decoder::buffer_limit(int videobuffersize,int audiobuffersize,stream_context *streamcontext){
stream_context *sc = streamcontext;
int mode2;
if(sc->videostream != -1 && sc->audiostream != -1)
mode2 = (sc->audiobuffer.size() > audiobuffersize || sc->videobuffer.size() > videobuffersize);  

if(sc->videostream == -1)
mode2 = (sc->audiobuffer.size() > audiobuffersize); 

if(sc->audiostream == -1)
mode2 = (sc->videobuffer.size() > videobuffersize); 


if(mode2){
av_read_pause(sc->pFormatCtx);
pthread_mutex_lock(&sc->demuxlock);
pthread_cond_wait(&sc->demuxcond, &sc->demuxlock);
pthread_mutex_unlock(&sc->demuxlock);
av_read_play(sc->pFormatCtx); 
return 1;
}

return 0;
}
