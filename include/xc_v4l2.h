/************************************************************************************/
/*      @brief     V4L2 functions                                                   */
/*      @file      xc_v4l2.h                                                        */
/*                                                                                  */
/*      @author    xc                                                               */
/*      @copyright xc                                                               */
/*                                                                                  */
/************************************************************************************/
#ifndef INCLUDE_XC_V4L2_H
#define INCLUDE_XC_V4L2_H


#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	V4L2_STREAM_TYPE_UNDEF = 0,
	V4L2_STREAM_TYPE_JPEG,
	V4L2_STREAM_TYPE_MJPEG,
	V4L2_STREAM_TYPE_H264,
} nV4L2_STREAM_TYPE;


extern int v4l2_open_camera(char *device_name);
extern int v4l2_init_camera(int fd, int stream_type, int height, int width, int fps);
extern int v4l2_start_capture(int fd);
extern int v4l2_read_frame(int fd, unsigned char **outbuf, int *len, int timeout); 
extern int v4l2_stop_capture(int fd);
extern int v4l2_close_camera(int fd);
extern int v4l2_lastErr(void);

#ifdef __cplusplus
}
#endif


#endif	/* INCLUDE_XC_V4L2_H */
