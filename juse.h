#ifndef _JUSE_H
#define _JUSE_H

#if defined (WINDOWS) || defined (WIN32)
#define OS_WINDOWS
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char BOOL;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define jerr printf

#endif
