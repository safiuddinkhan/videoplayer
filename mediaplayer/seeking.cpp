#include "internels.h"


int internel_decoder::seek_stream(double seektime,stream_context *sc){
double videopts = 0;
double audiopts = 0;
double firstpts = 0;
//double old_videopts = 0;
//double old_audiopts = 0;
int videokeyframe = 0;
int audiokeyframe = 0;



AVPacket packet;
int64_t vtimestamp;
//int64_t vtimestamp_min;
//int64_t vtimestamp_max;
int ret;
//double seekpoint = 0;
///////////////////////////////////////////////
//if(sc->seek == 1){ 
//cout <<"Hello 1..."<<endl;

//if(sc->seektime < 0)
//sc->seektime = 0;
int timediff;

 cout <<"Seeking..."<<endl;
if(sc->endthread == 1){
cout <<" Could Not Seek bcz demuxer has ended..."<<endl;
  return -1;
}



pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 1;
pthread_mutex_unlock(&sc->pauselock);
/*
if(sc->networkstream){
//cout <<"Hello network stream..."<<endl;
pthread_mutex_lock(&sc->demuxlock);
sc->demux_block = 1;
pthread_mutex_unlock(&sc->demuxlock);
}
*/
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

timediff = av_gettime();

pthread_cond_broadcast(&sc->demuxcond);  

pthread_mutex_lock(&sc->demuxpauselock);
sc->demuxpausetoggle = 1;
pthread_mutex_unlock(&sc->demuxpauselock);


pthread_mutex_lock(&sc->demuxpauselock);
if(sc->demuxpause == 0){
pthread_mutex_unlock(&sc->demuxpauselock);
while(true){

if(sc->endthread == 1){
cout <<" Could Not Seek bcz demuxer has ended..."<<endl;
  return -1;
}


//pthread_mutex_lock(&sc->demuxpauselock);
if(sc->demuxpause == 1)
  break;
//pthread_mutex_unlock(&sc->demuxpauselock);


pthread_cond_broadcast(&sc->demuxcond);  
cout <<"I am here..."<<endl;
}
pthread_mutex_unlock(&sc->demuxpauselock);

//pthread_mutex_lock(&sc->demuxpauselock);
//pthread_cond_wait(&sc->demux_waitcond, &sc->demuxpauselock);
//pthread_mutex_unlock(&sc->demuxpauselock);


}
pthread_mutex_unlock(&sc->demuxpauselock);

 // pthread_mutex_unlock(&sc->demuxlock);
timediff = av_gettime() - timediff;



cout << "Pass - 3 - "<<timediff<<endl;


cout <<sc->audiopause<<" - "<<sc->videopause<<" - "<<sc->demuxpause<<endl;

//sc->status = MP_SEEKING;
put_status(MP_SEEKING,sc);
//cout <<"Time Taken for pausing:"<<timediff<<endl;

//seektime = (videopts + sc->seektime);
//vtimestamp = (int64_t)(double)(seektime * AV_TIME_BASE);

//vtimestamp1 = (int64_t)(double)((audiopts + sc->seektime) / sc->audiobasetime);
//seekpoint = (sc->masterclock->gettime() + seektime);

if(sc->start_time >= 0){
if(seektime < sc->start_time)
seektime = sc->start_time;
}else{
if(seektime < 0)
seektime = 0;
}


cout <<"----------------------------------"<<endl;
cout <<"Seek Time Requested:"<<seektime<<endl;
cout <<"----------------------------------"<<endl;




  vtimestamp = seektime * AV_TIME_BASE;  

//old_videopts = videopts;
//old_audiopts = audiopts;
cout <<"Hello 2 ..."<<endl;

/////////////////////////////
//int64_t seek_target = is->seek_pos;
//            int64_t seek_min    = is->seek_rel > 0 ? seek_target - is->seek_rel + 2: INT64_MIN;
//            int64_t seek_max    = is->seek_rel < 0 ? seek_target - is->seek_rel - 2: INT64_MAX;
////////////////////////////
int flag;
//if(sc->seektime > sc->masterclock->gettime()){
//  flag = 0;
//}else{
  flag = AVSEEK_FLAG_BACKWARD;     
//}


timediff = av_gettime();
if(sc->videostream != -1)
avcodec_flush_buffers(sc->videoctx);
if(sc->audiostream != -1)
avcodec_flush_buffers(sc->audioctx);
//avio_flush(sc->pFormatCtx->pb);
timediff = av_gettime() - timediff;
cout <<"Time Taken for flushing LibAV buffers:"<<timediff<<endl;


timediff = av_gettime();
cout <<"time l crossed"<<endl;
ret = av_seek_frame(sc->pFormatCtx, -1, vtimestamp , flag);
cout <<"time l1 crossed"<<endl;
timediff = av_gettime() - timediff;
cout <<"Time Taken for seeking:"<<timediff<<endl;
//ret = avformat_seek_file(sc->pFormatCtx, -1,vtimestamp_min,vtimestamp,vtimestamp_max,flag); 



if(ret < 0){
   //sc->status = MP_ERROR;
cout <<"Seeking Error or EOF..."<<endl;

  pthread_mutex_lock(&sc->demuxpauselock);
sc->demuxpausetoggle = 0;
pthread_mutex_unlock(&sc->demuxpauselock);
pthread_cond_broadcast(&sc->demuxpausecond); 



pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 0;
pthread_mutex_unlock(&sc->pauselock);
pthread_cond_broadcast(&sc->pausecond);

  put_status(MP_ERROR,sc);
  return -1;
}


//sc->videoctx->skip_frame = AVDISCARD_DEFAULT;

/// Flush Internel Buffers
//int old_audio_buffer_size = sc->audiobuffer.size();
//int old_video_buffer_size = sc->videobuffer.size();
timediff = av_gettime();
empty_buffers(sc);
timediff = av_gettime() - timediff;
cout <<"Time Taken for clearing our buffers:"<<timediff<<endl;




///////////////////////////////////////////////////////////////////
/// Flushing video packets till we reach the key frame


sc->lastvideopts = -1;
sc->lastaudiopts = -1;
if(sc->videostream != -1){
 timediff = av_gettime();   
while(true){
//usleep(30000);
//usleep(300);

 if(av_read_frame(sc->pFormatCtx, &packet)<0)
break;




if(packet.stream_index==sc->videostream){
////////////
if(packet.pts == AV_NOPTS_VALUE){
  cout <<"PTS not found..."<<endl;
if(sc->lastvideopts == -1){
if(packet.dts == AV_NOPTS_VALUE)
videopts = -1;
else
videopts = packet.dts * sc->videobasetime;
}else{
videopts = sc->lastvideopts + (double)(packet.duration * sc->videobasetime);  
}

}else{
videopts = (double)(packet.pts * sc->videobasetime);
}
sc->lastvideopts = videopts;
////////////

videokeyframe = packet.flags;
cout <<"-- video pts:"<<videopts<<" - keyframe:"<<videokeyframe<<" - "<<(double)(packet.dts * sc->videobasetime)<<" - Size:"<<packet.size<<endl;
if(videokeyframe == AV_PKT_FLAG_KEY && videopts >= 0){
//cout <<" - Video Condition True - "<<endl; 
firstpts = videopts;  
pthread_mutex_lock(&sc->videolock);
sc->videobuffer.push(packet);
pthread_mutex_unlock(&sc->videolock);
break; 
}else{
av_free_packet(&packet);
}

}else if(packet.stream_index==sc->audiostream){
//cout <<"-- audio pts:"<<(double)(packet.pts * sc->audiobasetime)<<" - keyframe:"<<audiokeyframe<<" - "<<(double)(packet.dts * sc->audiobasetime)<<" - Size:"<<packet.size<<endl;

pthread_mutex_lock(&sc->audiolock);
sc->audiobuffer.push(packet);
pthread_mutex_unlock(&sc->audiolock);

}else{
  av_free_packet(&packet);
}  



}

timediff = av_gettime() - timediff;
cout <<"Time Taken for flushing extra video packets:"<<timediff<<endl;


///////////////////////////////////////////////////////////////////
/// flushing audiopackets till we reach desired audio frame
//sc->lastvideopts = -1;
sc->lastaudiopts = -1;

timediff = av_gettime();
while(true){
//usleep(300);

  if(sc->audiobuffer.size() == 0){

 while(sc->audiobuffer.size() == 0){   
//usleep(300);

 if(av_read_frame(sc->pFormatCtx, &packet)<0)
break;

if(packet.stream_index==sc->videostream){
pthread_mutex_lock(&sc->videolock);
sc->videobuffer.push(packet);
pthread_mutex_unlock(&sc->videolock);  
}else if(packet.stream_index==sc->audiostream){
pthread_mutex_lock(&sc->audiolock);
sc->audiobuffer.push(packet);
pthread_mutex_unlock(&sc->audiolock);  
}else{
  av_free_packet(&packet);
}

}

  if(sc->audiobuffer.size() == 0)
    break;


  }
//break;

  pthread_mutex_lock(&sc->audiolock);
  packet = sc->audiobuffer.front();
  pthread_mutex_unlock(&sc->audiolock);
  
////////////
if(packet.pts == AV_NOPTS_VALUE){
if(sc->lastaudiopts == -1){
if(packet.dts == AV_NOPTS_VALUE)
audiopts = -1;
else
audiopts = packet.dts * sc->audiobasetime;
}else{  
audiopts = sc->lastaudiopts + (double)(packet.duration * sc->audiobasetime);  
}
}else{
audiopts = (double)(packet.pts * sc->audiobasetime);
}
sc->lastaudiopts = audiopts;
////////////
audiokeyframe = packet.flags;



//cout << "Dumping Audio Packets:"<<audiopts<<" - "<<firstpts<<endl;  
if(firstpts >= audiopts && audiokeyframe == AV_PKT_FLAG_KEY){}else{
  break;
}

  sc->audiobuffer.pop();
  av_free_packet(&packet);


}

timediff = av_gettime() - timediff;
cout <<"Time Taken for flushing extra audio packets:"<<timediff<<endl;
///////////////////////////////////////////////////////////////////


}
//////////////////////////////////////////////

double newseektime;

if(sc->videostream != -1 && sc->audiostream != -1){

if(audiopts > firstpts){
  newseektime = firstpts;
}else{
  newseektime = audiopts;
}



}else if(sc->videostream == -1){
  newseektime = seektime;
}else if(sc->audiostream == -1){
  newseektime = firstpts;
}

cout <<"------------------------------------"<<endl;
cout <<"New Seek Time:"<<newseektime<<endl;
cout <<"------------------------------------"<<endl;

sc->masterclock->settime(newseektime - 0.04);
sc->masterclock->reset();


if(sc->audiostream != -1){
pthread_mutex_lock(&sc->audio_seek_status_lock);
sc->audioseek = 1;
pthread_mutex_unlock(&sc->audio_seek_status_lock);
}

if(sc->videostream != -1){
pthread_mutex_lock(&sc->video_seek_status_lock);
sc->videoseek = 1;
pthread_mutex_unlock(&sc->video_seek_status_lock);
}
///////////////////////////////////////////////////////////////////
/// Buffer some more packets
timediff = av_gettime();

timediff = av_gettime() - timediff;
cout <<"Time Taken for adding extra packets to buffer:"<<timediff<<endl;
///////////////////////////////////////////////////////////////////
/*
if(sc->networkstream){
pthread_mutex_lock(&sc->demuxlock);
sc->demux_block = 0;
pthread_mutex_unlock(&sc->demuxlock);
pthread_cond_broadcast(&sc->demuxcond);
}
*/

pthread_mutex_lock(&sc->demuxpauselock);
sc->demuxpausetoggle = 0;
pthread_mutex_unlock(&sc->demuxpauselock);
pthread_cond_broadcast(&sc->demuxpausecond); 



pthread_mutex_lock(&sc->pauselock);
sc->pausetoggle = 0;
pthread_mutex_unlock(&sc->pauselock);
pthread_cond_broadcast(&sc->pausecond);




//}

///////////////////////////////////////////////

cout <<"---------------------------------------------------------"<<endl;
  return 0;
}