#include "mediaplayer.h"

int mediaplayer::findandopencodec(AVCodecContext *pCodecCtx , int stream_index){


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



stream_type mediaplayer::stream_detector(char *url){
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
  return stream_v4l;
}else if(strcmp(protocol,"file") == 0){
  return stream_localfile;
}else{
  return stream_none;
}

}


int mediaplayer::loadfile(char *url,stream_context *streamcontext){
stream_context *sc = streamcontext;
AVInputFormat *format = NULL;



/////////// Detecting URL
stream_type st = stream_detector(url);
sc->streamtype = st;  

if(st == stream_network){  
sc->networkstream = 1; 
}else{
sc->networkstream = 0;
}

if(st == stream_v4l){
sc->v4l_device = 1;
}else{
sc->v4l_device = 0;
}


/////////////////////
if(sc->v4l_device){
char *p;
char *path;
p = strchr(url,':');
if(p == NULL)
return stream_none;
int len = strlen(url);
int loc = p - url;

if(url[loc + 1] == '/' && url[loc+2] == '/' && url[loc+3] == '/'){

path = new char[len - loc];
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
}


}

/////////////////////


//format = av_probe_input_format( &pd, 5);
//cout <<"Format detected - "<<format->name<<endl;

//if(format == NULL){
//  cout <<"Could not recognize file..."<<endl;
//}else{
//  cout <<"File Format:"<<format->name<<" - "<<format->long_name<<endl;
//}

AVDictionary *options = NULL;
//av_dict_set(&options, "video_size", "320x240", 0);
//av_dict_set(&options, "pixel_format", "rgb24", 0);


//format = NULL;//av_find_input_format("mp3");
if(avformat_open_input(&sc->pFormatCtx, url, format, &options)!=0){
 cout <<"Error Opening File..."<<endl; 
   //sc->status = MP_ERROR;  
  put_status(MP_ERROR,sc);
  return -1; 
}


//////////////////////////////////////////////////////////////////////////


if(st == stream_none)
sc->streamtype = stream_localfile;

AVDictionaryEntry *tag = NULL;

int key_len = 0;
int val_len = 0;


while ((tag = av_dict_get(sc->pFormatCtx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))){
/*
sc->mdata = new meta_data();

if(sc->mdata_curr == NULL){
sc->mdata_init = sc->mdata;
}else{
sc->mdata_curr->next = sc->mdata;
}


key_len = strlen(tag->key)+1;
val_len = strlen(tag->value)+1;
sc->mdata->key = new char[key_len];
sc->mdata->value = new char[val_len];
memcpy(sc->mdata->key,tag->key,key_len);
memcpy(sc->mdata->value,tag->value,val_len);


sc->mdata->next = NULL;
sc->mdata_curr = sc->mdata;
*/

//printf("%s=%s\n", sc->mdata->key, sc->mdata->value);

cout <<tag->key<<" - "<<tag->value<<endl;
}



//int count = av_dict_count(options);
//cout <<"no of meta keys:"<<count<<endl;

if(sc->networkstream){
  sc->pFormatCtx->probesize = 5*1024*1024;
  sc->pFormatCtx->max_analyze_duration = 40*60*AV_TIME_BASE;
}

  //sc->pFormatCtx->probesize = 50*1024*1024;
  //sc->pFormatCtx->max_analyze_duration = 10*60*AV_TIME_BASE;

//if(format == NULL)
//  cout <<"Nothing Found..."<<endl;
//cout <<format->name<<endl;


if(avformat_find_stream_info(sc->pFormatCtx,NULL)<0){
 cout <<"Error Returning File Info..."<<endl;
  //sc->status = MP_ERROR;  
put_status(MP_ERROR,sc);
return -1; 
}


format = sc->pFormatCtx->iformat;
cout <<" - "<<format->name<<" - "<<endl;
cout <<"Duration:"<<sc->pFormatCtx->duration<<endl;
cout <<"Bitrate:"<<sc->pFormatCtx->bit_rate<<endl;
cout <<"Video Codec ID:"<<sc->pFormatCtx->video_codec_id<<endl;
sc->totalduration = sc->pFormatCtx->duration / AV_TIME_BASE;




///////////////////////////////////////////////////////////////////

int i;
sc->videostream =-1;
sc->audiostream =-1;

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
//// work around to display png images 
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
findandopencodec(sc->videoctx,i);
}

sc->pFormatCtx->streams[i]->discard = AVDISCARD_DEFAULT;


}

vs = vs + 1;
break;
case AVMEDIA_TYPE_AUDIO:
if(as == 0){
sc->audiostream=i;
sc->audioctx = sc->pFormatCtx->streams[i]->codec;
findandopencodec(sc->audioctx,i);
sc->pFormatCtx->streams[i]->discard = AVDISCARD_DEFAULT;
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

height = sc->pFormatCtx->streams[sc->videostream]->codec->height;
width = sc->pFormatCtx->streams[sc->videostream]->codec->width;
sc->pixelformat = sc->pFormatCtx->streams[sc->videostream]->codec->pix_fmt;
}

if(sc->audiostream<0){
 cout <<"No Audio Stream Found..."<<endl; 
 }else{
 cout <<"Audio Stream Found"<<endl; 

sc->audiobasetime = av_q2d(sc->pFormatCtx->streams[sc->audiostream]->time_base);
cout <<"audio time base:"<<av_q2d(sc->pFormatCtx->streams[sc->audiostream]->time_base)<<endl;

channels = sc->audioctx->channels;
samplerate = sc->audioctx->sample_rate;
//sc->sampleformat = sc->audioctx->sample_fmt;

}




//////////////////////////////////////////////////////////////////////////
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
 enum AVCodecID id = AV_CODEC_ID_NONE;
 while (mime->id != AV_CODEC_ID_NONE) {
  if (!av_strncasecmp(mime->str, mime_attached, sizeof(mime_attached))) {
 // id = mime->id;
codec = avcodec_find_decoder(mime->id);
cout <<"mime-type:"<<mime->str<<endl;
  break;
  }
  mime++;
  }


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
  cout <<"Error Parsing Attached Image"<<endl;
sc->videoctx->width = 0;
sc->videoctx->height = 0;
height = 0;
width = 0;
}else{
cout <<frame->height<<" - "<<frame->width<<endl;
sc->videoctx->width = frame->width;
sc->videoctx->height = frame->height;
height = frame->height;
width = frame->width;
}

//break;
          //  }
} else{
cout <<"No Attached Image Found..."<<endl;          
}

} else {
cout << "Incorrect URL..."<<endl;      
}

}
//////////////////////////////////////////////////////////////////////////


///////////////////////// Video Acceleration /////////////////////////
/*
VAEntrypoint entrypoints[5];
 int num_entrypoints,vld_entrypoint;
    VAConfigAttrib attrib;
    VAConfigID config_id;
    VASurfaceID surface_id;
    VAContextID context_id;
    VABufferID pic_param_buf,iqmatrix_buf,slice_param_buf,slice_data_buf;
    int major_ver, minor_ver;
    VADisplay va_dpy;
    VAStatus va_status;
    int putsurface=0;


AVHWAccel *hwainfo = new AVHWAccel();
hwainfo = sc->videoctx->hwaccel;
//cout <<hwainfo->type<<endl;

AVHWAccel* hwaccel = NULL; 
while(hwaccel = av_hwaccel_next(hwaccel)){
cout <<hwaccel->name<<" - "<<hwaccel->pix_fmt<<" - "<<AV_PIX_FMT_VAAPI_VLD<<endl;
if(sc->videoctx->codec_id == hwaccel->id)
  break;
}
sc->videoctx->hwaccel_context = hwaccel;

*/
///////////////////////// Video Acceleration /////////////////////////



sc->start_time = (double)(sc->pFormatCtx->start_time / AV_TIME_BASE);

// Check if stream/file is seekable 
if(sc->pFormatCtx->pb != NULL){
if(sc->pFormatCtx->pb->seekable == 0){  
sc->is_seekable = 0;
}else{
int ret = av_seek_frame(sc->pFormatCtx, -1, sc->pFormatCtx->start_time , AVSEEK_FLAG_BACKWARD);
if(ret < 0)
sc->is_seekable = 0;
else  
sc->is_seekable = 1;
}
}
cout <<"Start Time:"<<sc->start_time<<endl;
//cout <<"Video Codec:"<<sc->videoctx->codec_name<<endl;
cout <<"File Size:"<<avio_size (sc->pFormatCtx->pb)<<endl;


//cout <<"GOP Size:"<<sc->videoctx->gop_size<<endl;
  //  sc->videoctx->skip_top   = lavc_param_skip_top;
  //  sc->videoctx->skip_bottom= lavc_param_skip_bottom;

return 0;
}


void init_all(){
 
}

video *mediaplayer::next_videoframe(){


}

audio *mediaplayer::next_audioframe(){


}

void get_metadata(char *key,char *value,int flag){
/*
meta_data *md;
md = sc->mdata_init;
if(flag == 1){ 
while(md->next != NULL){
cout <<md->key<<" = "<<md->value<<endl;
md = md->next;
}
}
*/
//return NULL;
}

mediaplayer::mediaplayer(char *file){

//cout <<avdevice_configuration ()<<endl;

avcodec_register_all();
avdevice_register_all();
avformat_network_init(); 
av_register_all(); 




sc = new stream_context();
///////////////////////////////////////////////
sc->pFormatCtx = avformat_alloc_context();
sc->attachedimage = 0;
//sc->audioctx = avcodec_alloc_context3(NULL);
//sc->videoctx = avcodec_alloc_context3(NULL);

sc->masterclock = new media_clock(); 
sc->aout = new audio();
sc->vout = new video();
//sc->vout1 = new video();

//is_network_stream = 0;
sc->networkstream = 0;
sc->v4l_device = 0;
sc->streamtype = stream_none;

sc->end_audiothread = 1;
sc->end_videothread = 1;


sc->endthread = 1;

//sc->vidframe = avcodec_alloc_frame();

//sc->status = MP_STOP;   
put_status(MP_STOP,sc);
sc->stop = 0;
pthread_cond_init (&sc->demuxcond, NULL);
pthread_cond_init (&sc->pausecond, NULL);
pthread_cond_init (&sc->audio_waitcond, NULL);
pthread_cond_init (&sc->video_waitcond, NULL);
pthread_cond_init (&sc->demux_waitcond, NULL);

pthread_cond_init (&sc->decodecond, NULL);
pthread_cond_init (&sc->decodecond1, NULL);
pthread_cond_init (&sc->videoframeupdate, NULL);
pthread_cond_init (&sc->audioframeupdate, NULL);







int ret = loadfile(file,sc);

streamtype = sc->streamtype;

if(ret < 0){
  //sc->status = MP_ERROR;
  put_status(MP_ERROR,sc);
  streamtype = stream_none;
}



//sc->vidframe = avcodec_alloc_frame(); 
//av_read_play(sc->pFormatCtx); 

}

void mediaplayer::set_pixelformat(char *fourcc_code){
//sc->cc = new colorspace_converter(height,width,get_pixelformat(),fourcc_code);

sc->pixel_format = fourcc_code;

}


int mediaplayer::play(){


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
sc->totalduration = 0;
sc->endthread = 0;
sc->pausetoggle = 0;

sc->audiopause = 0;
sc->videopause = 0;
sc->demuxpause = 0;
sc->demux_block = 0;

sc->audiopts = 0;
sc->videopts = 0;
sc->seek = 0;
//sc->audioseek = 0;
sc->seekdelay = 0;
sc->seektime = 0;
sc->seekback = 0;
sc->videopts = 0;
sc->audiopts = 0;
sc->videoseek = 0;
sc->audioseek = 0;
sc->new_audio_seektime = 0;
sc->new_video_seektime = 0;
sc->video_flag = 0;
sc->audio_flag = 0;

sc->end_audiothread = 0;
sc->end_videothread = 0;

//sc->masterclock->settime(sc->start_time); 
//sc->masterclock->reset();
//if(sc->status == MP_STOP)
//sc->stop = 1;


sc->stop = 0;
//sc->status = MP_PLAYING;  
put_status(MP_PLAYING,sc);


if(sc->is_seekable){
// Rewind Video to Start
int ret = av_seek_frame(sc->pFormatCtx, -1, sc->pFormatCtx->start_time , AVSEEK_FLAG_BACKWARD);
if(ret < 0){
cout <<"Seeking not supported for this video..."<<endl;
}
}

  pthread_create(&sc->demuxerthread,NULL,demuxer,sc);

if(sc->videostream != -1 || sc->attachedimage == 1)
  pthread_create(&sc->videothread,NULL,videoplayback,sc);  
 
if(sc->audiostream != -1)
  pthread_create(&sc->audiothread,NULL,audioplayback,sc); 



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

if(sc->videostream != -1)
pthread_join(sc->videothread,&exit);  

//sc->status = MP_STOP;
put_status(MP_STOP,sc);
cout <<"Media Stopped"<<endl;
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
if(sc->seek == 0){// && sc->videoseek == 0 && sc->audioseek == 0){ 
sc->seektime = timestamp;
sc->seek = 1;
seek_stream(sc->seektime,sc);
//////////////////////////////////////////////
int j = 0;
while(true){


pthread_mutex_lock(&sc->audio_seek_status_lock);
if(sc->audioseek == 1)
j = j + 1;
pthread_mutex_unlock(&sc->audio_seek_status_lock);

pthread_mutex_lock(&sc->video_seek_status_lock);
if(sc->videoseek == 1)
j = j + 1;
pthread_mutex_unlock(&sc->video_seek_status_lock);

if(sc->videostream != -1 && sc->audiostream != -1){
if(j > 1)
break;
}else{
if(j > 0)
break;  
}

cout <<"Wait for all threads to start again after seeking..."<<endl;

}
//////////////////////////////////////////////

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

void mediaplayer::put_status(mediaplayer_status status,stream_context *sc){
pthread_mutex_lock(&sc->status_lock);
sc->status = status;
pthread_mutex_unlock(&sc->status_lock);
}


//char * mediaplayer::get_pixelformat(){
// return get_fourcc_code(sc->pixelformat);
//}

mediaplayer::~mediaplayer(){
delete(sc->masterclock);

delete(sc->aout);
delete(sc->vout);
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







