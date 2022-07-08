#include "framework.h"
#include "crypt_des.h"


unsigned char CryptDes::parity_tbl[256] =
{
	0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x07, 0x07,
	0x08, 0x08, 0x0b, 0x0b, 0x0d, 0x0d, 0x0e, 0x0e,
	0x10, 0x10, 0x13, 0x13, 0x15, 0x15, 0x16, 0x16,
	0x19, 0x19, 0x1a, 0x1a, 0x1c, 0x1c, 0x1f, 0x1f,
	0x20, 0x20, 0x23, 0x23, 0x25, 0x25, 0x26, 0x26,
	0x29, 0x29, 0x2a, 0x2a, 0x2c, 0x2c, 0x2f, 0x2f,
	0x31, 0x31, 0x32, 0x32, 0x34, 0x34, 0x37, 0x37,
	0x38, 0x38, 0x3b, 0x3b, 0x3d, 0x3d, 0x3e, 0x3e,
	0x40, 0x40, 0x43, 0x43, 0x45, 0x45, 0x46, 0x46,
	0x49, 0x49, 0x4a, 0x4a, 0x4c, 0x4c, 0x4f, 0x4f,
	0x51, 0x51, 0x52, 0x52, 0x54, 0x54, 0x57, 0x57,
	0x58, 0x58, 0x5b, 0x5b, 0x5d, 0x5d, 0x5e, 0x5e,
	0x61, 0x61, 0x62, 0x62, 0x64, 0x64, 0x67, 0x67,
	0x68, 0x68, 0x6b, 0x6b, 0x6d, 0x6d, 0x6e, 0x6e,
	0x70, 0x70, 0x73, 0x73, 0x75, 0x75, 0x76, 0x76,
	0x79, 0x79, 0x7a, 0x7a, 0x7c, 0x7c, 0x7f, 0x7f,
	0x80, 0x80, 0x83, 0x83, 0x85, 0x85, 0x86, 0x86,
	0x89, 0x89, 0x8a, 0x8a, 0x8c, 0x8c, 0x8f, 0x8f,
	0x91, 0x91, 0x92, 0x92, 0x94, 0x94, 0x97, 0x97,
	0x98, 0x98, 0x9b, 0x9b, 0x9d, 0x9d, 0x9e, 0x9e,
	0xa1, 0xa1, 0xa2, 0xa2, 0xa4, 0xa4, 0xa7, 0xa7,
	0xa8, 0xa8, 0xab, 0xab, 0xad, 0xad, 0xae, 0xae,
	0xb0, 0xb0, 0xb3, 0xb3, 0xb5, 0xb5, 0xb6, 0xb6,
	0xb9, 0xb9, 0xba, 0xba, 0xbc, 0xbc, 0xbf, 0xbf,
	0xc1, 0xc1, 0xc2, 0xc2, 0xc4, 0xc4, 0xc7, 0xc7,
	0xc8, 0xc8, 0xcb, 0xcb, 0xcd, 0xcd, 0xce, 0xce,
	0xd0, 0xd0, 0xd3, 0xd3, 0xd5, 0xd5, 0xd6, 0xd6,
	0xd9, 0xd9, 0xda, 0xda, 0xdc, 0xdc, 0xdf, 0xdf,
	0xe0, 0xe0, 0xe3, 0xe3, 0xe5, 0xe5, 0xe6, 0xe6,
	0xe9, 0xe9, 0xea, 0xea, 0xec, 0xec, 0xef, 0xef,
	0xf1, 0xf1, 0xf2, 0xf2, 0xf4, 0xf4, 0xf7, 0xf7,
	0xf8, 0xf8, 0xfb, 0xfb, 0xfd, 0xfd, 0xfe, 0xfe 
};
unsigned char CryptDes::h_bit[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
unsigned char CryptDes::ip_tbl[64] =
{
	58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6,
	64, 56, 48, 40, 32, 24, 16, 8,
	57, 49, 41, 33, 25, 17,  9, 1,
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5,
	63, 55, 47, 39, 31, 23, 15, 7
};
unsigned char CryptDes::e_tbl[48] =
{
	32,  1,  2,  3,  4,  5,
	4,  5,  6,  7,  8,  9,
	8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1 
};
unsigned char CryptDes::pc1_tbl[56] =
{
	57, 49, 41, 33, 25, 17,  9,  1,
	58, 50, 42, 34, 26, 18, 10,  2,
	59, 51, 43, 35, 27, 19, 11,  3,
	60, 52, 44, 36, 63, 55, 47, 39,
	31, 23, 15,  7, 62, 54, 46, 38,
	30, 22, 14,  6, 61, 53, 45, 37,
	29, 21, 13,  5, 28, 20, 12,  4 
};
unsigned char CryptDes::ls_tbl[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};
unsigned char CryptDes::pc2_tbl[48] =
{
	14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32 
};
unsigned char CryptDes::s_tbl[8][64] = {
    {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13},

    {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9},

    {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
    1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12},

    {7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
    3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14},

    {2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3},

    {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
    4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13},

    {4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
    6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12},

    {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
    2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
};
unsigned char CryptDes::p_tbl[32] =
{
	16,  7, 20, 21, 29, 12, 28, 17,
	1, 15, 23, 26,  5, 18, 31, 10,
	2,  8, 24, 14, 32, 27,  3,  9,
	19, 13, 30,  6, 22, 11,  4, 25 
};
unsigned char CryptDes::ip1_tbl[64] = {
	40, 8, 48, 16, 56, 24, 64, 32,
	39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30,
	37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28,
	35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26,
	33, 1, 41,  9, 49, 17, 57, 25 
};


void CryptDes::p_tbl_func()
{
	int i = 0;

	for (i = 0; i < 4; i++) {
		p_convert[loop][i] = 0;
	}
	for (i = 0; i < 32; i++) {
		id = p_tbl[i];
        by1 = (unsigned char)((id - 1) / 8);
        bb1 = (unsigned char)((id - 1) % 8);
		s_bit = p_data[by1] & h_bit[bb1];

		if (s_bit != 0) {
            by2 = (unsigned char)( i / 8 );
            bb2 = (unsigned char)( i % 8 );
			p_convert[loop][by2] |= h_bit[bb2];
		}
	}
	for (i = 0; i < 4; i++) {
		r0_data[loop + 1][i] = 0;
	}
	for (i = 0; i < 4; i++) {
		r0_data[loop + 1][i] = l0_data[loop][i] ^ p_convert[loop][i];
	}	
	for (i = 0; i < 4; i++) {
		l0_data[loop + 1][i] = r0_data[loop][i];
	}	
}

void CryptDes::s_tbl_func()
{
	int i = 0;
	int pnt = 0;

	for (i = 0; i < 6; i++) {
		s_box[i] = 0;
	}

	if (p == 0) {
		pnt = loop;
	} else {
		pnt = 15 - loop;
	}
	for (i = 0; i < 6; i++) {
		s_box[i] = r0_convert[loop][i] ^ k1_convert[pnt][i];
	}

	for (i = 0; i < 4; i++) {
		p_data[i] = 0;
	}

	/**** 0 ****/
	if ((s_box[0] & 0x80) == 0) {
		if ((s_box[0] & 0x04) == 0) { 
			g_no = 0;
		} else {
			g_no = 1;
		}
	} else {
		if ((s_box[0] & 0x04) == 0) { 
			g_no = 2;
		} else {
			g_no = 3;
		}
	}
	r_no = (s_box[0] & 0x78) >> 3;
	s_dt = (s_tbl[0][g_no * 16 + r_no]) << 4;
	p_data[0] |= s_dt & 0xf0;
	/**** 1 ****/
	if ((s_box[0] & 0x02) == 0) {
		if ((s_box[1] & 0x10) == 0) {
			g_no = 0; 
		} else {
			g_no = 1; 
		}
	} else {
		if ((s_box[1] & 0x10) == 0) {
			g_no = 2; 
		} else {
			g_no = 3;
		}
	}
	r_no  = (s_box[0] & 0x01) << 3;
	r_no |= (s_box[1] & 0xe0) >> 5;
	s_dt = (s_tbl[1][g_no * 16 + r_no]) & 0x0f;
	p_data[0] |= s_dt;
	/**** 2 ****/
	if ((s_box[1] & 0x08) == 0) {
		if ((s_box[2] & 0x40) == 0) { 
			g_no = 0; 
		} else {
			g_no = 1;
		}
	} else {
		if ((s_box[2] & 0x40) == 0) {
			g_no = 2; 
		} else {
			g_no = 3; 
		}
	}
	r_no  = (s_box[1] & 0x07) << 1;
	r_no |= (s_box[2] & 0x80) >> 7;
	s_dt = (s_tbl[2][g_no * 16 + r_no]) << 4;
	p_data[1] |= s_dt & 0xf0;
	/**** 3 ****/
	if ((s_box[2] & 0x20) == 0) {
		if ((s_box[2] & 0x01) == 0) {
			g_no = 0;
		} else {
			g_no = 1;
		}
	} else {
		if ((s_box[2] & 0x01) == 0) {
			g_no = 2; 
		} else {
			g_no = 3;
		}
	}
	r_no = (s_box[2] & 0x1e) >> 1;
	s_dt = (s_tbl[3][g_no * 16 + r_no]) & 0x0f;
	p_data[1] |= s_dt;

	/**** 4 ****/
	if ((s_box[3] & 0x80) == 0) {
		if ((s_box[3] & 0x04) == 0) {
			g_no = 0; 
		} else {
			g_no = 1; 
		}
	} else {
		if ((s_box[3] & 0x04) == 0) {
			g_no = 2;
		} else {
			g_no = 3;
		}
	}
	r_no = (s_box[3] & 0x78) >> 3;
	s_dt = (s_tbl[4][g_no * 16 + r_no]) << 4;
	p_data[2] |= s_dt & 0xf0;
	/**** 5 ****/
	if ((s_box[3] & 0x02) == 0) {
		if ((s_box[4] & 0x10) == 0) {
			g_no = 0;
		} else {
			g_no = 1;
		}
	} else {
		if ((s_box[4] & 0x10) == 0) { 
			g_no = 2;
		} else {
			g_no = 3; 
		}
	}
	r_no  = (s_box[3] & 0x01) << 3;
	r_no |= (s_box[4] & 0xe0) >> 5;
	s_dt = (s_tbl[5][g_no * 16 + r_no]) & 0x0f;
	p_data[2] |= s_dt;
	/**** 6 ****/
	if ((s_box[4] & 0x08) == 0) {
		if ((s_box[5] & 0x40) == 0) { 
			g_no = 0;
		} else {
			g_no = 1;
		}
	} else {
		if ((s_box[5] & 0x40) == 0) {
			g_no = 2;
		} else {
			g_no = 3;
		}
	}
	r_no  = (s_box[4] & 0x07) << 1;
	r_no |= (s_box[5] & 0x80) >> 7;
	s_dt = (s_tbl[6][g_no * 16 + r_no]) << 4;
	p_data[3] |= s_dt & 0xf0;
	/**** 7 ****/
	if ((s_box[5] & 0x20) == 0) {
		if ((s_box[5] & 0x01) == 0) {
			g_no = 0; 
		} else {
			g_no = 1; 
		}
	} else {
		if ((s_box[5] & 0x01) == 0) { 
			g_no = 2;
		} else {
			g_no = 3; 
		}
	}
	r_no = (s_box[5] & 0x1e) >> 1;
	s_dt = (s_tbl[7][g_no * 16 + r_no]) & 0x0f;
	p_data[3] |= s_dt;
}

void CryptDes::pc2_tbl_func()
{
	int i = 0;

	for (i = 0; i < 6; i++) {
		k1_convert[loop][i] = 0;
	}

	for (i = 0; i < 48; i++) {
		id = pc2_tbl[i];
		if (id < 29) {
            by1 = (unsigned char)((id - 1) / 8);
            bb1 = (unsigned char)((id - 1) % 8);
			s_bit = c0_convert[loop + 1][by1] & h_bit[bb1];
			if (s_bit != 0) {
                by2 = (unsigned char)( i / 8 );
                bb2 = (unsigned char)( i % 8 );
				k1_convert[loop][by2] |= h_bit[bb2];
			}
		} else {
            by1 = (unsigned char)((id - 29) / 8);
            bb1 = (unsigned char)((id - 29) % 8);
			s_bit = d0_convert[loop + 1][by1] & h_bit[bb1];
			if (s_bit != 0) {
                by2 = (unsigned char)( i / 8 );
                bb2 = (unsigned char)( i % 8 );
				k1_convert[loop][by2] |= h_bit[bb2];
			}
		}
	}
}

void CryptDes::ls_tbl_func()
{
	int i = 0;

	ls = ls_tbl[loop];
	for (i = 0; i < 4; i++) {
		c0_convert[loop + 1][i] = c0_convert[loop][i] << ls;
		d0_convert[loop + 1][i] = d0_convert[loop][i] << ls;
	}
	for (i = 0; i < 3; i++) {
		c0_convert[loop + 1][i] |= c0_convert[loop][i + 1] >> (8 - ls);
		d0_convert[loop + 1][i] |= d0_convert[loop][i + 1] >> (8 - ls);
	}
	c0_convert[loop + 1][3] |= (c0_convert[loop][0] >> (4 - ls)) & 0xf0;
	d0_convert[loop + 1][3] |= (d0_convert[loop][0] >> (4 - ls)) & 0xf0;
}

void CryptDes::pc1_tbl_func()
{
	int i = 0;

	for (i = 0; i < 7; i++) {
		pc1_data[i] = 0;
	}

	for (i = 0; i < 56; i++) {
		id  = pc1_tbl[i];
        by1 = (unsigned char)((id - 1) / 8);
        bb1 = (unsigned char)((id - 1) % 8);
		s_bit = key_dt[by1] & h_bit[bb1];

		if (s_bit != 0) {
            by2 = (unsigned char)(i / 8);
            bb2 = (unsigned char)(i % 8);
			pc1_data[by2] |= h_bit[bb2];
		}
	}
	for (i = 0; i < 4; i++) {
		c0_convert[0][i] = pc1_data[i];
	}

	c0_convert[0][3] &= 0xf0;

	for (i = 0; i < 3; i++) {
		d0_convert[0][i]  = pc1_data[i + 3] << 4;
		d0_convert[0][i] |= pc1_data[i + 4] >> 4;
	}
	d0_convert[0][3]  = pc1_data[3 + 3] << 4;
}

void CryptDes::e_tbl_func()
{
	int i = 0;

	for (i = 0; i < 6; i++) {
		r0_convert[loop][i] = 0;
	}	
	for (i = 0; i < 48; i++) {
		id  = e_tbl[i];
        by1 = (unsigned char)((id - 1) / 8);
        bb1 = (unsigned char)((id - 1) % 8);
		s_bit = r0_data[loop][by1] & h_bit[bb1];

		if (s_bit != 0)
		{
            by2 = (unsigned char)( i / 8 );
            bb2 = (unsigned char)( i % 8 );
			r0_convert[loop][by2] |= h_bit[bb2];
		}
	}
}

void CryptDes::ip_tbl_func()
{
	int i = 0;

	for (i = 0; i < 8; i++) {
		ip_data[i] = 0;
	}

	for (i = 0; i < 64; i++) {
		id  = ip_tbl[i];
        by1 = (unsigned char)((id - 1) / 8);
        bb1 = (unsigned char)((id - 1) % 8);
		s_bit = text[by1] & h_bit[bb1];

		if (s_bit != 0) {
            by2 = (unsigned char)(i / 8);
            bb2 = (unsigned char)(i % 8);
			ip_data[by2] |= h_bit[bb2];
		}
	}
	for (i = 0; i < 4; i++) {
		l0_data[0][i] = ip_data[i];
		r0_data[0][i] = ip_data[i + 4];
	}
}

void CryptDes::des()
{
	int i = 0;

	ip_tbl_func();
	pc1_tbl_func();

	for (i = 0; i < 4; i++) {
		p_data[i] = 0;
	}

	for (loop = 0; loop < 16; loop++) {
		ls_tbl_func();
		pc2_tbl_func();
	}
	for (loop = 0; loop < 16; loop++) {
		e_tbl_func();
		s_tbl_func();
		p_tbl_func();
	}
	for (i = 0; i < 8; i++) {
		output_data[i] = 0;
	}
	for (i = 0; i < 64; i++) {
		id = ip1_tbl[i];
		if (id < 33) {
            by1 = (unsigned char)((id - 1) / 8);
            bb1 = (unsigned char)((id - 1) % 8);
			s_bit = r0_data[16][by1] & h_bit[bb1];
            by2 = (unsigned char)( i / 8 );
            bb2 = (unsigned char)( i % 8 );
			if (s_bit != 0) {
				output_data[by2] |= h_bit[bb2];
			}

		} else {
            by1 = (unsigned char)((id - 33) / 8);
            bb1 = (unsigned char)((id - 33) % 8);
			s_bit = l0_data[16][by1] & h_bit[bb1];
            by2 = (unsigned char)( i / 8 );
            bb2 = (unsigned char)( i % 8 );
			if (s_bit != 0) {
				output_data[by2] |= h_bit[bb2];
			}
		}
	}
}

void CryptDes::key_func(unsigned char* key_data)
{
	if (key_data[0] != 0) {
		for (int i = 0; i < 8; i++) {
			key_dt[i] = parity_tbl[(char2bin(key_data[i * 2]) << 4) + char2bin(key_data[i * 2 + 1])];
		}
	}
}

void CryptDes::data_func(unsigned char* in_data)
{
	if (in_data[0] != 0) {
		for (int i = 0; i < 8; i++) {
			text[i]  = (char2bin(in_data[i * 2]) << 4) + char2bin(in_data[i * 2 + 1]);
		}
	}
}

unsigned char CryptDes::char2bin(unsigned char c)
{
	unsigned char ret = c & 0x0f;

	if (c > '9' || c < '0') {
		return ret + 9;
	}

	return ret;
}

bool CryptDes::crypt_des(unsigned char* in_data, unsigned long in_size, unsigned char* key, unsigned long key_size,
	unsigned long flag, unsigned char* out_data, unsigned long out_size)
{
	unsigned long idx = 0;
	unsigned char local_in_data[16];
	unsigned char key_data[16];
	unsigned char work;

    if ((in_size == 0) ||
		(key == NULL) || (key_size == 0) ||
		(out_data == NULL) || (out_size == 0)) {
		return false;
	}

	memset(local_in_data, 0x00, sizeof(local_in_data));
	memset(key_data, 0x00, sizeof(key_data));

	for (idx = 0; idx < in_size; idx++) {
		work = in_data[idx] >> 4;
        if (work <= 0x09) {
			local_in_data[idx * 2] = work + 0x30;
		} else {
			local_in_data[idx * 2] = work + 0x57;
		}

		work = in_data[idx] & 0x0f;
        if (work <= 0x09) {
			local_in_data[idx * 2 + 1] = work + 0x30;
		} else {
			local_in_data[idx * 2 + 1] = work + 0x57;
		}
	}

	for (idx = 0; idx < key_size; idx++) {
		work = key[idx] >> 4;
        if (work <= 0x09) {
			key_data[idx * 2] = work + 0x30;
		} else {
			key_data[idx * 2] = work + 0x57;
		}

		work = key[idx] & 0x0f ;
        if (work <= 0x09) {
			key_data[idx * 2 + 1] = work + 0x30;
		} else {
			key_data[idx * 2 + 1] = work + 0x57;
		}
	}

	for (idx = 0; idx < 16; idx++) {
		text[idx] = 0;
	}
	data_func(local_in_data);//压缩后的cdk又经解压缩16
	key_func(key_data);//初始cdk又经解压缩16

	if (flag == ENCRYPTION)
	{
		p = 0;    // Flag 加密
		des();
	}
	if (flag == DECRYPTION)
	{
		p = 1;   // Flag 解密
		des();
	}
	memcpy(out_data, output_data, out_size);

	return true;
}
