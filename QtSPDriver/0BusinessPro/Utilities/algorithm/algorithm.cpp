#include "framework.h"
#include "algorithm.h"


using namespace std;


static CryptDes g_des;



/**
 @功能：	计算BCC
 @参数：	in_data：输入数据				in_size：输入数据长度
 @		bcc：BCC数据Buffer（1字节变量地址）
 @返回：	true：成功		false：失败
 */
bool Algorithm::calc_bcc(unsigned char* in_data, unsigned long in_size, unsigned char* bcc)
{
    unsigned long idx = 0;

    if ((in_data == NULL) || (in_size == 0) ||
        (bcc == NULL)) {
        return false;
    }

    *bcc = in_data[0];
    for (idx = 1; idx < in_size; idx++) {
        *bcc ^= in_data[idx];
    }

    return true;
}

/**
 @功能：	计算CRC
 @参数：	algorithm：算法（只支持CRC_ALG_CRC16/CRC_ALG_CRC16_CCITT）
 @		in_data：输入数据				in_size：输入数据长度
 @		crc：CRC数据Buffer
 @返回：	true：成功		false：失败
 */
bool Algorithm::calc_crc(unsigned short algorithm, unsigned char* in_data, unsigned long in_size, unsigned short* crc)
{
    unsigned short crc_reg = 0x0000;
    unsigned short crc_polynomial = 0x0000;
    int i = 0;
    unsigned long idx = 0;
    unsigned short crc_data = 0x0000;

    if ((algorithm == 0) || (in_data == NULL) ||
        (in_size == 0) || (crc == NULL)) {
        return false;
    }

    switch (algorithm) {
    case CRC_ALG_CRC16:
        crc_polynomial = 0xA001;
        for (idx = 0; idx < in_size; idx++) {
            crc_reg ^= in_data[idx];
            for (i = 0; i < 8; i++) {
                if (crc_reg & 0x0001) {
                    crc_reg >>= 1;
                    crc_reg ^= crc_polynomial;
                } else {
                    crc_reg >>= 1;
                }
            }
        }
        break;
    case CRC_ALG_CRC16_CCITT:
        crc_polynomial = 0x1021;
        for (idx = 0; idx < in_size; idx++) {
            crc_data = in_data[idx];
            crc_data <<= 8;

            for (i = 0; i < 8; i++)
            {
                if ((crc_reg ^ crc_data) & 0x8000)
                {
                    crc_reg = (crc_reg << 1) ^ crc_polynomial;
                }
                else
                {
                    crc_reg <<= 1;
                }
                crc_data <<= 1;
            }
        }
        break;
    default:
        return false;
        break;
    }

    *crc = crc_reg;

    return true;
}

/**
 @功能：	XOR计算
 @参数：	in_data：数据					out_data：输出Buffer
 @		xor_data：XOR数据				data_size：数据长度
 @返回：	true：成功		false：失败
 */
bool Algorithm::calc_xor(unsigned char* in_data, unsigned char* out_data, unsigned char* xor_data, unsigned long data_size)
{
    if ((in_data == NULL) || (out_data == NULL) ||
        (xor_data == NULL) || (data_size == 0)) {
        return false;
    }

    while (data_size > 0) {
        *out_data++ = (*in_data++) ^ (*xor_data++);
        data_size--;
    }

    return true;
}

/**
 @功能：	MAC计算
 @参数：	algorithm: 算法
 @		in_data：数据				in_size：数据长度
 @		key：MAC秘钥					key_size：秘钥长度
 @		out_data：输出Buffer			out_buf_size：输出Buffer长度
 @返回：	true：成功		false：失败
 */
bool Algorithm::calc_mac(unsigned short algorithm, unsigned char* in_data, unsigned long in_size,
    unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size)
{
    unsigned char local_in_data[8] = { 0 };
    unsigned char local_out_data[8] = { 0 };
    unsigned char vec[8] = { 0 };
    unsigned char tmp_key[24] = { 0 };
    unsigned long loop = 0;
    unsigned long rem = 0;
    unsigned long idx = 0;
    unsigned long tmp_in_size = 0;

    if ((in_data == NULL) || (in_size == 0) ||
        (key == NULL) ||
        (out_data == NULL) || (out_buf_size == 0)) {
        return false;
    }

    switch (algorithm)
    {
    case MAC_ALG_ANSI_X99:
        if (key_size != 8) {
            return false;
        }
        memcpy(tmp_key, key, key_size);
        break;
    case MAC_ALG_ISO9797_1:
    case MAC_ALG_X919:
    case MAC_ALG_ISO16609:
        switch (key_size) {
        case 8:
            memcpy(tmp_key, key, key_size);
            memcpy(tmp_key + 8, key, 8);
            memcpy(tmp_key + 16, key, 8);
            break;
        case 16:
            memcpy(tmp_key, key, key_size);
            memcpy(tmp_key + 16, key, 8);
            break;
        case 24:
            memcpy(tmp_key, key, key_size);
            break;
        default:
            return false;
            break;
        }
        break;
    case MAC_ALG_AUTO:
        switch (key_size) {
        case 8:
            algorithm = MAC_ALG_ANSI_X99;
            break;
        case 16:
            algorithm = MAC_ALG_ISO9797_1;
            break;
        case 24:
            algorithm = MAC_ALG_ISO16609;
            break;
        default:
            return false;
            break;
        }
        return calc_mac(algorithm, in_data, in_size, key, key_size, out_data, out_buf_size);
        break;
    default:
        break;
    }

    memset(vec, 0x00, sizeof(vec));
    loop = in_size / 8;
    rem = in_size % 8;
    if (rem > 0) {
        loop++;
    }

    tmp_in_size = in_size;
    for (idx = 0; idx < loop; idx++) {
        memset(local_in_data, 0x00, sizeof(local_in_data));
        memset(local_out_data, 0x00, sizeof(local_out_data));
        if (tmp_in_size < 8) {
            memcpy(local_in_data, in_data + idx * 8, tmp_in_size);
        } else {
            memcpy(local_in_data, in_data + idx * 8, 8);
            tmp_in_size -= 8;
        }
        if (algorithm == MAC_ALG_X919) {
            calc_xor(vec, local_out_data, local_in_data, 8);
            if (loop - idx != 1) {
                encrypt_des(local_out_data, sizeof(local_out_data), tmp_key, 8, vec, sizeof(vec));
            } else {
                encrypt_3des(local_out_data, sizeof(local_out_data), tmp_key, key_size, vec, sizeof(vec));
            }
        }
        else if ((algorithm == MAC_ALG_ANSI_X99) || (algorithm == MAC_ALG_ISO9797_1)) {
            // Ci＝eK(Pi⊕Ci-1)
            calc_xor(vec, local_out_data, local_in_data, 8);
            encrypt_des(local_out_data, sizeof(local_out_data), tmp_key, 8, vec, sizeof(vec));
        } else {	// MAC_ALG_ISO16609
            // Ci＝eK3（dK2（eK1(Pi⊕Ci-1)）
            calc_xor(vec, local_out_data, local_in_data, 8);
            encrypt_3des(local_out_data, sizeof(local_out_data), tmp_key, 16, vec, sizeof(vec));
        }
    }

    if (algorithm == MAC_ALG_ISO9797_1) {
        // Cr＝dKr(Cn)
        decrypt_des(vec, sizeof(vec), tmp_key + 8, 8, local_out_data, sizeof(local_out_data));
        // Cl＝eKl(Cr)
        encrypt_des(local_out_data, sizeof(local_out_data), tmp_key + 16, 8, out_data, out_buf_size);
    } else {
        if (out_buf_size < sizeof(vec)) {
            memcpy(out_data, vec, out_buf_size);
        } else {
            memcpy(out_data, vec, sizeof(vec));
        }
    }

    return true;
}


/**
 @功能：	异或加密
 @参数：	in_data：被加密数据及加密结果Buffer
 @		in_size：被加密数据长度
 @		xor_data：异或值
 @返回：	true：成功		false：失败
 */
bool Algorithm::crypt_xor(unsigned char* in_out_data, unsigned long in_size, unsigned char xor_data)
{
    unsigned long idx = 0;

    if ((in_out_data == NULL) || (in_size == 0)) {
        return false;
    }

    if (xor_data == 0x00) {
        return true;
    }

    for (idx = 0; idx < in_size; idx++)
    {
        in_out_data[idx] ^= xor_data;
    }

    return true;
}

/**
 @功能：	单DES加密
 @参数：	in_data：被加密数据				in_size：被加密数据长度
 @		key：秘钥						key_size：秘钥长度
 @		out_data：输出Buffer             out_buf_size：输出Buffer长度
 @返回：	输出数据长度
 */
unsigned long Algorithm::encrypt_des(
    unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size)
{
    unsigned long out_size = 0;
    unsigned long loop = 0;
    unsigned long rem = 0;
    unsigned long idx = 0;
    unsigned char local_out_data[8] = { 0 };
    unsigned char local_in_data[8] = { 0 };

    if ((in_data == NULL) || (in_size == 0) ||
        (key == NULL) || (key_size != 8) ||
        (out_data == NULL) || (out_buf_size == 0)) {
        return 0;
    }

    loop = in_size / 8;
    rem = in_size % 8;
    if (rem > 0) {
        loop++;
    }
    for (idx = 0; idx < loop; idx++) {
        memset(local_out_data, 0x00, sizeof(local_out_data));
        memset(local_in_data, 0x00, sizeof(local_in_data));

        if (in_size < 8) {
            memcpy(local_in_data, in_data + idx * 8, in_size);
        } else {
            memcpy(local_in_data, in_data + idx * 8, 8);
            in_size -= 8;
        }

        g_des.crypt_des(local_in_data, 8, key, 8, ENCRYPTION, local_out_data, 8);
        out_size += 8;
        if (out_size > out_buf_size) {
            memcpy(out_data + idx * 8, local_out_data, (out_buf_size - idx * 8));
            out_size = out_buf_size;
            return out_size;
        }
        memcpy(out_data + idx * 8, local_out_data, 8);
    }

    return out_size;
}

/**
 @功能：	单DES解密
 @参数：	in_data：被解密数据				in_size：被解密数据长度
 @		key：秘钥						key_size：秘钥长度
 @		out_data：输出Buffer             out_buf_size：输出Buffer长度
 @返回：	输出数据长度
 */
unsigned long Algorithm::decrypt_des(
    unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size)
{
    unsigned long out_size = 0;
    unsigned long loop = 0;
    unsigned long rem = 0;
    unsigned long idx = 0;
    unsigned char local_out_data[8] = { 0 };
    unsigned char local_in_data[8] = { 0 };

    if ((in_data == NULL) || (in_size == 0) ||
        (key == NULL) || (key_size != 8) ||
        (out_data == NULL) || (out_buf_size == 0)) {
        return 0;
    }

    loop = in_size / 8;
    rem = in_size % 8;
    if (rem > 0) {
        loop++;
    }
    for (idx = 0; idx < loop; idx++)
    {
        memset(local_out_data, 0x00, sizeof(local_out_data));
        memset(local_in_data, 0x00, sizeof(local_in_data));

        if (in_size < 8) {
            memcpy(local_in_data, in_data + idx * 8, in_size);
        } else {
            memcpy(local_in_data, in_data + idx * 8, 8);
            in_size -= 8;
        }

        g_des.crypt_des(local_in_data, 8, key, 8, DECRYPTION, local_out_data, 8);
        out_size += 8;
        if (out_size > out_buf_size) {
            memcpy(out_data + idx * 8, local_out_data, (out_size - idx * 8));
            out_size = out_buf_size;
            return out_size;
        }
        memcpy(out_data + idx * 8, local_out_data, 8);
    }

    return out_size;
}

/**
 @功能：	3DES加密     1、左密钥加密   2、 右密钥解密  3、 左密钥加密
 @参数：	in_data：被加密数据				in_size：被加密数据长度
 @		key：秘钥						key_size：秘钥长度
 @		out_data：输出Buffer             out_buf_size：输出Buffer长度
 @返回：	输出数据长度
 */
unsigned long Algorithm::encrypt_3des(
    unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size)
{
    unsigned long out_size = 0;
    unsigned long loop = 0;
    unsigned long rem = 0;
    unsigned long idx = 0;
    unsigned char local_out_data[8] = { 0 };
    unsigned char local_in_data[8] = { 0 };
    unsigned char working_buffer[8] = { 0 };
    unsigned char tmp_key[24] = { 0 };

    if ((in_data == NULL) || (in_size == 0) ||
        (key == NULL) || (out_data == NULL) ||
        (out_buf_size == 0)) {
        return 0;
    }

    switch (key_size) {
    case 8:
        memcpy(tmp_key, key, key_size);
        memcpy(tmp_key + 8, key, 8);
        memcpy(tmp_key + 16, key, 8);
        break;
    case 16:
        memcpy(tmp_key, key, key_size);
        memcpy(tmp_key + 16, key, 8);
        break;
    case 24:
        memcpy(tmp_key, key, key_size);
        break;
    default:
        return 0;
        break;
    }

    loop = in_size / 8;
    rem = in_size % 8;
    if (rem > 0) {
        loop++;
    }
    for (idx = 0; idx < loop; idx++) {
        memset(local_out_data, 0x00, sizeof(local_out_data));
        memset(local_in_data, 0x00, sizeof(local_in_data));
        memset(working_buffer, 0x00, sizeof(working_buffer));

        if (in_size < 8) {
            memcpy(local_in_data, in_data + idx * 8, in_size);
        } else {
            memcpy(local_in_data, in_data + idx * 8, 8);
            in_size -= 8;
        }
        g_des.crypt_des(local_in_data, 8, tmp_key, 8, ENCRYPTION, local_out_data, 8);
        g_des.crypt_des(local_out_data, 8, tmp_key + 8, 8, DECRYPTION, working_buffer, 8);
        g_des.crypt_des(working_buffer, 8, tmp_key + 16, 8, ENCRYPTION, local_out_data, 8);
        out_size += 8;
        if (out_size > out_buf_size) {
            memcpy(out_data + idx * 8, local_out_data, (out_buf_size - idx * 8));
            out_size = out_buf_size;
            return out_size;
        }
        memcpy(out_data + idx * 8, local_out_data, 8);
    }

    return out_size;
}

/**
 @功能：	3DES解密     1、左密钥解密   2、 右密钥加密  3、 左密钥解密
 @参数：	in_data：被解密数据				in_size：被解密数据长度
 @		key：秘钥						key_size：秘钥长度
 @		out_data：输出Buffer             out_buf_size：输出Buffer长度
 @返回：	输出数据长度
 */
unsigned long Algorithm::decrypt_3des(
    unsigned char* in_data, unsigned long in_size, unsigned char* key, int key_size, unsigned char* out_data, unsigned long out_buf_size)
{
    unsigned long out_size = 0;
    unsigned long loop = 0;
    unsigned long rem = 0;
    unsigned long idx = 0;
    unsigned char local_out_data[8] = { 0 };
    unsigned char local_in_data[8] = { 0 };
    unsigned char working_buffer[8] = { 0 };
    unsigned char tmp_key[24] = { 0 };

    if ((in_data == NULL) || (in_size == 0) ||
        (key == NULL) || (out_data == NULL) ||
        (out_buf_size == 0)) {
        return 0;
    }

    switch (key_size) {
    case 8:
        memcpy(tmp_key, key, key_size);
        memcpy(tmp_key + 8, key, 8);
        memcpy(tmp_key + 16, key, 8);
        break;
    case 16:
        memcpy(tmp_key, key, key_size);
        memcpy(tmp_key + 16, key, 8);
        break;
    case 24:
        memcpy(tmp_key, key, key_size);
        break;
    default:
        return 0;
        break;
    }

    loop = in_size / 8;
    rem = in_size % 8;
    if (rem > 0) {
        loop++;
    }
    for (idx = 0; idx < loop; idx++) {
        memset(local_out_data, 0x00, sizeof(local_out_data));
        memset(local_in_data, 0x00, sizeof(local_in_data));
        memset(working_buffer, 0x00, sizeof(working_buffer));

        if (in_size < 8) {
            memcpy(local_in_data, in_data + idx * 8, in_size);
        } else {
            memcpy(local_in_data, in_data + idx * 8, 8);
            in_size -= 8;
        }
        g_des.crypt_des(local_in_data, 8, tmp_key + 16, 8, DECRYPTION, local_out_data, 8);
        g_des.crypt_des(local_out_data, 8, tmp_key + 8, 8, ENCRYPTION, working_buffer, 8);
        g_des.crypt_des(working_buffer, 8, tmp_key, 8, DECRYPTION, local_out_data, 8);
        out_size += 8;
        if (out_size > out_buf_size) {
            memcpy(out_data + idx * 8, local_out_data, (out_buf_size - idx * 8));
            out_size = out_buf_size;
            return out_size;
        }
        memcpy(out_data + idx * 8, local_out_data, 8);
    }

    return out_size;
}

/**
 @功能：	生成指定个数的指定范围内的随机整数
 @参数：	min_data：随机数最小值					max_data：随机数最大值
 @		random_list：生成的随机整数			count：指定随机整数个数
 @		must_different：随机整数是否互不相同	sort：随机整数是否排序
 @返回：	true：成功		false：失败
 */
bool Algorithm::get_random_numbers(int min_data, int max_data, int random_list[], int count,
    bool must_different/* = false*/, bool sort/* = false*/)
{
    list<int> lst;
    list<int>::iterator it;
    int number = 0;
    int idx = 0;

    if ((random_list == NULL) || (count == 0) ||
        (min_data > max_data) || (max_data > RAND_MAX)) {
        return false;
    }
    if ((must_different) && (max_data - min_data + 1 < count)) {
        return false;
    }

    srand((unsigned int)time(0));
    do {
        number = (rand() % (max_data - min_data + 1)) + min_data;
        if ((must_different == false) ||
            (lst.size() == 0) ||
            (find(lst.begin(), lst.end(), number) == lst.end())) {
            lst.push_back(number);
        }
    } while (lst.size() < (size_t)count);

    if (sort) {
        lst.sort();
    }

    for (idx = 0, it = lst.begin(); it != lst.end(); idx++, it++) {
        random_list[idx] = *it;
    }

    return true;
}
