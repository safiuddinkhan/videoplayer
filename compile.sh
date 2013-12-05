#!/bin/sh
clear
#g++ player.cpp -o player -lSDL -lSDL_image -lasound -lavutil -lavformat -lavcodec -lz -lm -lswresample -lswscale -pthread
#g++ player.cpp -o player -lSDL -lSDL_image -lao -ldl -lavutil -lavformat -lavcodec -lz -lm -lswresample -lswscale -pthread

g++ -pg -c mediaplayer.cpp -o mediaplayer.o
#g++ -c player.cpp -o player.o
g++ -pg -c fourcc.cpp -o fourcc.o
# g++ -c colorspaceconverter.cpp -o colorspaceconverter.o
g++ -pg -c clock.cpp -o clock.o
g++ -pg -c audio.cpp -o audio.o
g++ -pg -c video.cpp -o video.o
g++ -pg -c demux.cpp -o demux.o
#g++ -o player player.o mediaplayer.o fourcc.o clock.o audio.o video.o demux.o -lSDL -lao -lavutil -lavformat -lavcodec -lavdevice -lswresample -lswscale -pthread `pkg-config --cflags --libs taglib`  -lva -lva-x11 -I/usr/include/va  

g++ -pg -c player2.cpp -o player2.o
g++ -pg -o player2 player2.o mediaplayer.o fourcc.o clock.o audio.o video.o demux.o -lao -lavutil -lavformat -lavcodec -lavdevice -lswresample -lswscale -pthread `pkg-config --cflags --libs taglib` -lX11 -lXext -lXv -lva -lva-x11 -I/usr/include/va  

#gcc app_test.c -o app_test -lxdo

# g++ pngviewer.cpp -o pngviewer  -lSDL -lavutil -lavformat -lavcodec -lswscale