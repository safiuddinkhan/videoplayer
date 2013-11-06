



#include "mediaplayer.h"
#include <iostream>
#include <SDL/SDL.h>

SDL_Surface *screen;

using namespace std;
//pthread_t renderthread;
//pthread_t audiothread;

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

//SDL_Window *win;

//SDL_Surface *screen;
//SDL_Overlay *surf;
//SDL_Rect rect; 

//SDL_Surface *videosurf;
//SDL_Color textColor = { 255, 255, 255 };
//SDL_Surface *message;
//TTF_Font *font = NULL;
int width1;
int height1;



//void * renderscreen(void *arg){
//uint8_t *data;
//int linesize;  
//double *videopts;
//video *out;


//SDL_Texture *bitmapTex;
//SDL_Renderer *ren;
 
//mediaplayer *mp = (mediaplayer *)arg;

// char *output = new char(100);
//ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
//videosurf = SDL_CreateRGBSurface(0, mp->width, mp->height, 32, 0, 0, 0, 0);
//bitmapTex = SDL_CreateTextureFromSurface(ren, videosurf);


//bitmapTex = SDL_CreateTexture(ren,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,mp->width,mp->height);
   
//int i,j;
//while(true){
//out = mp->get_playback_videoframe();
//cout <<out->linesize[0]<<endl;
//cout <<mp->get_pixelformat()<<endl;
//sleep(1);
//out1 = cc->convert(out);
//sprintf(output,"Video PTS: %f",out->pts);
//message = TTF_RenderText_Solid( font, output, textColor );


//SDL_LockYUVOverlay(surf);
//out->data[0] = surf->pixels[0];
//out->data[1] = surf->pixels[2];
//out->data[2] = surf->pixels[1];
//out->linesize[0] = surf->pitches[0];
//out->linesize[1] = surf->pitches[2];
//out->linesize[2] = surf->pitches[1];
//SDL_UnlockYUVOverlay(surf);
//SDL_DisplayYUVOverlay(surf, &rect);

//cout <<"Video PTS - "<<out->pts<<" - "<<mp->getstatus()<<endl;

//SDL_LockSurface(videosurf); 
//videosurf->pixels = out->data[0]; 
//videosurf->pitch = out->linesize[0];
//SDL_UnlockSurface(videosurf); 

//SDL_UpdateTexture(bitmapTex,NULL,out->data[0],out->linesize[0]);

//SDL_LockTexture(bitmapTex,NULL,(void **)&data,&linesize);
//   memcpy(data,out->data[0], out->linesize[0]*mp->height);
//   memcpy(data+(out->linesize[0]*mp->height),out->data[1], out->linesize[1]*mp->height);
//SDL_UnlockTexture(bitmapTex);

//free(out->data);
//SDL_FreeSurface(videosurf);
//SDL_RenderClear(ren);

//SDL_RenderCopy(ren, bitmapTex, NULL, NULL);
//SDL_RenderPresent(ren);

//SDL_FreeSurface(bitmapSurface);

//SDL_BlitSurface(message, &rect, screen, &rect);
//SDL_BlitSurface(videosurf, NULL, screen, &rect);

 //apply_surface( 0, 150, message, screen );

//SDL_Flip(screen);
//SDL_FreeSurface(message);
//}

//}

//void * playaudio(void *arg){

//while(true){
//out = mp->get_playback_audioframe();
//if(mp->getstatus() == MP_STOP){}else{

//}
//cout<<"Audio PTS - "<<out->pts<<endl;
//av_freep(&out.data);
//}

//pthread_exit(NULL); 
//} 



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

mediaplayer *mp = new mediaplayer(argv[1]);
mp->set_pixelformat((char *)"YV12");

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

//cout <<"------------- Meta Data -------------"<<endl;
//mp->get_metadata(NULL);
//cout <<"------------- Meta Data -------------"<<endl;

cout <<"Video Native Pixel Format:"<<mp->get_pixelformat()<<endl;
int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | SDL_HWACCEL;
SDL_WM_SetCaption( "Experiemental Player", NULL );
screen = SDL_SetVideoMode(mp->width, mp->height, 32, options);
mp->sc->screen = screen;




//int height = mp->height;
//int width = mp->width;

//int width1 =  mp->width;
//int height1 = mp->height;
//if(mp->width > 1366 || mp->height > 768){
//  width1 = mp->width / 2;
//  height1 = mp->height / 2;
//}

//win = SDL_CreateWindow("VIDEOSINK", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mp->width, mp->height, 0);

//font = TTF_OpenFont( "/home/safi/times.ttf", 20 );
//if(font == NULL){
//  cout <<"font not found...."<<endl;
//}





//cout <<get_fourcc_code(mp->get_pixelformat())<<endl;
//cout <<get_avpixelformat((char *)"BGRA")<<endl;

SDL_Event event;

void *exit;
//pthread_create(&renderthread,NULL,renderscreen,mp); 
//pthread_create(&audiothread,NULL,playaudio,mp);


//pthread_create(&test_thread,NULL,test_status,mp); 
//mp->play();

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
