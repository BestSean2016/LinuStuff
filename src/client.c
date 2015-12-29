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


static int _do_you_want_to_license(const char* filepath) {
    char buf[1024];
    char* p = 0;
    
    if (getuid()) {
        printf("Warning: You must run me as root!\n");
        exit(0);
    }

show_hit:
    printf("Can not open your license file at '%s'.\n", filepath);
    printf("Do you want to a new license file (You must run me as root)? [y/n]:\n");
    p = fgets(buf, 1023, stdin);

    if (!p) {
        printf("Error when I read your input!\n");
        return (-1);
    }
    
    if (!strcasecmp(buf, "yes\n") || !strcasecmp(buf, "y\n"))
        return 1;
    else if (!strcasecmp(buf, "no\n") || !strcasecmp(buf, "n\n"))
        return 0;
    else
        goto show_hit;
}    

static int _generate_hdsn(const char* filepath) {
    char szSN[512];
    int result = _do_you_want_to_license(filepath);
    if (result == -1) return (-1);
    
    if (result == 1) { //yes
        result = get_first_hdsn(szSN, sizeof(szSN));
        if (!result)
            result = encode_hd_sn_base64(szSN);
        
        if (!result)
            printf("Your serial number is :\n---------------------\n%s\n---------------------\n"
                "Please Copy It and Send to stoneguo@126.com with subject 'Serial Number for License', Thanks!\n",
                gszEntext);
        else
            printf("Error occured when get serial number, Do You Run Me as root?\n");
    }
    
    return (0);
}

static int check_date_now(const char* szDate) {
    struct tm t;
    struct tm *lt;
    time_t now = time(0);
    time_t expire = 0;
    char* p = (char*)szDate;
    
    lt = localtime(&now);
    memcpy(&t, lt, sizeof(struct tm));
    
    t.tm_year = atoi(p) - 1900;
    p += 5;
    t.tm_mon = atoi(p) - 1;
    p += 3;
    t.tm_mday = atoi(p);
    p += 3;
    t.tm_hour = atoi(p);
    p += 3;
    t.tm_min = atoi(p);
    p += 3;
    t.tm_sec = atoi(p);
    
    expire = mktime(&t);

    return (now >= expire);
}

static int _get_license(const char* filepath) {
    char szHdsn[512];
    char szDate[32];
    char szSN[512];
    int result = get_license(filepath, szHdsn, sizeof(szHdsn), szDate, sizeof(szDate));
    if (result) {
        printf("error\n");
        return (-1);
    }
    
    result = get_first_hdsn(szSN, sizeof(szSN));
    if (!strcmp(szSN, szHdsn))
        printf("The Serial Number Was Checked!\n");
    else {
        printf("Wrong Serial Number!\n");
        return (-2);
    }
    
    result = check_date_now(szDate);
    
    if (result)
        printf("No Good! you have a invaild license.\n");
    else
        printf("Everthing is OK!\n");
        
    return (0);
}

int main(int argc, char* argv[]) {
    int result = 0;
    struct stat buf;
    if (argc != 2) {
        printf("Usage: client <license path>\n");
        return result;
    }
    
    /* check the license file */
    result = stat(argv[1], &buf);
    if (result)
        return _generate_hdsn(argv[1]);
    else
        return _get_license(argv[1]);

    return (0);
}
