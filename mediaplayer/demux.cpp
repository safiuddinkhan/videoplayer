#include "internels.h"

void internel_decoder::demux_pause(stream_context *streamcontext){
stream_context *sc = streamcontext;

pthread_mutex_lock(&sc->demuxpauselock);
if(sc->demuxpausetoggle == 1){
  cout <<"Demuxer blocked..."<<endl; 
sc->demuxpause = 1;
pthread_cond_wait(&sc->demuxpausecond, &sc->demuxpauselock);
sc->demuxpause = 0;
cout <<"Demuxer unblocked..."<<endl;

}
pthread_mutex_unlock(&sc->demuxpauselock);

}

// Demuxer

void *internel_decoder::demuxer(void *arg){
stream_context *sc = (stream_context *)arg;



AVPacket packet;
int stream_buffer = 0;

int yes_pause = 0;
int fc = 0;

av_init_packet(&packet);
int max_videobuffer = 50;
int max_audiobuffer = 50;
int ret;

av_read_play(sc->pFormatCtx);


while(true){

if(sc->stop == 1){
  break;
}


ret = av_read_frame(sc->pFormatCtx, &packet);

if(ret < 0) 
break;


if(packet.stream_index==sc->videostream){
pthread_mutex_lock(&sc->videolock);
sc->videobuffer.push(packet);
pthread_mutex_unlock(&sc->videolock);

pthread_cond_broadcast(&sc->decodecond1); 

}else if(packet.stream_index==sc->audiostream){

pthread_mutex_lock(&sc->audiolock);
sc->audiobuffer.push(packet);
pthread_mutex_unlock(&sc->audiolock);

pthread_cond_broadcast(&sc->decodecond);

}else{
av_free_packet(&packet);
}



if(sc->networkstream){
///////////////////////////////////////////////////////////////////////////////////////////////////////
int64_t timediff; 
cout <<"Video Buffer:"<<sc->videobuffer.size()<<" - Audio Buffer:"<<sc->audiobuffer.size()<<endl;
demux_pause(sc);

int ret;

if(fc == 0){
ret = init_buffering(10,10,sc);
if(ret == 1)
  fc = 1;
else if(ret == -1)
  break;
}


ret = detect_pause(sc);
if(ret == 0){
  fc = 0;
  yes_pause = 0;
}else if(ret == 1){
  yes_pause = 1;
}else if(ret == -1){
  break;
}

cout <<"yes_pause:"<<yes_pause<<" - fc:"<<fc<<endl;

if(yes_pause == 1 && fc == 1){
ret = end_buffering(300,300,sc);
if(ret == 1){
yes_pause = 0;
fc = 0;
}
}


ret = buffer_limit(500,500,sc);
if(ret == 1)
fc = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////

}else{

//cout <<"Video Buffer:"<<sc->videobuffer.size()<<" - Audio Buffer:"<<sc->audiobuffer.size()<<endl;

buffer_limit(max_audiobuffer,max_videobuffer,sc);
demux_pause(sc);

}






}



sc->endthread = 1;


pthread_cond_broadcast(&sc->decodecond);
pthread_cond_broadcast(&sc->decodecond1);
pthread_cond_broadcast(&sc->pausecond);


void *exit;


if(sc->audiostream != -1)
pthread_join(sc->audiothread,&exit);  

if(sc->videostream != -1 || sc->attachedimage == 1)
pthread_join(sc->videothread,&exit);  
//sc->stop = 1;

cout <<"Demux Thread ended..."<<endl;
//sc->status = MP_STOP;
put_status(MP_STOP,sc);




pthread_exit(NULL);  

}
