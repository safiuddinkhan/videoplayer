#include "common.h"

media_clock::media_clock(){
itime = av_gettime();
}

void media_clock::reset(){
itime = av_gettime();
}

double media_clock::gettime(){
curtime = ((double)av_gettime() - (double)itime)/(double)AV_TIME_BASE;
return (stime + curtime);
}

void media_clock::settime(double time){
stime = time;
}