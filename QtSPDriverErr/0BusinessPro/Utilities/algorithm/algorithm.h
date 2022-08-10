#ifndef ALGORITHM_H
#define ALGORITHM_H


#include "algorithm_global.h"
#include "__common_version_def.h"


#define CRC_ALG_CRC4				(4)			// x4+x+1						3
#define CRC_ALG_CRC8				(8)			// x8+x5+x4+1					31
#define CRC_ALG_CRC8_2				(9)			// x8+x2+x1+1					07
#define CRC_ALG_CRC8_3				(10)		// x8+x6+x4+x3+x2+x1			5E
#define CRC_ALG_CRC12				(12)		// x12+x11+x3+x+1				80F
#define CRC_ALG_CRC16				(16)		// x16+x15+x2+1					8005
#define CRC_ALG_CRC16_CCITT			(17)		// x16+x12+x5+1					1021
#define CRC_ALG_CRC32				(32)		// x32+x26+x23+...+x2+x+1		04C11DB7
#define CRC_ALG_CRC32c				(33)		// x32+x28+x27+...+x8+x6+1		1EDC6F41

#define MAC_ALG_AUTO				(0)			// Single Key: ANSI X 9.9	Double Key: ISO 9797-1	Triple Key: ISO 16609
#define MAC_ALG_ANSI_X99			(1)			// ISO 8731-2 or ANSI X 9.9
#define MAC_ALG_ISO9797_1			(2)			// ISO 9797-1 MAC Algorithm 3
#define MAC_ALG_ISO16609			(3)			// ISO 16609 MAC Algorithm 1
#define MAC_ALG_X919				(4)


class ALGORITHM_EXPORT Algorithm
{
    DEFINE_STATIC_VERSION_FUNCTIONS("algorithm", "0.0.0.0", TYPE_DYNAMIC)
public:
    static bool calc_bcc(unsigned char* in_data, unsigned long in_size, unsigned char* bcc);
    static bool calc_crc(unsigned short algorithm, unsigned char* in_data, unsigned long in_size, unsigned short* crc);
    static bool calc_xor(unsigned char* in_data, unsigned char* out_data, unsigned char* xor_data, unsigned long data_size);
    static bool calc_mac(unsigned short algorithm, unsigned char* in_data, unsigned long in_size,
        unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size);

    static bool crypt_xor(unsigned char* in_out_data, unsigned long in_size, unsigned char xor_data);
    static unsigned long encrypt_des(
        unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size);
    static unsigned long decrypt_des(
        unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size);
    static unsigned long encrypt_3des(
        unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size);
    static unsigned long decrypt_3des(
        unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size);

    static bool get_random_numbers(int min_data, int max_data, int random_list[], int count,
        bool must_different = false, bool sort = false);
};

#endif // ALGORITHM_H
