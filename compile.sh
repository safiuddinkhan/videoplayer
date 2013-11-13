#!/bin/sh
clear
#g++ player.cpp -o player -lSDL -lSDL_image -lasound -lavutil -lavformat -lavcodec -lz -lm -lswresample -lswscale -pthread
#g++ player.cpp -o player -lSDL -lSDL_image -lao -ldl -lavutil -lavformat -lavcodec -lz -lm -lswresample -lswscale -pthread

g++ -c mediaplayer.cpp -o mediaplayer.o
g++ -c player.cpp -o player.o
g++ -c fourcc.cpp -o fourcc.o
# g++ -c colorspaceconverter.cpp -o colorspaceconverter.o
g++ -c clock.cpp -o clock.o
g++ -c audio.cpp -o audio.o
g++ -c video.cpp -o video.o
g++ -c demux.cpp -o demux.o
g++ -o player player.o mediaplayer.o fourcc.o clock.o audio.o video.o demux.o -lSDL -lao -lavutil -lavformat -lavcodec -lavdevice -lswresample -lswscale -pthread `pkg-config --cflags --libs taglib`  #-lva -lva-x11 -I/usr/include/va  
#gcc app_test.c -o app_test -lxdo

g++ pngviewer.cpp -o pngviewer  -lSDL -lavutil -lavformat -lavcodec -lswscale