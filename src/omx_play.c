/************************************************************************************/
/*      @brief     OMX functions                                                    */
/*      @file      omx_play.c                                                       */
/*                                                                                  */
/*      @author    xc                                                               */
/*      @copyright xc                                                               */
/*       http://maemo.org/api_refs/5.0/beta/libomxil-bellagio/group__video.html     */
/************************************************************************************/

 /* **************************************************************************** */
 /* include                                                                      */
 /* **************************************************************************** */

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "getopt.h"
#include "xc_debug.h"
#include "xc_v4l2.h"

#ifndef OMX_SKIP64BIT
#define OMX_SKIP64BIT
#endif

#include "omx_api.h"

#define VERSION "0.5"

/********************************************************/
/*                                                      */
/********************************************************/

void print_help( void ){
	
	printf("args\n"
			"\t-d: device default: /dev/video0\n"
			"\t-H: image height default:640 \n"
			"\t-W: image width default:480 \n"
			"\t-F: image fps default:30 \n"
			"\t-h: this help \n"
			);
	
}

/****************************************************************************//**
 *  @brief			
 *
 *	@param[in]		
 *	@retval			
 *		
 ****************************************************************************** */
void handler(int sig) {
  void *array[128];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 128);

  // print out all the frames to stderr
  fprintf(stderr, "\n----\n\nError: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  fflush(stdout);
  exit(1);
}
/********************************************************/
/*						enum定義						*/
/********************************************************/

/** プロセス状態定義 */

typedef enum{
	_STREAM_TYPE_UNDEF = 0,
	_STREAM_TYPE_JPEG,
	_STREAM_TYPE_MJPEG,
	_STREAM_TYPE_H264,
} n_STREAM_TYPE;



 /* **************************************************************************** */
 /* define																		 */
 /* **************************************************************************** */


#define SRV_STR_MAX	16

typedef struct {
    char stream_type;
	int image_height;
	int image_width;
	int image_fps;
    char device[SRV_STR_MAX];
} _VIDEO_INFO;


/****************************************************************************//**
 *  @brief			
 *
 *	@param[in]		
 *	@retval			
 *		
 ****************************************************************************** */
 int main(int argc, char** argv)
 {
 
	int opt;
	_VIDEO_INFO serverInfo;
	time_t timer;

	signal(SIGSEGV, handler);   // install our handler
	signal(SIGABRT, handler);   // install our handler
	signal(SIGPIPE, SIG_IGN);
    
    memset((char *)&serverInfo, 0, sizeof(serverInfo));


	timer = time(NULL);          /* 経過時間を取得 */
	printf("%s Ver %s Build %s\n",argv[0],VERSION,ctime(&timer));
	
	opterr = 0; //getopt()のエラーメッセージを無効にする。
	
	/* init */
	snprintf(serverInfo.device, SRV_STR_MAX, "/dev/video0");
	serverInfo.stream_type = _STREAM_TYPE_H264;
	serverInfo.image_height = 1080;
	serverInfo.image_width = 1920;
	serverInfo.image_fps = 30;

	while ((opt = getopt(argc, argv, "d:u:f:C:R:H:W:F:h")) != -1) {
		//コマンドライン引数のオプションがなくなるまで繰り返す
		switch (opt) {
		case 'd':	// MRL default: "/dev/video0"
			snprintf(serverInfo.device, SRV_STR_MAX, "%s",optarg);
			serverInfo.device[SRV_STR_MAX-1] = '\0';
			break;
		case 'H':	//  height default:1920
			serverInfo.image_height = strtol(optarg, NULL, 10);
			break;
		case 'W':	//  width default:1080
			serverInfo.image_width = strtol(optarg, NULL, 10);
			break;
		case 'F':	//  framerate default:30
			serverInfo.image_fps = strtol(optarg, NULL, 10);
			break;
		case 'h':
			print_help();
			exit(0);
		default:
			break;
		}
	}
	
	OMX_Open();
	OMX_Start();

	int camera_fd = v4l2_open_camera(serverInfo.device);
	v4l2_init_camera( camera_fd, serverInfo.stream_type, serverInfo.image_height, serverInfo.image_width, serverInfo.image_fps);
	v4l2_start_capture( camera_fd );
	size_t img_size = 0;
	unsigned char *img_buf = NULL;
		
	while(1){
		int ret = v4l2_read_frame(camera_fd, (unsigned char**)&img_buf, (int *)&img_size, 0);
		if( ret  == -1 ){
			//error
			DBG_PRNTF_ERR("start:%d\n",ret);
			return -1;
		}else if( ret == 1){
			continue;
		}
		
		if(img_buf == NULL || img_size == 0){
			// error
			DBG_PRNTF_ERR("err:%d\n",img_size);
			return -1;
		} else if(img_size > 0){
			OMX_Decode((char*)img_buf, img_size);
		}
	}
	v4l2_stop_capture(camera_fd);
	v4l2_close_camera(camera_fd);
	OMX_Stop();
	OMX_Close();
	

	printf("\n\n --- play End --- \n");
    return 0;
}

