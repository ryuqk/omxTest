/************************************************************************************/
/*      @brief     omx functions                                                    */
/*      @file      omx_api.h                                                        */
/*                                                                                  */
/*      @author    xc                                                               */
/*      @copyright xc                                                               */
/*                                                                                  */
/************************************************************************************/
#ifndef INCLUDE_XC_OMX_API_H
#define INCLUDE_XC_OMX_API_H

#ifdef __cplusplus
extern "C" {
#endif


extern int OMX_Open(void);
extern int OMX_Start(void);
extern int OMX_Decode(char *framePtr, unsigned int len);
extern int OMX_Stop(void);
extern int OMX_Close(void);
extern int OMX_lastErr(void);

#ifdef __cplusplus
}
#endif


#endif	/* INCLUDE_XC_OMX_API_H */
