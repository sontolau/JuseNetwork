#ifndef _JUTILITIES_H
#define _JUTILITIES_H

#include "jmodule.h"

#ifdef JUSE_SERVER_SIDE

#elif defined (JUSE_CLIENT_SIDE)

typedef void* HJUSE;

extern HJUSE J_ConnectToJuseNetwork (const char *proto,
                                   const char *ip, 
                                   unsigned short port);

extern int J_SendData (HJUSE, void *data, unsigned int len);

extern int J_RecvData (HJUSE, void **data, unsigned int szbuf);

extern int J_GetErrorCode (HJUSE);


extern void J_DisconnectFromJuseNetwork (HJUSE);

#endif

#endif
