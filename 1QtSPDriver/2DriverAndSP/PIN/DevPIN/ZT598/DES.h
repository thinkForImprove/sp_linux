#ifndef DES_H
#define DES_H

#define     ENCRYPT     0
#define     DESCRYPT    1
#define     PINLEN      9
//////////////////////////////////////////////////////////////////////////
class CCDES
{
public:
    CCDES();
    virtual ~CCDES();

    // DES ADD
    void AscToBcd(unsigned char *charbcd, unsigned char *charasc, int len);
    void BcdToAsc(unsigned char *charasc, unsigned char *charbcd, int len);

    void key_encrypt_Asc(unsigned char *input, unsigned char *key);
    void key_encrypt_Bcd(unsigned char *input, unsigned char *key);

    void key_uncrypt_Asc(unsigned char *input, unsigned char *key);
    void key_uncrypt_Bcd(unsigned char *input, unsigned char *key);

    void pan_encrypt_Bcd(unsigned char *input, unsigned char *pan, unsigned char *key);
    void pan_encrypt_Asc(unsigned char *input, unsigned char *pan, unsigned char *key);
    void pan_uncrypt_Bcd(unsigned char *input, unsigned char *pan, unsigned char *key);
    void pan_uncrypt_Asc(unsigned char *input, unsigned  char *pan, unsigned  char *key);

    //DES ADD
public:
    long  gUndes(char *DataInput, char *key, char *DataOutput);
    long  gDes(char *DataInput, char *key, char *DataOutput);
    //long  StringToMAC(unsigned char * string, unsigned char * MacKey, unsigned char * MAC);

public:
    unsigned char cdesoutput[9];
    unsigned char cascoutput[17];
protected:
    unsigned char cdeskey[9];
    unsigned char cdesinput[9];

    unsigned char   KS[16][48];
    unsigned char   E[64];

    void setkey(unsigned char *key);
    void encrypt(unsigned char *block, int edflag);
    void expand(unsigned char *in, unsigned char *out);
    void compress(unsigned char *in, unsigned char *out);

};

#endif
