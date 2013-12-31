enum pixel_format {  
MP_NONE,	
MP_YV12,
MP_I420,
MP_UYVY,
MP_YUY2,
MP_RGBX,
MP_BGRX,
MP_RGB16,
MP_BGR16,
MP_RGB24,
MP_BGR24,
MP_RGBA,
MP_BGRA
};

AVPixelFormat get_pixelformat(pixel_format pixformat);
int get_fourcc_code(pixel_format pixformat);
pixel_format avpixfmt2pixformat(AVPixelFormat pixelformat);