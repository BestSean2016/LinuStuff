#include <stdio.h>
#include <sys/stat.h>

#include "readdisk.h"
#include "des.h"
#include "base64.h"
#include "license.h"


static const unsigned char the_key[24] =
{
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 
    0x32, 0x10, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 
    0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC, 0xBA, 0x98, 
};

static const unsigned char the_key2[24] =
{
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 
    0x54, 0x32, 0x10, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 
    0x98, 0x76, 0x54, 0x32, 0x10, 0xFE, 0xDC, 0xBA, 
};

unsigned char gszHdsn[1025];
unsigned char gszEntext[8193];

#define times_for_encrypt   10000

/**
 * return the length of ecrypt code
 */
static int _encode_hd_sn_(char* str, const unsigned char key[24]) {
    int result = 0;
    int j = 0;
    unsigned char* p = (unsigned char*)str;
    int len = (int)strlen((char*)str);
    int i = 0;
    
    mbedtls_des3_context ctx3;
    mbedtls_des3_init( &ctx3 );
    mbedtls_des3_set3key_enc( &ctx3, key );
    
    
    for (; !result && j < times_for_encrypt; ++j) {
        for (; i < len; i += 8, p += 8) {
            result = mbedtls_des3_crypt_ecb(&ctx3, p, p);
        }
    }

    mbedtls_des3_free( &ctx3 );
    return (result) ? -1 : len;
}

char* append_char(char* buffer, size_t maxlen, char c, size_t n) {
    size_t i = 0;
    size_t len = strlen(buffer);
    if (maxlen < len  + n + 1)
        return (0);
    for (; i < n; ++i)
        buffer[len + i] = c;
    
    buffer[len + i] = 0;
    return buffer;
}


char* append_string(char* buffer, size_t maxlen, const char* str, size_t n) {
    size_t i = 0;
    size_t blen = strlen(buffer);
    size_t slen = strlen(str);
    
    if (maxlen < blen + n * slen + 1)
        return (0);
    
    for (; i < n; ++i) {
        strncpy(buffer + blen, str, maxlen - blen - 1);
        blen += slen;
    }
    
    buffer[blen + slen] = 0;
    return buffer;
}


/**
 * 
 */
int align_multi_8(char* szHdsn, size_t maxlen) {
    char * p = (char*)gszHdsn;

    memset(gszHdsn, 0, sizeof(gszHdsn));
    if (!(strlen(szHdsn) % 8))
        p = append_char(szHdsn, maxlen, ' ', 3);
    
    if (p)
        p = append_string((char*)gszHdsn, sizeof(gszHdsn), szHdsn, 8);
    
    return (p) ? 0 : (-1);
}

int encode_hd_sn(const char* szHdsn) {
    int result = 0;
    int maxlen = strlen(szHdsn) + 9;
    char* buf = (char*)malloc(maxlen);
    
    memset(gszEntext, 0, sizeof(gszEntext));

    strncpy(buf, szHdsn, maxlen);
    if (0 != (result = align_multi_8(buf, maxlen)))
        return -1;
    
    return _encode_hd_sn_((char*)gszHdsn, the_key);
}


static int _decode_hd_sn_(unsigned char* str, size_t txtlen, const unsigned char key[24]) {
    int result = 0;
    size_t i = 0;
    int j = 0;
    unsigned char* p = str;
    
    mbedtls_des3_context ctx3;
    mbedtls_des3_init( &ctx3 );
    mbedtls_des3_set3key_dec( &ctx3, key );
    
    
    for (; !result && j < times_for_encrypt; ++j) {
        for (; i < txtlen; i += 8, p += 8)
            result = mbedtls_des3_crypt_ecb(&ctx3, p, p);
    }

    mbedtls_des3_free( &ctx3 );
    
    return (result);
}

int encode_hd_sn_base64(const char* szHdsn) {
    int result = 0;
    size_t n = 0;
    if (0 > (result = encode_hd_sn(szHdsn)))
        return -2;
    
    result = mbedtls_base64_encode(gszEntext, sizeof(gszEntext), &n, gszHdsn, result);
    if (result != 0 || n != strlen((char*)gszEntext))
        return -3;
    
    return (0);
}


int decode_hd_sn_base64(const char* szEnText) {
    size_t n = 0;
    int result = 0;
    
    memset(gszEntext, 0, sizeof(gszEntext));
    memset(gszHdsn, 0, sizeof(gszHdsn));
    
    if (sizeof(gszEntext) <= strlen(szEnText))
        return (-1);

    result = mbedtls_base64_decode(gszEntext, sizeof(gszEntext), &n, (unsigned char*)szEnText, strlen((char*)szEnText));    
    if (result) return (-2);
    
    result = _decode_hd_sn_(gszEntext, n, the_key);
    if (result) return (-3);
    
    
    n = strlen((char*)gszEntext);
    if (n % 8) return (-4);
   
    n = n / 8;
    if (n >= sizeof(gszHdsn)) return (-5);
    
    memcpy(gszHdsn, gszEntext, n);
    gszHdsn[n] = 0;

    return (0);
}


static int write_license_file(const char* filepath, const char* enbuf) {
    int result = 0;
    FILE* fp = fopen(filepath, "w");
    if (!fp) return (-1);
    
    if (strlen(enbuf) != fwrite(enbuf, 1, strlen(enbuf), fp))
        result = -2;
    
    fclose(fp);
    
    if (!result) {
        char command[1024];
        snprintf(command, 1024, "chmod +r %s", filepath);
        system(command);
    }
    return (result);
}

int generate_license(const char* filepath, const char* szHdsn, const char* szDate)
{
    int result = 0;
    size_t len = strlen(szHdsn) + strlen(szDate) + 8;
    char* buffer = (char*)malloc(len);
    char* enbuf  = (char*)malloc(len * 4);
    size_t buflen =0;
    
    strcpy(buffer, szHdsn);
    strcat(buffer, szDate);
    
    if ((len - 8) % 8)
        append_char(buffer, len, ' ', 8 - len % 8); // (len - 8) % 8  = len % 8
    buflen = strlen(buffer);
    
    while(1) {
        size_t n = 0;
        if (1 > (result =  _encode_hd_sn_(buffer, the_key2))) {
            result = -3;
            break;
        }

        if (0 != (result = mbedtls_base64_encode((unsigned char*)enbuf, len * 4, &n, (unsigned char*)buffer, buflen)))
            break;
        
        result = write_license_file(filepath, enbuf);
        break;
    }
    
    if (enbuf)  free(enbuf);
    if (buffer) free(buffer);

    return (result);
}


static int get_license_file(const char* filepath, char* buf, size_t len)
{
    int result = 0;
    FILE* fp = fopen(filepath, "r");
    if (!fp) return -1;
    if (NULL == fgets(buf, len, fp))
        result = -2;
    fclose(fp);
    return result;
}


static int split_hdsn_date(char* buffer, char* szHdsn, size_t hdsn_len, char* szDate, size_t date_len) {
    int result = 0;
    //remove space
    char* buf = (char*)buffer + strlen(buffer) - 1;
    while(isspace(*buf))
        buf--;
    *(++buf) = 0;
    
    //copy date
    strncpy(szDate, buf - strlen("2016-01-16 01:02:03"), date_len);
    
    //copy hdsn
    {
        char tmp = 0;
        tmp = buffer[buf - buffer - strlen("2016-01-16 01:02:03")];
        buffer[buf - buffer - strlen("2016-01-16 01:02:03")] = 0;
        strncpy(szHdsn, buffer, hdsn_len);
        buffer[buf - buffer - strlen("2016-01-16 01:02:03")] = tmp;
    }
    
    return result;
}

int get_license(const char* filepath, char* szHdsn, size_t hdsn_len, char* szDate, size_t date_len)
{
    size_t n = 0;
    size_t maxlen = hdsn_len + date_len + 1;
    char* buffer = (char*)calloc(1, maxlen);
    char* enbuf = (char*)calloc(1, maxlen);
    
    int result = get_license_file(filepath, enbuf, maxlen);
    if (result) return (-1);
    
    result = mbedtls_base64_decode((unsigned char*)buffer, maxlen, &n, (unsigned char*)enbuf, strlen(enbuf));
    if (result) return (-2);
    
    result = _decode_hd_sn_((unsigned char*)buffer, n, the_key2);
    if (result) return (-3);
    
    result = split_hdsn_date(buffer, szHdsn, hdsn_len, szDate, date_len);
    return (result);
}


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
            printf("Error occured when get serial number.\n");
    }
    
    return (1);
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
    exit(-1005);
    }
   
    result = get_first_hdsn(szSN, sizeof(szSN));
    if (!strcmp(szSN, szHdsn)) {
        //printf("The Serial Number Was Checked!\n");
    } else {
        printf("Wrong Serial Number. '%s'\n", szSN);
        exit(-1004);
    }
    
    result = check_date_now(szDate);
    
    if (result) {
        printf("No Good! you have a invaild license.\n");
    exit(-1003);
    } else {
        //printf("Everthing is OK!\n");
    }

    return (0);
}

int check_license(const char* filepath) {
    int result = 0;
    struct stat buf;
    
    /* check the license file */
    result = stat(filepath, &buf);
    if (result)
        return _generate_hdsn(filepath);
    else
        return _get_license(filepath);

    return (0);
}

