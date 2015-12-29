#ifndef __READ_HARD_DISK_H_INCLUDE__
#define __READ_HARD_DISK_H_INCLUDE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
/* we need parted and uuid library and devel library */
#include <parted/parted.h>




#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


extern void print_hard_disk_lisk(void);
extern int  get_first_hd(char* devName, int len);
extern int  get_first_hdsn(char* szSN, int nLimit);
extern int  get_first_hd_via_pipe(char* devName, int len);
extern int  get_first_hdsn_via_pipe(const char* szDevName, char* szSN, int nLimit);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__READ_HARD_DISK_H_INCLUDE__

