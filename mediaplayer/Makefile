objects = vaapi.o internels.o mediaplayer.o clock.o audio.o video.o demux.o seeking.o buffering.o player2.o
CFLAGS = -lao -lavutil -lavformat -lavcodec -lavdevice -lswresample -lswscale -pthread `pkg-config --cflags --libs taglib` -lX11 -lXext -lXv -lva -lva-x11 -I/usr/include/va
INCLUDE = -I./include

player2 : $(objects)
	g++ -o player2 $(objects) $(CFLAGS)  

vaapi.o : $(mediaplayer_path)vaapi.cpp 
	g++ -c $(mediaplayer_path)vaapi.cpp $(INCLUDE)

internels.o : $(mediaplayer_path)internels.cpp 
	g++ -c $(mediaplayer_path)internels.cpp $(INCLUDE)

mediaplayer.o : $(mediaplayer_path)mediaplayer.cpp
	g++ -c $(mediaplayer_path)mediaplayer.cpp $(INCLUDE)
 
clock.o : $(mediaplayer_path)clock.cpp 
	g++ -c $(mediaplayer_path)clock.cpp $(INCLUDE)
 
audio.o : $(mediaplayer_path)audio.cpp
	g++ -c $(mediaplayer_path)audio.cpp $(INCLUDE)
 
video.o : $(mediaplayer_path)video.cpp
	g++ -c $(mediaplayer_path)video.cpp $(INCLUDE)
 
demux.o : $(mediaplayer_path)demux.cpp
	g++ -c $(mediaplayer_path)demux.cpp $(INCLUDE)
 
seeking.o : $(mediaplayer_path)seeking.cpp
	g++ -c $(mediaplayer_path)seeking.cpp $(INCLUDE)
 
buffering.o : $(mediaplayer_path)buffering.cpp
	g++ -c $(mediaplayer_path)buffering.cpp $(INCLUDE)

player2.o : $(mediaplayer_path)player2.cpp
	g++ -c $(mediaplayer_path)player2.cpp $(INCLUDE)

clean :
	rm -rf $(objects) player2


