
//test_gprof.c


#include "mediaplayer.h"
#include <iostream>
#include <iomanip>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include <SDL/SDL.h>
#include <pthread.h>
#include <ao/ao.h>


int done = 0;
int height;
int width;
int hw_accel = 0;
Window window;
mediaplayer *mp;
int toggle_fullscreen = 0;
struct ctx{
XvImage *image;
Display *display;
XvPortID port;
Window window;
GC gc;
unsigned int format;
int width;
int height;
int new_width;
int new_height;
pthread_mutex_t display_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t image_lock = PTHREAD_MUTEX_INITIALIZER;
Colormap colourmap;
int *linesize;
ao_device *device;
ao_sample_format *audio_format;
int channels;
double audiopts;
double videopts;
  GC pen;
  XGCValues values;
  XFontStruct *font;
};



#define GUID_YUV12_PLANAR 0x32315659
#define GUID_UYVY_PLANAR 0x59565955



using namespace std;


pthread_t test_thread;
pthread_mutex_t threadcontrollock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t threadcontrol;


void *test_status(void * arg){
  ctx *c = (ctx *)arg;
while(true){
//pthread_mutex_lock(&c->display_lock);
//pthread_mutex_unlock(&c->display_lock);



////////////////////////////////////////////////////// debug display
char text[100];
sprintf(text,"Video PTS: %f - Audio PTS: %f - Master Clock: %f",c->videopts,c->audiopts,mp->getpos());
XSetForeground(c->display, c->gc, WhitePixel(c->display, DefaultScreen(c->display)));
int tw = XTextWidth(c->font, text, strlen(text));
XFillRectangle( c->display, c->window, c->pen, 0, 0, tw+20, 15);
XDrawString(c->display, c->window, c->gc, 10, 10, text, strlen(text));
XFlush(c->display);
////////////////////////////////////////////////////// debug display

pthread_mutex_lock(&threadcontrollock);
pthread_cond_wait(&threadcontrol, &threadcontrollock);
pthread_mutex_unlock(&threadcontrollock);

}
pthread_exit(NULL); 
}

//uint8_t **video_data


void video_init(void **data,int *linesize,void * opaque){
   ctx *c = (ctx *)opaque;
//c->image->data = (char *)data[0];

//data = (void **)&c->image->data;
//c->image->data = (char *)data[0];

//data[0] = c->image->data[0];
//data[1] = c->image->data[1];
//data[2] = c->image->data[2];
//linesize = c->image->pitches;
//linesize[0] = c->image->pitches[0];
//linesize[1] = c->image->pitches[1];
//linesize[2] = c->image->pitches[2];


cout <<"pitches:"<<c->image->pitches[0]<<" - "<<c->image->pitches[1]<<" - "<<c->image->pitches[2]<<endl;
cout <<"linesize:"<<linesize[0]<<" - "<<linesize[1]<<" - "<<linesize[2]<<endl;

//c->image->offsets[1] = (data[1] - data[0]);
/*
c->image->width = c->width;
c->image->height = c->height;

c->image->pitches[0] = linesize[0];
c->image->pitches[1] = linesize[1];
c->image->pitches[2] = linesize[2];

c->image->offsets[0] = 0;
c->image->offsets[2] = linesize[0] * c->height;
c->image->offsets[1] = ((linesize[1]/2) * c->height) + (linesize[0] * c->height);

c->image->data_size = ((linesize[2]/2) * c->height) + ((linesize[1]/2) * c->height) + (linesize[0] * c->height);
*/
/*
cout <<c->image->offsets[0]<<endl;
cout <<c->image->offsets[1]<<endl;
cout <<c->image->offsets[2]<<endl;

cout <<"data size:"<<c->image->data_size<<endl;
cout <<"calculated data size:"<<(linesize[0] * c->height + linesize[1] * c->height + linesize[2] * c->height)<<endl;
cout <<"new width:"<<c->image->width<<" - new height:"<<c->image->height<<endl;
*/
done = 1;
}


void video_callback(void * hardware_context,void **data,int *linesize , double pts,void * opaque){
  ctx *c = (ctx *)opaque;


if(hardware_context == NULL){
if(linesize[0] != c->image->pitches[0] || linesize[1] != c->image->pitches[1] || linesize[2] != c->image->pitches[2]){
//  cout <<"Linesize Different..."<<endl;
int y;
for(y = 0; y < c->height;y++){
memcpy(c->image->data+(y * c->image->pitches[0]),(char *)data[0] + (y * linesize[0]),c->image->pitches[0]);
memcpy((c->image->data+c->image->offsets[1])+(y/2 * c->image->pitches[1]),(char *)data[1] + (y/2 * linesize[1]),c->image->pitches[1]);
memcpy((c->image->data+c->image->offsets[2])+(y/2 * c->image->pitches[2]),(char *)data[2] + (y/2 * linesize[2]),c->image->pitches[2]);
}

}else{
c->image->data = (char *)data[0];  
}
XvPutImage( c->display, c->port, c->window, c->gc, c->image, 0, 0, c->image->width, c->image->height, 0, 0, c->new_width, c->new_height);


//cout <<"New Image Size:"<<c->image->height<<" - "<<c->image->width<<endl;
hw_accel = 0;
}else{
hw_accel = 1;
//////////////// Video Acceleration //////////////// 

VAStatus va_status;
VAConfigID config_id;
VAContextID context_id;
VADisplay va_dpy;
//cout <<"test1"<<endl;
vaapi_context *vc = (vaapi_context*)hardware_context;
va_dpy = vc->display;
//cout <<"test2"<<endl;
VASurfaceID id = (VASurfaceID)(uintptr_t)data[3];
//cout <<"test4"<<endl;
va_status = vaSyncSurface(va_dpy, id);
//cout <<"test5"<<endl;
if(va_status == VA_STATUS_SUCCESS){
 // cout <<"Successful..."<<endl;
}

vaPutSurface(va_dpy, id, window,
                        0, 0,
                        c->width, c->height,
                        0, 0,
                        c->new_width, c->new_height,
                        NULL, 0,
                        VA_FRAME_PICTURE);
//cout <<"test6"<<endl;

//if(va_status == VA_STATUS_SUCCESS){
//  cout <<"Done putsurface Successful..."<<endl;
//  XCopyArea(sc->x11_dpy, sc->pix, sc->window, sc->gc, 0, 0,
//          sc->videoctx->height, sc->videoctx->width,
//          0, 0);
//  }

//////////////// Video Acceleration ////////////////
}

c->videopts = pts;

pthread_cond_broadcast(&threadcontrol);

}

void audio_callback(uint8_t * data,int size, double pts, void * opaque){
ctx *c = (ctx *)opaque;
c->audiopts = pts;
pthread_cond_broadcast(&threadcontrol);
ao_play(c->device, (char *)data, size*c->channels*(c->audio_format->bits/8));
}



static Atom xv_intern_atom_if_exists( Display *display, XvPortID port,
                                      char const *atom_name )
{
  XvAttribute * attributes;
  int attrib_count,i;
  Atom xv_atom = None;

  attributes = XvQueryPortAttributes( display, port, &attrib_count );
  if( attributes!=NULL )
  {
    for ( i = 0; i < attrib_count; ++i )
    {
      if ( strcmp(attributes[i].name, atom_name ) == 0 )
      {
        xv_atom = XInternAtom( display, atom_name, False );
        break; // found what we want, break out
      }
    }
    XFree( attributes );
  }

  return xv_atom;
}

Bool waitForNotify( Display *, XEvent *e, char *arg )
{
  return ( e->type == MapNotify ) && ( e->xmap.window == (Window)arg );
}

// Main Function
int main(int argc, char *argv[]){

XInitThreads();
  Display *display = XOpenDisplay( NULL );


 unsigned int ver, rel, req, ev, err;
    bool retVal =
      ( XvQueryExtension( display, &ver, &rel, &req, &ev, &err ) == Success );
    if ( !retVal )
      return 1;

unsigned int adaptors;
  XvAdaptorInfo *ai = NULL;
  
    if(XvQueryAdaptors( display, DefaultRootWindow( display ), &adaptors, &ai ) == Success ){

    }else{
      return 1;
    }


  XvPortID port = 0;
  for ( int i=0; i<adaptors; i++ ) {
    if ( ( ai[i].type & ( XvInputMask | XvImageMask ) ) ==
         ( XvInputMask | XvImageMask ) ) {
      for ( int p=ai[i].base_id; p<ai[i].base_id+ai[i].num_ports; p++ )
        if ( !XvGrabPort( display, p, CurrentTime ) ) {
          port = p;
          break;
        };
      if ( port != 0 )
        break;
    };
  };

  if ( !port )
    return 1;

 //XvGrabPort( display, 87, CurrentTime);
 //port = 87;
  cout << "Xv port is " << port << endl;


  int colourkey = 0;
/*
  Atom xvColorKey = xv_intern_atom_if_exists( display, port, "XV_COLORKEY" );
  if ( xvColorKey != None ) {
    cout << "Require drawing of colorkey" << endl;
   
    if ( XvGetPortAttribute( display, port, xvColorKey, &colourkey ) != Success )
      return 1;
    Atom xvAutoPaint = xv_intern_atom_if_exists( display, port,"XV_AUTOPAINT_COLORKEY" );
    if ( xvAutoPaint != None ) {
      cout << "Enabling autopainting" << endl;
      XvSetPortAttribute( display, port, xvAutoPaint, 1 );
      xvColorKey = None;
    };
  } else {
    cout << "No drawing of colourkey required" << endl;
  }
*/

/*
Atom sync = xv_intern_atom_if_exists( display, port,"XV_SYNC_TO_VBLANK");
if(sync != None){
cout <<"Setting Value..."<<endl;
if(XvSetPortAttribute( display, port, sync, 1 )!= Success )
return 1;
}
*/
  unsigned int formats;
  XvImageFormatValues *fo;
  fo = XvListImageFormats( display, port, (int *)&formats );
  if ( !fo )
    return 1;
  unsigned int format = 0;
  for ( int i=0; i<formats; i++ ) {
    cout << "found format " << (char *)&fo[i].id << " GUID 0x"
         << setbase( 16 ) << fo[i].id << setbase( 10 );
    if ( fo[i].id == 0x30323449 ) { /////// UYVY GUID 0x59565955 pixel format
       format = fo[i].id;
      cout << " (to be used)";
    };
    cout << endl;
  };
  if ( !format )
    return 1;

    int depth;
  {
    XWindowAttributes attribs;
    XGetWindowAttributes( display, DefaultRootWindow( display ), &attribs );
    depth = attribs.depth;
    if (depth != 15 && depth != 16 && depth != 24 && depth != 32) depth = 24;
  }
  XVisualInfo visualInfo;
  XMatchVisualInfo( display, DefaultScreen( display ), depth, TrueColor,
                    &visualInfo );
  XSetWindowAttributes xswa;
  Colormap colourMap =
    XCreateColormap( display, DefaultRootWindow( display ), visualInfo.visual,
                     AllocNone );
  xswa.colormap = colourMap;
  xswa.border_pixel = 0;

////
 // xswa.background_pixel = colourkey;
////
  xswa.event_mask =
    ExposureMask | StructureNotifyMask | KeyPressMask | PointerMotionMask;
  unsigned long mask = CWBorderPixel | CWColormap | CWEventMask;
/////
 // if ( xvColorKey != None ) mask |= CWBackPixel;
////

////////////////////////////////////////////////////////////////////////////////

cout <<"Test Player"<<endl;




init_all();

pthread_cond_init (&threadcontrol, NULL);
mp = new mediaplayer(argv[1],(char *)"YV12",display);
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



int totalheight = 768;
int totalwidth = 1366;
int x;



height = mp->height;
width = mp->width;

if(width == 0 && height == 0){
width = 500;
height = 500;
}


/*

if(height > totalheight || width > totalwidth){

width = width / 2;
height = height / 2;

}
*/


if((mp->aspect_ratio_num > mp->aspect_ratio_den) && (mp->width < mp->height)){
	width = mp->height;
	height = mp->width;
}




ctx c;


cout <<mp->width<<" - "<<mp->height<<endl;
cout <<width<<" = "<<height<<endl;

////////////////////////////////////////////////////////////////////////////////////
xswa.border_pixel = 0;
xswa.background_pixel = 0;


////////////////////////////////////////////////////////////////////////////////
window = XCreateWindow( display,
                                 RootWindow( display, visualInfo.screen ),
                                 0, 0, width, height, 0,
                                 visualInfo.depth, InputOutput,
                                 visualInfo.visual,
                                 mask,
                                 &xswa );


/*
  Window win2 = XCreateSimpleWindow(
        display,
        RootWindow(display, DefaultScreen(display)),
        0, 0, width, height,
        1, BlackPixel(display, DefaultScreen(display)), WhitePixel(display, DefaultScreen(display))
    );
XMapWindow( display, win2 );
*/

XWindowAttributes *attrib = (XWindowAttributes*)malloc(sizeof(XWindowAttributes));
XGetWindowAttributes(display,window,attrib);

XStoreName(display, window, "Experiemental Player 2");
XEvent event;
  XMapWindow( display, window );
  XIfEvent( display, &event, waitForNotify, (char *)window );

  XGCValues xgcv;
  
c.font = XLoadQueryFont(display, "fixed");
  if (!c.font) {
cout <<"Error could not load the fonts"<<endl;    
  }
  xgcv.font = c.font->fid;
  
  GC gc = XCreateGC( display, window, 0L, &xgcv );

    //if ( xvColorKey != None ) {
    XSetForeground( display, gc, colourkey );
    XFillRectangle( display, window, gc, 0, 0, width, height );
    cerr << "Filled " << width << 'x' << height << "-rectangle with colour "
         << colourkey << '.' << endl;
  //}





 

 
 c.display = display;
 c.port = port;
 c.window = window;
 c.gc = gc;
 c.colourmap = colourMap;
/////////////////////////
height = mp->height;
width = mp->width;
/////////////////////////
 c.height = height;
 c.width = width;
 c.new_height = height;
 c.new_width = width;
 c.format = format;
////////////////////////////////////////////////////////////////////////////////

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
//format.matrix = (char *)"L,R";

  c.values.foreground = BlackPixel(display, DefaultScreen(display));
  c.values.line_width = 1;
  c.values.line_style = LineSolid;
  c.pen = XCreateGC(display, window, GCForeground|GCLineWidth|GCLineStyle,&c.values);
char *pixels;

c.image = (XvImage *)XvCreateImage(display, port, format, NULL, width, height);
pixels = (char *)malloc(sizeof(char) * c.image->data_size);
c.image->data = pixels;
c.device = ao_open_live(default_driver, c.audio_format, NULL);
  if (c.device == NULL) {
  cout <<"Sound Error..."<<endl;
    }

////////////////////////////////////////////////////////////////////////////////


//char *fontname;
//char * text = (char *)"Hello...";
//  int text_width;
//  int textx, texty;


 // XGCValues values;
  //values.font = font->fid;
  //GC pen = XCreateGC(display, window, GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);
  //text_width = XTextWidth(font, text, strlen(text));


//Pixmap pix = XCreatePixmap(display, window, mp->height, mp->width, depth);
mp->window = window;
//mp->pix = pix;
//mp->gc = gc;

mp->set_callbacks(video_init,video_callback,audio_callback,&c);



//SDL_Event event1;

void *exit;

pthread_create(&test_thread,NULL,test_status,&c); 

mp->play();

 int keypress = 0;
int fc = 0;

  //XEvent event;
while(true){

//////////////////////////////////////////////////////////////////////
//cout <<"Hello..."<<endl;
   
 if ( XCheckMaskEvent( display,
                          KeyPressMask | ExposureMask | StructureNotifyMask,
                          &event ) ) {
   //XNextEvent(display, &event);   
      switch ( event.type ) {
      case ConfigureNotify:
      if(width != event.xconfigure.width ||height != event.xconfigure.height){
c.new_height = event.xconfigure.height;
c.new_width = event.xconfigure.width;
//XClearWindow(display, window);
cout <<"Size of Window changed..."<<endl;
      }
      break;
      
      case Expose:
        cerr << "Repainting" << endl;
//XClearWindow(display, window);
XGetWindowAttributes(display,window,attrib);
c.new_width = attrib->width;
c.new_height = attrib->height;
if(done == 0){
      XSetForeground( display, gc, colourkey );
      XFillRectangle( display, window, gc, 0, 0, c.new_width, c.new_height);
}
//pthread_mutex_lock(&c.display_lock);
if(hw_accel == 0){
cout <<"No Hardware Acceleration Enabled..."<<endl;
        XvPutImage( display, port, window, gc,
                    c.image, 0, 0, width, height, 0, 0, c.new_width, c.new_height );
}
//pthread_mutex_unlock(&c.display_lock);

pthread_cond_broadcast(&threadcontrol);



        break;

      case KeymapNotify:
     XRefreshKeyboardMapping(&event.xmapping);
      break;

      case KeyPress:
     // break;
     // case KeyRelease:
        cerr << "Key was pressed:"<<event.xkey.keycode<<endl;
       if ( event.xkey.keycode == 0x09 )
          return 1;//
       //   quit = true;

if ( event.xkey.keycode == 113 ){
mp->seek(mp->getpos()-15);  

 
}

if ( event.xkey.keycode == 114 ){
mp->seek(mp->getpos()+15);
}

if ( event.xkey.keycode == 65 ){
mp->pause();
}

if ( event.xkey.keycode == 39 ){
mp->stop();
}

if ( event.xkey.keycode == 33 ){
mp->play();
}

if ( event.xkey.keycode == 36 ){
  
Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
Atom fullScreen = XInternAtom(display,"_NET_WM_STATE_FULLSCREEN", False);

XEvent xev;
xev.xclient.type=ClientMessage;
xev.xclient.serial = 0;
xev.xclient.send_event=True;
xev.xclient.window=window;
xev.xclient.message_type=wmState;
xev.xclient.format=32;

if(toggle_fullscreen == 0){
xev.xclient.data.l[0] = 1;
toggle_fullscreen = 1;
cout <<"entering full screen state:"<<endl;
}else{
xev.xclient.data.l[0] = 0;
toggle_fullscreen = 0;
cout <<"leaving full screen state:"<<endl;
}
xev.xclient.data.l[1] = fullScreen;
xev.xclient.data.l[2] = 0;

XSendEvent(display, DefaultRootWindow(display), False,
SubstructureRedirectMask | SubstructureNotifyMask,
&xev);
}

        break;
      default:
    //  usleep(100000);
      sched_yield();
        break;
      };
}

      usleep(10000);
      sched_yield();


  
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


//free( image->data );
//  // Free shared memory!!!!
//cout <<"Here..."<<endl;

  XFree( c.image );
 // // XSync( display, False ); ?
  XFreeGC( display, gc );
  XvUngrabPort( display, port, CurrentTime );
  XDestroyWindow( display, window );
  XFreeColormap( display, colourMap );
  XvFreeAdaptorInfo(ai);
  XFree(fo);
  XCloseDisplay( display );
 ao_close(c.device);
 ao_shutdown();

return 0;	
}
