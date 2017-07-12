#ifndef __COMMON_DAEMONIZE__
#define __COMMON_DAEMONIZE__


void daemonize(void);
int daemonize_write_pidfile(char *path);


#endif /* __COMMON_DAEMONIZE__ */
