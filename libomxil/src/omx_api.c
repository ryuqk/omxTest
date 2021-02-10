/************************************************************************************/
/*      @brief     omx functions                                                    */
/*      @file      omx_api.c                                                        */
/*                                                                                  */
/*      @author    xc                                                               */
/*      @copyright xc                                                               */
/*      refer to https://shop.cqpub.co.jp/hanbai/books/47/47101.html                */
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
#include <errno.h>
#include "bcm_host.h"
#include "ilclient.h"

/********************************************************/
/*                                                      */
/********************************************************/


static OMX_VIDEO_PARAM_PORTFORMATTYPE format;
enum Component_E { COMP_DEC=0, COMP_SCHE, COMP_REND, COMP_CLOCK };
static COMPONENT_T *clist[5];
enum Tunnel_E { TUN_DECOUT=0, TUN_RENDIN, TUN_CLOCKOUT };
static TUNNEL_T tunnel[4];
static ILCLIENT_T *client;
static int omx_err = 0;
static OMX_BUFFERHEADERTYPE *omx_buf;
static int omx_port_settings_changed = 0;
static int omx_first_packet = 1;

/****************************************************************************//**
 *  @brief
 *
 *	@param[in]
 *	@retval
 *
 ****************************************************************************** */
int OMX_Open(void)
{
	OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;

	bcm_host_init();	// OpenMAXを使う前に必要
	omx_err = 0;
	memset(clist, 0, sizeof(clist));
	memset(tunnel, 0, sizeof(tunnel));

	memset(&cstate, 0, sizeof(cstate));
	cstate.nSize = sizeof(cstate);

	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));

	if ((client = ilclient_init()) == NULL){
		omx_err = __LINE__;
		return -1;
	}
	if (OMX_Init() != OMX_ErrorNone) {
		ilclient_destroy(client);
		omx_err = __LINE__;
		return -1;
	}

	do {

		if (ilclient_create_component(client, &clist[COMP_DEC], (char *)"video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS)) != 0){
			omx_err = __LINE__;
			break;
		}
		if (ilclient_create_component(client, &clist[COMP_REND], (char *)"video_render", ILCLIENT_DISABLE_ALL_PORTS) != 0){
			omx_err = __LINE__;
			break;
		}
		if (ilclient_create_component(client, &clist[COMP_CLOCK], (char *)"clock", ILCLIENT_DISABLE_ALL_PORTS) != 0){
			omx_err = __LINE__;
			break;
		}
		if (ilclient_create_component(client, &clist[COMP_SCHE], (char *)"video_scheduler", ILCLIENT_DISABLE_ALL_PORTS) != 0){
			omx_err = __LINE__;
			break;
		}

		cstate.nVersion.nVersion = OMX_VERSION;
		cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
		cstate.nWaitMask = 1;
		if (!omx_err && OMX_SetParameter(ILC_GET_HANDLE(clist[COMP_CLOCK]), OMX_IndexConfigTimeClockState, &cstate) != OMX_ErrorNone){
			omx_err = __LINE__;
			break;
		}

		set_tunnel(&tunnel[TUN_DECOUT],   clist[COMP_DEC], 131, clist[COMP_SCHE], 10);
		set_tunnel(&tunnel[TUN_RENDIN],   clist[COMP_SCHE], 11, clist[COMP_REND], 90);
		set_tunnel(&tunnel[TUN_CLOCKOUT], clist[COMP_CLOCK],80, clist[COMP_SCHE], 12);

		if (ilclient_setup_tunnel(&tunnel[TUN_CLOCKOUT], 0, 0) != 0){
			omx_err = __LINE__;
			break;
		}

		format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
		format.nVersion.nVersion = OMX_VERSION;
		format.nPortIndex = 130;
		format.eCompressionFormat = OMX_VIDEO_CodingAVC;

	} while(0);
	
	if (!omx_err) {
		ilclient_change_component_state(clist[COMP_CLOCK], OMX_StateExecuting);
		ilclient_change_component_state(clist[COMP_DEC],   OMX_StateIdle);
		return -1;
	}
	
	return 0;

}

int OMX_Start(void)
{
	if(omx_err != 0){
		 return -1;
	}
	if ((OMX_SetParameter(ILC_GET_HANDLE(clist[COMP_DEC]), OMX_IndexParamVideoPortFormat, &format) == OMX_ErrorNone) && (ilclient_enable_port_buffers(clist[COMP_DEC], 130, NULL, NULL, NULL) == 0))
	{
		ilclient_change_component_state(clist[COMP_DEC], OMX_StateExecuting);
		return 0;
	}else{
		omx_err = __LINE__;
		return -1;
	}
	
}

int OMX_Decode(char *framePtr, unsigned int data_len)
{
	if(omx_err != 0){
		 return -1;
	}
	int remain_len = data_len;
	int decode_len = 0;
	int ofs = 0;
	
	while( remain_len > 0){
		if (( omx_buf = ilclient_get_input_buffer(clist[COMP_DEC], 130, 1)) != NULL){
			if( omx_buf->nAllocLen <= remain_len  ){
				decode_len = omx_buf->nAllocLen;
				remain_len -= omx_buf->nAllocLen;
//				printf("%s L%d %d %d %d / %d \n",__FUNCTION__,__LINE__, ofs, decode_len ,data_len, omx_buf->nAllocLen);
			}else{
				decode_len = remain_len;
				remain_len = 0;
//				printf("%s L%d %d %d %d / %d \n",__FUNCTION__,__LINE__, ofs, decode_len ,data_len, omx_buf->nAllocLen);
			}
			if (decode_len == 0){
				omx_err = __LINE__;
				return -1;
			}
			memcpy(omx_buf->pBuffer, framePtr + ofs, decode_len);
			ofs += decode_len;
			
			if (omx_port_settings_changed == 0 &&
				((decode_len > 0 && ilclient_remove_event(clist[COMP_DEC], OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) ||
				 (decode_len == 0 && ilclient_wait_for_event(clist[COMP_DEC], OMX_EventPortSettingsChanged, 131, 0, 0, 1,
		                                               ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 10000) == 0)))
			{ 
				omx_port_settings_changed = 1;

				if (ilclient_setup_tunnel(&tunnel[TUN_DECOUT], 0, 0) != 0) {
					omx_err = __LINE__; 
					return -1;
				}
				ilclient_change_component_state(clist[COMP_SCHE], OMX_StateExecuting);

				if (ilclient_setup_tunnel(&tunnel[TUN_RENDIN], 0, 1000) != 0) {
					omx_err = __LINE__;
					return -1;
				}
				ilclient_change_component_state(clist[COMP_REND], OMX_StateExecuting);
			}


			omx_buf->nFilledLen = decode_len;
			decode_len = 0;

			omx_buf->nOffset = 0;
			if (omx_first_packet) {
				omx_buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
				omx_first_packet = 0;
			}
			else
				omx_buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;

			if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(clist[COMP_DEC]), omx_buf) != OMX_ErrorNone) {
				omx_err = __LINE__;
				return -1;
			}
		}else{
				printf("%s L%d %d %d %d \n",__FUNCTION__,__LINE__, ofs, decode_len ,data_len);
		
		}
	}
	return 0;
}

int OMX_Stop(void)
{
	omx_buf->nFilledLen = 0;
	omx_buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

	if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(clist[COMP_DEC]), omx_buf) != OMX_ErrorNone){
		omx_err = __LINE__;
	}

	// wait for EOS from render
	ilclient_wait_for_event(clist[COMP_REND], OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, 10000);

	// need to flush the renderer to allow clist[COMP_DEC] to disable its input port
	ilclient_flush_tunnels(tunnel, 0);

	ilclient_disable_port_buffers(clist[COMP_DEC], 130, NULL, NULL, NULL);
	omx_port_settings_changed = 0;
	return 0;
}

int OMX_Close(void)
{
	ilclient_disable_tunnel(&tunnel[0]);
	ilclient_disable_tunnel(&tunnel[1]);
	ilclient_disable_tunnel(&tunnel[2]);
	ilclient_teardown_tunnels(tunnel);

	ilclient_state_transition(clist, OMX_StateIdle);
	ilclient_state_transition(clist, OMX_StateLoaded);

	ilclient_cleanup_components(clist);

	OMX_Deinit();

	ilclient_destroy(client);
	return 0;
}

int OMX_lastErr(void)
{
	int ret = omx_err;
	omx_err = 0;
	return ret;
}
