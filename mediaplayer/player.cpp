



#include "mediaplayer.h"
#include <iostream>
#include <SDL/SDL.h>
#include <ao/ao.h>


int done = 0;
int height;
int width;

struct ctx{
SDL_Surface *screen;
SDL_Overlay *surf;
SDL_Rect rect; 
ao_device *device;
ao_sample_format *audio_format;
int channels;
};


using namespace std;

/*
pthread_t test_thread;


void *test_status(void * arg){
mediaplayer *mp = (mediaplayer *)arg;
mediaplayer_status oldstatus ;
int pid = getpid();
FILE *f;
while(true){
if(oldstatus != mp->getstatus()){
cout <<"----------------------------"<<endl;
cout <<"current status:"<<mp->getstatus()<<endl;
cout <<"----------------------------"<<endl;

f = fopen("/dev/shm/sdlplayer_signal", "w");
//fputc((int)n,f);
fprintf(f,"%d,%d\n",(int)mp->sc->status, pid);
fclose(f);
}
oldstatus = mp->getstatus();

SDL_Delay(100);

}


pthread_exit(NULL); 
}
*/


void audio_callback(uint8_t * data,int size, double pts, void * opaque){
ctx *c = (ctx *)opaque;
ao_play(c->device, (char *)data, size*c->channels*(c->audio_format->bits/8));
}



void video_init(void **data,int *linesize,void * opaque){
   ctx *c = (ctx *)opaque;

cout<<" Done..."<<endl;
cout <<"linesize:"<<linesize[0]<<" - "<<linesize[1]<<" - "<<linesize[2]<<endl;
SDL_LockYUVOverlay(c->surf);
data[0] = c->surf->pixels[0];
data[1] = c->surf->pixels[2];
data[2] = c->surf->pixels[1];

linesize[0] = c->surf->pitches[0];
linesize[1] = c->surf->pitches[2];
linesize[2] = c->surf->pitches[1];
SDL_UnlockYUVOverlay(c->surf);
done = 1;
cout <<"linesize:"<<linesize[0]<<" - "<<linesize[1]<<" - "<<linesize[2]<<endl;
cout <<"linesize1:"<<c->surf->pitches[0]<<" - "<<c->surf->pitches[1]<<" - "<<c->surf->pitches[2]<<endl;


}


void video_callback(void * hardware_context , void **data , double pts,void * opaque){
  ctx *c = (ctx *)opaque;

SDL_DisplayYUVOverlay(c->surf, &c->rect);	
cout <<pts<<endl;
}




// Main Function
int main(int argc, char *argv[]){


cout <<"Test Player"<<endl;
 if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1)
    {
        printf("cannot initialize SDL\n");
        return EXIT_FAILURE;
    }

//     if( TTF_Init() == -1 ) { return false; }


//if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
//cout <<"Error Initializing SDL ..."<<endl;
//return 0;
//}



init_all();

mediaplayer *mp = new mediaplayer(argv[1],(char *)"YV12",NULL);

/*
switch(mp->streamtype){
case stream_network:
cout <<"url is a network stream"<<endl;
break;
case stream_v4l:
cout <<"url is a v4l device"<<endl;
break;
case stream_localfile:
cout <<"url is a local file"<<endl;
break;
case stream_none:
cout <<"no stream detected..."<<endl;
break;
}
*/

cout <<"------------- Meta Data -------------"<<endl;
char *key;
char *value;
metadata_entry *entry;
while(true){
entry = mp->get_metadata();
if(entry == NULL)
break;

cout <<entry->key<<" - "<<entry->value<<endl;
}
cout <<"------------- Meta Data -------------"<<endl;

//cout <<"Video Native Pixel Format:"<<mp->get_pixelformat()<<endl;
int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | SDL_HWACCEL;

int totalheight = 768;
int totalwidth = 1366;
int x;



height = mp->height;
width = mp->width;
if(width == 0 && height == 0){
width = 200;
height = 200;
}

if(height > totalheight || width > totalwidth){

width = width / 2;
height = height / 2;

}

if((mp->aspect_ratio_num > mp->aspect_ratio_den) && (mp->width < mp->height)){
	width = mp->height;
	height = mp->width;
}


SDL_WM_SetCaption( "Experiemental Player", NULL );
ctx c;

int default_driver;
//ao_device *device;
ao_initialize();
default_driver = ao_default_driver_id();
c.audio_format = new ao_sample_format();

c.audio_format->bits = 16;
c.channels = 2;
c.audio_format->channels = c.channels;
c.audio_format->rate = mp->samplerate;
c.audio_format->byte_format = AO_FMT_NATIVE;
c.screen = SDL_SetVideoMode(width, height, 32, options);
cout <<mp->width<<" - "<<mp->height<<endl;
cout <<width<<" = "<<height<<endl;
c.surf = SDL_CreateYUVOverlay(mp->width, mp->height, SDL_YV12_OVERLAY, c.screen);
   c.rect.x = 0;
   c.rect.y = 0;
   c.rect.w = width;
   c.rect.h = height;

c.device = ao_open_live(default_driver, c.audio_format, NULL);
  if (c.device == NULL) {
  cout <<"Sound Error..."<<endl;
    }


mp->set_callbacks(video_init,video_callback,audio_callback,&c);

cout <<"here..."<<endl;


SDL_Event event;

void *exit;

//pthread_create(&test_thread,NULL,test_status,mp); 

mp->play();


 int keypress = 0;
int fc = 0;

while(true){

//if(mp->getstatus() == MP_STOP && fc == 0){
//test_send_signal((int)mp->getstatus());
//fc = 1;
//}	

//if(mp->getstatus() == MP_PLAYING){
//test_send_signal((int)mp->getstatus());
//}	


while(SDL_PollEvent(&event)){ 
switch(event.type){
case SDL_QUIT: 
return 0;
break;

case SDL_KEYDOWN:
keypress = event.key.keysym.sym;
//cout <<"key press:"<<keypress<<endl;
if(keypress == 109){
cout <<"------------- Meta Data -------------"<<endl;
char *key;
char *value;
metadata_entry *entry;
while(true){
entry = mp->get_metadata();
if(entry == NULL)
break;

cout <<entry->key<<" - "<<entry->value<<endl;
}
cout <<"------------- Meta Data -------------"<<endl;	
}

if(keypress == 27){ 
return 0;
}

if(keypress == 275){
//mp->seek(7800);
mp->seek(mp->getpos()+15);
}

if(keypress == 276){
mp->seek(mp->getpos()-15);
}

if(keypress == 115){
mp->stop();
//delete(mp);
}

if(keypress == 112){
 // cout <<argv[1]<<endl;
 // cout <<"load file:"<<mp->loadfile(argv[1])<<endl;
//mp = new mediaplayer(argv[1]);
//mp->set_pixelformat((char *)"YV12");
//mp->sc->screen = screen;
mp->play();
}

if(keypress == 107){
mp->seek(2279);	
}

if(keypress == SDLK_SPACE){
cout <<"Video Paused..."<<endl;
mp->pause();


}  
sched_yield();
break;
}
}


SDL_Delay(100);

//cout <<mp->getpos()<<" - "<<mp->getduration()<<endl;

}
///////////////////////////////////////////////////////

/*
if(sc->endthread == 2){
cout <<"mode 2"<<endl;  
}else{
sc->endthread = 1;
pthread_join(sc->videothread,&exit);
pthread_join(sc->audiothread,&exit);
}
avcodec_close(sc->videoctx);
avcodec_close(sc->audioctx);
avformat_close_input(&sc->pFormatCtx); 
*/
//pthread_join(renderthread,&exit);
//pthread_join(audiothread,&exit);
endprog:

//SDL_FreeYUVOverlay(surf);
//SDL_FreeSurface(screen);

return 0;	
}
