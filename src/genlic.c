#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "readdisk.h"
#include "serialno.h"


static int get_input(char* txt,  size_t maxlen) {
    char *p = 0, *tail = 0;
    p = fgets(txt, maxlen, stdin);
    if (!p) return (-1);

    tail = txt + strlen(txt) - 1;
    while(isspace((*tail))) {
        *tail = 0;
        --tail;
    }
    return (0);
}

int main(int argc, char* argv[]) {
    int result = 0;
    char txt[1024];
    
    while(1) {
        printf("Please Enter the Serial Number:\n");
        result = get_input(txt, sizeof(txt));
        if (result) break;
        
        result = decode_hd_sn_base64(txt);
        if (result) break;
        
        printf("the Serial Number is:\n%s\n\n", (char*)gszHdsn);
        
        printf("Please Enter a Expire Date(int yyyy-mm-dd hh:mm:ss format):\n");
        result = get_input(txt, sizeof(txt));
        if (result) break;
        
        result = generate_license("the_license", (char*)gszHdsn, txt);
        break;
    }
    
    if (result)
        printf("Error occured!\n");
    
    return result;
}
