#ifndef __MY_PIPE_H_INCLUDE__
#define __MY_PIPE_H_INCLUDE__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define BREAK_PIPE break

#define MYPIPE_OPEN(command, model) \
    int the_pipe_result = 0;\
    {\
        if (open_the_pipe(command, model))\
            return (-1);\
\


#define MYPIPE_READ_LINE \
        char the_pipe_buffer[1024];\
        int  pipe_buffer_len = 1024;\
        while(1) {\
            while((!the_pipe_result) && fgets(the_pipe_buffer, pipe_buffer_len, the_global_pipe)) {\
\


#define MYPIPE_CLOSE \
            }\
            BREAK_PIPE;\
        }\
\
        close_the_pipe();\
    }\
\



#define MYPIPE_RETURN_PIPE_RESULT \
        return the_pipe_result;\
\





#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


extern FILE* the_global_pipe;
extern int   open_the_pipe(const char* command, const char* model);
extern void  close_the_pipe();

// extern int read_and_print_pipe(const char* cmd, const char* model);

#ifdef __cplusplus
}
#endif //__cplusplus



#endif //__MY_PIPE_H_INCLUDE__
