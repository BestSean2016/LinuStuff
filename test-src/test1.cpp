/*************************************************************************************
 * 
 * VERY VERY IMPORTANT
 * 
 * --- prequest : ---
 * 1. before you do this test, you must install two libraries and develop libraries
 *    libparted, libuuid and libparted-devel, libuuid-devel
 * 2. you must install the command hdparm
 * 3. make sure you have 'fdisk' command and run as well
 * 4. you must run this programm as root
 * 
 *************************************************************************************/

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "readdisk.h"
#include "mypipe.h"
#include "des.h"
#include "base64.h"
#include "license.h"
#include <string.h>


int read_and_print_pipe(const char* cmd, const char* model) {
    MYPIPE_OPEN(cmd, model)
    MYPIPE_READ_LINE
        printf("%s", the_pipe_buffer);
    MYPIPE_CLOSE
    MYPIPE_RETURN_PIPE_RESULT
}


TEST_CASE( "Teset for Read Hard Disk Serial Number", "ReadDiskSN") {
    if (getuid()) {
        printf("Warning: You must run me as root!\n");
        exit(0);
    }
    
    /////////////////////////////////////////////////////////////
    // read hard disk sn
    // printf("Get Hard Disk and Partition:\n");
    // read_and_print_pipe("fdisk -l", "r");
    // printf("\n\n");
    // 
    // get the identify of hard disk
    // printf("Get Hard Disk Identify:\n");
    // read_and_print_pipe("hdparm -i /dev/sda", "r");
    // printf("\n\n");
    /////////////////////////////////////////////////////////////

    char myFirstHD_1[128];
    int result = get_first_hd_via_pipe(myFirstHD_1, 128);
    REQUIRE( result == 0 );
    
    char myFirstHD_2[128];
    result = get_first_hd(myFirstHD_2, 128);   
    REQUIRE( result == 0 );
    
    printf("-Hard Disk path - 1: %s, 2: %s\n", myFirstHD_1, myFirstHD_2);
    REQUIRE( !strcmp(myFirstHD_1, myFirstHD_2) );
    
    
    char szSerialNo_1[256];
    result = get_first_hdsn_via_pipe(myFirstHD_1, szSerialNo_1, 256);
    REQUIRE( result == 0 );
    
    char szSerialNo_2[256];
    result = get_first_hdsn(szSerialNo_2, 256);
    REQUIRE( result == 0 );
    
    printf("-Serial Number- 1: %s, 2: %s\n", szSerialNo_1, szSerialNo_2);
    REQUIRE( !strcmp(szSerialNo_1, szSerialNo_2) );
}


TEST_CASE( "Test for DES, 3DES", "DES") {
    printf("test des\n");
    
    int result = mbedtls_des_self_test(8);
    
    REQUIRE(result == 0);
    
    
    char *path = getenv("HOME");
    printf("path = %s\n",path);
    
    REQUIRE(!strcmp(path, "/root"));
}

TEST_CASE( "Test for Base64", "base64") {
    printf("test base64\n");
    
    int result = mbedtls_base64_self_test(8);
    REQUIRE(result == 0);    
}

TEST_CASE( "Test for String Function", "String") {
    printf("test append_char\n");
    
    char* buf = 0;
    int sizeofbuf = 10;
    buf = (char*)malloc(sizeofbuf);

    strncpy(buf, "12345", 9);
    char* p = append_char(buf, sizeofbuf, 'a', 3);
    REQUIRE(p == buf);
    REQUIRE(!strcmp(buf, "12345aaa"));
    
    strncpy(buf, "12345678", 9);
    p = append_char(buf, sizeofbuf, 'a', 3);
    REQUIRE(p == 0);
    REQUIRE(!strcmp(buf, "12345678"));
    
    
    printf("test append_string\n");
    char* large_buf = 0;
    int sizeof_large_buf = 128;
    large_buf = (char*)malloc(sizeof_large_buf);
    strncpy(large_buf, "abcdef", sizeof_large_buf);
    p = append_string(large_buf, sizeof_large_buf, buf, 5);
    REQUIRE(p == large_buf);
    REQUIRE(!strcmp(large_buf, "abcdef1234567812345678123456781234567812345678"));
    

    strncpy(buf, "12345678", 9);
    strncpy(large_buf, "abcdef", sizeof_large_buf);
    for (int i = 1; i < 16; ++i) {
        //printf("%d: %s\n", i, large_buf);
        p = append_string(large_buf, sizeof_large_buf, buf, 1);
        REQUIRE(p == large_buf);
    }    
    p = append_string(large_buf, sizeof_large_buf, buf, 1);
    REQUIRE(p == 0);
    free(large_buf);
    free(buf);

    printf("test align_multi_8\n");
    sizeofbuf = 12;
    buf = (char*)malloc(sizeofbuf);
    strncpy(buf, "12345678", sizeofbuf);
    int result = align_multi_8(buf, sizeofbuf);
    REQUIRE(result == 0);
    REQUIRE(!strcmp(buf, "12345678   "));
    REQUIRE(!strcmp((char*)gszHdsn, "12345678   12345678   12345678   12345678   12345678   12345678   12345678   12345678   "));
    free(buf);
    
    sizeofbuf = 20;
    buf = (char*)malloc(sizeofbuf);
    strncpy(buf, "1234567812345678", sizeofbuf);
    result = align_multi_8(buf, sizeofbuf);
    REQUIRE(result == 0);
    REQUIRE(!strcmp(buf, "1234567812345678   "));
    REQUIRE(!strcmp((char*)gszHdsn, "1234567812345678   1234567812345678   1234567812345678   1234567812345678   1234567812345678   1234567812345678   1234567812345678   1234567812345678   "));


    strncpy(buf, "123456781234", sizeofbuf);
    result = align_multi_8(buf, sizeofbuf);
    REQUIRE(result == 0);
    REQUIRE(!strcmp(buf, "123456781234"));
    REQUIRE(!strcmp((char*)gszHdsn, "123456781234123456781234123456781234123456781234123456781234123456781234123456781234123456781234"));
    free(buf);

    sizeofbuf = strlen("WD-WCAV56962982") + 1;
    buf = (char*)malloc(sizeofbuf);
    strncpy(buf, "WD-WCAV56962982", sizeofbuf);
    result = align_multi_8(buf, sizeofbuf);
    REQUIRE(result == 0);
    REQUIRE(!strcmp(buf, "WD-WCAV56962982"));
    REQUIRE(!strcmp((char*)gszHdsn, "WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982"));
    free(buf);   
}


#define test_disk_serial_number "WD-WCAV56962982"

TEST_CASE("encode / decode", "enc/dec") {
    printf("test encode\n");
    
    memset(gszHdsn, 0, sizeof(gszHdsn));
    memset(gszEntext, 0, sizeof(gszEntext));
    
    int result = encode_hd_sn(test_disk_serial_number);
    REQUIRE(result == strlen("WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982WD-WCAV56962982"));
    
    size_t n = 0;
    result = mbedtls_base64_encode(gszEntext, sizeof(gszEntext), &n, gszHdsn, result);
    REQUIRE(result == 0);
    REQUIRE(n == strlen((char*)gszEntext));

    char* p = strdup((char*)gszEntext);
    
    result = encode_hd_sn_base64(test_disk_serial_number);
    REQUIRE(result == 0);
    REQUIRE(!strcmp(p, (char*)gszEntext));

    printf("encoded-hd-sn is '%s'\n", p);

    printf("test decode\n");
    result = decode_hd_sn_base64(p);
    REQUIRE(result == 0);
    REQUIRE(!strcmp((char*)gszHdsn, test_disk_serial_number));
    
    free(p);    
}

#define expire_date "2016-01-16 01:02:03"

TEST_CASE("generate_license", "generate_license") {
    printf("test generate_license\n");

    int result = generate_license("license.txt", test_disk_serial_number, expire_date);
    REQUIRE(result == 0);
    
    char szHdsn[512];
    char szDate[32];
    result = get_license("license.txt", szHdsn, sizeof(szHdsn), szDate, sizeof(szDate));
    REQUIRE(result == 0);
    REQUIRE(!strcmp(szHdsn, test_disk_serial_number));
    REQUIRE(!strcmp(szDate, expire_date));
}


