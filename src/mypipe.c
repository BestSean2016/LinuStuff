#include "mypipe.h"

FILE* the_global_pipe = 0;


int open_the_pipe(const char* command, const char* model) {
    return ((NULL == (the_global_pipe = popen(command, model))) ?
        (-1) : (0));
}

void close_the_pipe() {
    if (the_global_pipe) {
        pclose(the_global_pipe);
        the_global_pipe = 0;
    }
}



/*
int read_and_print_pipe(const char* cmd, const char* model) {
    MYPIPE_OPEN(cmd, model)
    MYPIPE_READ_LINE
        printf("%s", the_pipe_buffer);
    MYPIPE_CLOSE    
}
*/


/*
{
    int the_pipe_result = 0;
    if (open_the_pipe(cmd, model))
        return (-1);
    
    char the_pipe_buffer[1024];
    int  pipe_buffer_len = 1024;
    while(1) {
        while(fgets(the_pipe_buffer, pipe_buffer_len, the_global_pipe)) {
            printf("%s", the_pipe_buffer);
        }
        BREAK_PIPE;
    }
    
    close_the_pipe();
    return the_pipe_result;
}
*/
