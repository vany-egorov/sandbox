#ifndef __SIGNAL_H__
#define __SIGNAL_H__


#include <errno.h>     /* errno */
#include <stdio.h>     /* printf, fprintf */
#include <signal.h>    /* SIGINT, SIGTERM */
#include <semaphore.h> /* sem_t, sem_init, sem_wait, sem_post, sem_destroy */


extern volatile sig_atomic_t signal_sigstop_received;


int signal_wait(void);
int signal_init(void);


#endif /* __SIGNAL_H__ */
