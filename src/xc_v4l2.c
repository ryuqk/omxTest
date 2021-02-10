/************************************************************************************/
/*      @brief     V4L2 functions                                                   */
/*      @file      xc_v4l2.c                                                        */
/*                                                                                  */
/*      @author    xc                                                               */
/*      @copyright xc                                                               */
/*                                                                                  */
/************************************************************************************/

 /* **************************************************************************** */
 /* include                                                                      */
 /* **************************************************************************** */

#include <unistd.h>			/* close */
#include <sys/types.h>		/* open */
#include <sys/stat.h>		/* open */
#include <fcntl.h>			/* open */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include "xc_debug.h"
#include "xc_v4l2.h"


/********************************************************/
/*                                                      */
/********************************************************/

typedef struct{
    void *start;
    int length;
}BUFTYPE;

BUFTYPE *usr_buf;
static unsigned int n_buffer = 0;
static int is_open = -1;
static int v4l2_err = 0;

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
 static int xioctl(int fd, int request, void *arg)
{
	for (; ; ) {
		int ret = ioctl(fd, request, arg);
		if (ret < 0) {
			if (errno == EINTR){
				continue;
			}
			return -errno;
		}
		break;
	}

	return 0;
}
/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
static int init_mmap(int fd){
    struct v4l2_requestbuffers reqbufs;

    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count = 1;
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbufs.memory = V4L2_MEMORY_MMAP;

    if( xioctl(fd,VIDIOC_REQBUFS,&reqbufs) < 0)    {
        DBG_PRNTF_ERR("V4L2:Error");
        perror("Fail to ioctl 'VIDIOC_REQBUFS'");
        v4l2_err = __LINE__;
        return -1;
    }

    n_buffer = reqbufs.count;
    DBG_PRNTF_V4L2_INF("n_buffer = %d\n", n_buffer);
    usr_buf = (BUFTYPE *)calloc(reqbufs.count, sizeof(BUFTYPE));
    if(usr_buf == NULL){
        DBG_PRNTF_ERR("Out of memory\n");
        v4l2_err = __LINE__;
        return -1;
    }

    DBG_PRNTF_V4L2_INF("reqbufs.count = %d\n", reqbufs.count);
    /*map kernel cache to user process*/
    for(n_buffer = 0; n_buffer < reqbufs.count; ++n_buffer){
        //stand for a frame
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffer;

        /*check the information of the kernel cache requested*/
        if( xioctl(fd,VIDIOC_QUERYBUF,&buf) < 0){
	        DBG_PRNTF_ERR("V4L2:Error");
            perror("Fail to ioctl : VIDIOC_QUERYBUF");
	        v4l2_err = __LINE__;
	        return -1;
        }

        usr_buf[n_buffer].length = buf.length;
        usr_buf[n_buffer].start = (char *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE,MAP_SHARED, fd, buf.m.offset);

        if(MAP_FAILED == usr_buf[n_buffer].start){
	        DBG_PRNTF_ERR("V4L2:Error");
            perror("Fail to mmap");
	        v4l2_err = __LINE__;
	        return -1;
        }
    }
	return 0;
}

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_open_camera(char *device)
{
    int fd = -1;
    struct v4l2_input inp;
	if(is_open == -1){
		/* マルチデバイスに対応する場合は処理を変更する事 */
	    fd = open(device, O_RDWR | O_NONBLOCK,0);
	    if(fd < 0){
	        DBG_PRNTF_ERR( "%s open err \n", device);
	        v4l2_err = __LINE__;
	        return -1;
	    };
	
	    inp.index = 0;
	    if ( xioctl (fd, VIDIOC_S_INPUT, &inp) < 0){
	        DBG_PRNTF_ERR( "VIDIOC_S_INPUT \n");
	        v4l2_err = __LINE__;
	        return -1;
	    }
	    is_open = 1;
	}
    return fd;
}

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_init_camera(int fd, int stream_type, int height, int width, int fps)
{
    struct v4l2_capability  cap;    /* decive fuction, such as video input */
    struct v4l2_format      tv_fmt; /* frame format */
    struct v4l2_fmtdesc     fmtdesc;    /* detail control value */
	struct v4l2_streamparm   param;

	if(is_open == -1){
        DBG_PRNTF_ERR( "already open\n");
        v4l2_err = __LINE__;
        return -1;
	}


	if(fd == -1){
        DBG_PRNTF_ERR( "bad fd\n");
        v4l2_err = __LINE__;
        return -1;
	}

    /*show all the support format*/
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.index = 0 ;                 /* the number to check */
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;

    /* check video decive driver capability */
    if(xioctl(fd, VIDIOC_QUERYCAP, &cap)  < 0){
        DBG_PRNTF_ERR( "fail to ioctl VIDEO_QUERYCAP \n");
        v4l2_err = __LINE__;
        return -1;
    }

    /*judge wherher or not to be a video-get device*/
    if(!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE)){
        DBG_PRNTF_ERR( "The Current device is not a video capture device \n");
        v4l2_err = __LINE__;
        return -1;
    }

    /*judge whether or not to supply the form of video stream*/
    if(!(cap.capabilities & V4L2_CAP_STREAMING)){
        DBG_PRNTF_V4L2_INF("The Current device does not support streaming i/o\n");
        v4l2_err = __LINE__;
        return -1;
    }

    DBG_PRNTF_V4L2_INF("format %d x %d fps %d \n",height, width, fps);

    DBG_PRNTF_V4L2_INF("\ncamera driver name is : %s\n",cap.driver);
    DBG_PRNTF_V4L2_INF("camera device name is : %s\n",cap.card);
    DBG_PRNTF_V4L2_INF("camera bus information: %s\n",cap.bus_info);


    /*display the format device support*/
    DBG_PRNTF_V4L2_INF("\n");
    while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)>0){
        DBG_PRNTF_V4L2_INF("support device %d.%s\n",fmtdesc.index+1,fmtdesc.description);
        fmtdesc.index++;
    }
    DBG_PRNTF_V4L2_INF("\n");

    /*set the form of camera capture data*/
    tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;      /*v4l2_buf_typea,camera must use V4L2_BUF_TYPE_VIDEO_CAPTURE*/
    tv_fmt.fmt.pix.width = width;
    tv_fmt.fmt.pix.height = height;
	switch(stream_type){
	case V4L2_STREAM_TYPE_JPEG:
		tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
		break;
	case V4L2_STREAM_TYPE_H264:
		tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
		break;
	case V4L2_STREAM_TYPE_MJPEG:
		tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
		break;
	default:
		tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
		break;
	}
    if (xioctl(fd, VIDIOC_S_FMT, &tv_fmt)< 0){
        DBG_PRNTF_ERR("VIDIOC_S_FMT set err\n");
        close(fd);
        v4l2_err = __LINE__;
        return -1;
    }
#if 1
	memset(&(param), 0, sizeof(param));
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	param.parm.capture.timeperframe.numerator = 1;
	param.parm.capture.timeperframe.denominator = fps;

	if ( xioctl(fd, VIDIOC_S_PARM, &param) < 0){
        DBG_PRNTF_V4L2_INF("VIDIOC_S_PARM set err\n");
     //   exit(-1);
     //   close(fd);
	}
#endif


    init_mmap(fd);
	is_open =1;
	return 0;
}


/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_start_capture(int fd)
{
	if(is_open == -1){
        DBG_PRNTF_ERR( "Not open\n");
        v4l2_err = __LINE__;
        return -1;
	}

    unsigned int i;
    enum v4l2_buf_type type;
    //DBG_PRNTF_V4L2("fd=%d\n",fd);
    DBG_PRNTF_V4L2_INF("n_buffer = %d\n", n_buffer);
    /*place the kernel cache to a queue*/
    for(i = 0; i < n_buffer; i++){
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if( xioctl(fd, VIDIOC_QBUF, &buf) < 0){
            perror("2 Fail to ioctl 'VIDIOC_QBUF'");
	        v4l2_err = __LINE__;
			return -1;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if( xioctl(fd, VIDIOC_STREAMON, &type) < 0){
        DBG_PRNTF_ERR("i=%d.\n", i);
        perror("VIDIOC_STREAMON");
        close(fd);
        v4l2_err = __LINE__;
		return -1;
    }

    return 0;
}

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_read_frame(int fd, unsigned char **outbuf, int *len, int timeout)
{
    struct v4l2_buffer buf;
	fd_set fds;
    struct timeval *tvp = NULL;
    struct timeval tv;
    int ret;

	if(is_open == -1){
        DBG_PRNTF_ERR( "Not open\n");
        v4l2_err = __LINE__;
        return -1;
	}

    FD_ZERO(&fds);
    FD_SET(fd,&fds);

    /*Timeout*/
    if(timeout == 0){
    	tvp = NULL;
    }else{
	    tv.tv_sec = 0;
	    tv.tv_usec = timeout;
	    tvp = &tv;
    }

    ret = select(fd + 1, &fds, NULL, NULL,tvp);
    if(0 > ret){
        if(EINTR == errno){
			DBG_PRNTF_V4L2_INF("select received SIGINT \n");
	        v4l2_err = __LINE__;
			return -1;
		}
    }else if(0 == ret){
        DBG_PRNTF_V4L2_INF("select Timeout\n");
        v4l2_err = __LINE__;
		return -1;
    }
	if(FD_ISSET(fd, &fds)){
	    memset(&buf, 0, sizeof(buf));
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory = V4L2_MEMORY_MMAP;

	    if( xioctl(fd, VIDIOC_DQBUF,&buf) < 0){
	        DBG_PRNTF_ERR("V4L2:Error");
	        perror("Fail to ioctl 'VIDIOC_DQBUF'");
	        v4l2_err = __LINE__;
			return -1;
	    }

		*len = buf.bytesused;
		*outbuf = (unsigned char *)usr_buf[buf.index].start;

	    if( xioctl(fd, VIDIOC_QBUF,&buf) < 0){
	        DBG_PRNTF_ERR("V4L2:Error");
	        perror("1 Fail to ioctl 'VIDIOC_QBUF'");
	        v4l2_err = __LINE__;
			return -1;
	    }
	}else{
	//	DBG_PRNTF_V4L2_INF("TIMEOUT \n");
	    return 1;
	}
    return 0;
}

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_stop_capture(int fd)
{
	if(is_open == -1){
        DBG_PRNTF_ERR( "Not open\n");
        v4l2_err = __LINE__;
        return -1;
	}

    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if( ioctl(fd,VIDIOC_STREAMOFF,&type) < 0){
        DBG_PRNTF_ERR("V4L2:Error");
        perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
        v4l2_err = __LINE__;
		return -1;
    }
    return 0;
}

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_close_camera(int fd)
{
	if(is_open == -1){
        DBG_PRNTF_ERR( "Already Close\n");
        return 0;
	}

    unsigned int i;
    for(i = 0;i < n_buffer; i++){
        if(-1 == munmap(usr_buf[i].start,usr_buf[i].length)){
	        DBG_PRNTF_ERR("V4L2:Error");
	        perror("Fail to munmap");
	        v4l2_err = __LINE__;
			return -1;
        }
    }
    free(usr_buf);
    if(-1 == close(fd)){
        DBG_PRNTF_ERR("V4L2:Error");
        perror("Fail to close fd");
        v4l2_err = __LINE__;
		return -1;
    }
    is_open = -1;
    return 0;
}

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int v4l2_lastErr(void)
{
	int ret = v4l2_err;
	v4l2_err = 0;
	return ret;
}

