#ifndef _IO_H_
#define _IO_H_

int gr_openstream(char *path);
void gr_writestream(char *string, ...);
void gr_flushstream(int discard);
void gr_closestream(void);

#endif
