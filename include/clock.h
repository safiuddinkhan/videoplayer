#include <stdint.h>

class media_clock{
private:
int64_t itime;
double curtime;
double stime;

public:  
media_clock();
void reset();
double gettime();
void settime(double time);
};  