#ifndef __NET_INTREFACE_H_INCLUDE__
#define __NET_INTREFACE_H_INCLUDE__

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern int enum_if(void);
extern int first_if_mac(char* sn, size_t len);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* __NET_INTREFACE_H_INCLUDE__ */
