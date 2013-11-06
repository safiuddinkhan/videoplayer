struct video{
uint8_t **data;
int *linesize;
double pts;
};

struct audio{
uint8_t *data;
int size;
double pts;
};
