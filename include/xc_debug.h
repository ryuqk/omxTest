/************************************************************************************/
/*      @brief                                                                      */
/*      @file      xc_debug.h                                                       */
/*                                                                                  */
/*      @author    xc                                                               */
/*      @copyright xc                                                               */
/*                                                                                  */
/************************************************************************************/
#ifndef INCLUDE_XC_DBG_H
#define INCLUDE_XC_DBG_H

 /* **************************************************************************** */
 /* include                                                                      */
 /* **************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
	
	#define dbg_printf	printf
	
/* ---------------------------------------------*/
/*	ログ出力関連								*/
/* ---------------------------------------------*/

#if 0
	#define __DBG_FUNC			1
	#define __DBG_DBG			1
	#define __DBG_SERVER		1
	#define __DBG_CLIENT		1
	#define __DBG_SESSION		1
	#define __DBG_STREAM		0
	#define __DBG_RTP			1
	#define __DBG_RTCP			1
	#define __DBG_ERR			1
	#define __DBG_DUMP			1
	#define __DBG_V4L2			1
	#define __DBG_V4L2_INFO		1
	#define __DBG_PL			0
#else
	#define __DBG_FUNC			0
	#define __DBG_DBG			0
	#define __DBG_SERVER		0
	#define __DBG_CLIENT		0
	#define __DBG_SESSION		0
	#define __DBG_STREAM		0
	#define __DBG_RTP			0
	#define __DBG_RTCP			0
	#define __DBG_ERR			1
	#define __DBG_DUMP			0
	#define __DBG_V4L2			0
	#define __DBG_V4L2_INFO		0
	#define __DBG_PL			0
#endif

	
	
	#if __DBG_FUNC			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_FUNC(...)		(dbg_printf("[%04d][%20s][LOG] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_FUNC(...)
	#endif

	#if __DBG_DBG			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF(...)		(dbg_printf("[%04d][%20s][LOG] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF(...)
	#endif

	#if __DBG_SERVER			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_SRV(...)		(dbg_printf("[%04d][%20s][SRV] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_SRV(...)
	#endif

	#if __DBG_CLIENT			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_CLI(...)		(dbg_printf("[%04d][%20s][CLI] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_CLI(...)
	#endif

	#if __DBG_SESSION			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_SES(...)		(dbg_printf("[%04d][%20s][SES] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_SES(...)
	#endif

	#if __DBG_STREAM			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_STM(...)		(dbg_printf("[%04d][%20s][STM] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_STM(...)
	#endif

	#if __DBG_RTP			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_RTP(...)		(dbg_printf("[%04d][%20s][STM] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_RTP(...)
	#endif

	#if __DBG_RTCP			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_RTCP(...)		(dbg_printf("[%04d][%20s][STM] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_RTCP(...)
	#endif

	#if __DBG_ERR			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_ERR(...)		(dbg_printf("[%04d][%20s][ERR] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_ERR(...)
	#endif

	#if __DBG_DUMP			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_DUMP(...)		dbg_printf( __VA_ARGS__)
	#else
	 #define DBG_PRNTF_DUMP(...)
	#endif

	#if __DBG_V4L2			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_V4L2(...)		(dbg_printf("[%04d][%20s][V4L] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_V4L2(...)
	#endif

	#if __DBG_V4L2			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_V4L2_INF(...)		(dbg_printf("[%04d][%20s][V4L] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_V4L2_INF(...)
	#endif

	#if __DBG_PL			/*!< @brief R/W関数コールのログ出しデバッグ用		*/
	 #define DBG_PRNTF_PL(...)		(dbg_printf("[%04d][%20s][PL_] --- ", __LINE__, __FUNCTION__ ),dbg_printf( __VA_ARGS__))
	#else
	 #define DBG_PRNTF_PL(...)
	#endif
	

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_XC_DBG_H */
