#ifndef __APPS_3_SIGNAL__
#define __APPS_3_SIGNAL__


#include <errno.h>     /* errno */
#include <stdio.h>     /* printf, fprintf */
#include <signal.h>    /* SIGINT, SIGTERM */
#include <string.h>    /* strerror */
#include <semaphore.h> /* sem_t, sem_init, sem_wait, sem_post, sem_destroy */
#include <sysexits.h>  /* EX_OSERR */


int signal_init(void);
int signal_wait(void);


#endif /* __APPS_3_SIGNAL__ */
