#ifndef __SERIAL_NUMBER_H_INCLUDE__
#define __SERIAL_NUMBER_H_INCLUDE__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int encode_expire(const char* szHdsn, const char* szDate, char* szDateCode, size_t maxlen);
extern int decode_expire(const char* szDateCode, const char* szHdsn, time_t* tDate);
/**
 * \brief check the expire that saved in license filepath
 * \param filepath, the path of license file
 * \param szHdsn, the serial number of first hard disk
 * \return 1 if it was expired, else return 0
 */
extern int check_expire(const char* filepath, const char* szHdsn);

extern char* append_char(char* buffer, size_t maxlen, char c, size_t n);
extern char* append_string(char* buffer, size_t maxlen, const char* str, size_t n);
extern int   align_multi_8(char* szHdsn, size_t maxlen);

extern unsigned char gszHdsn[1025];
extern unsigned char gszEntext[8193];

/**
 * \brief encode the serial number of first hard disk with the default key
 * \return if succssed return the length encoded - text, else return < 0
 */
extern int encode_hd_sn(const char* szHdsn);
extern int encode_hd_sn_base64(const char* szHdsn);
extern int decode_hd_sn_base64(const char* szEnText);


extern int generate_license(const char* filepath, const char* szHdsn, const char* szDate);
extern int get_license(const char* filepath, char* szHdsn, size_t hdsn_len, char* szDate, size_t date_len);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_NUMBER_H_INCLUDE__ */
