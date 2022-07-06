#ifndef CRYPT_DES_H
#define CRYPT_DES_H


#define     ENCRYPTION                    0		//加密
#define     DECRYPTION                    1		//解密


class CryptDes
{
public:
	bool crypt_des(unsigned char* in_data, unsigned long in_size, unsigned char* key, unsigned long key_size,
		unsigned long flag, unsigned char* out_data, unsigned long out_size);

private:
	unsigned char char2bin(unsigned char c);
	void data_func(unsigned char* in_data);
	void key_func(unsigned char* key_data);
	void des();
	void ip_tbl_func();
	void e_tbl_func();
	void pc1_tbl_func();
	void ls_tbl_func();
	void pc2_tbl_func();
	void s_tbl_func();
	void p_tbl_func();


private:
	int loop, id;
	unsigned char by1, bb1, by2, bb2, s_bit;
	unsigned char text[16], key_dt[16];
	unsigned char ip_data[8],
		l0_data[17][4],
		r0_data[17][4],
		r0_convert[16][6],
		pc1_data[7],
		c0_convert[17][4],
		d0_convert[17][4],
		ls,
		k1_convert[16][6],
		s_box[6],
		s_dt,
		g_no,
		r_no,
		p_data[4],
		p_convert[16][4],
		output_data[8],
		p;

private:
	static unsigned char parity_tbl[256];
	static unsigned char h_bit[8];
	static unsigned char ip_tbl[64];
	static unsigned char e_tbl[48];
	static unsigned char pc1_tbl[56];
	static unsigned char ls_tbl[16];
	static unsigned char pc2_tbl[48];
	static unsigned char s_tbl[8][64];
	static unsigned char p_tbl[32];
	static unsigned char ip1_tbl[64];
};


#endif // CRYPT_DES_H
