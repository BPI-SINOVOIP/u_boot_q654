#include <common.h>
#include <image.h>
#include "secure_verify.h"
#include "../ed25519/ed25519.h"
#include "sp_otp.h"

#define RSATEST   0  
#ifdef RSATEST
#include <cpu_func.h>
#include "crypto_drv.h"      
#endif


#define VERIFY_SIGN_MAGIC_DATA	(0x7369676E)

/***********************************
|---------------------------|
|        kernel data        |
|---------------------------|
|---------------------------|
|   sig_flag_data(8byte)    |
|---------------------------|
|---------------------------|
|      sig data(64byte)     |
|---------------------------|
***********************************/

#if (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
	(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
static volatile struct hb_gp_regs *otp_data = (volatile struct hb_gp_regs *)(HB_GP_REG);
#elif defined(CONFIG_TARGET_PENTAGRAM_Q645) || defined(CONFIG_TARGET_PENTAGRAM_SP7350)
static volatile struct hb_gp_regs *otp_data = (volatile struct hb_gp_regs *)(KEY_HB_GP_REG);
#endif


static u32 read_sb_flag(void)
{
	return ((otp_data->hb_gpio_rgst_bus32[0] >> 13) & 0x1); /* G350.0[13] */
}

void prn_dump_buffer(unsigned char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (i && !(i & 0xf)) {
			printf(" \n");
		}
		printf("0x%x ",buf[i]);
	}
	puts(" \n");
}

static void load_otp_pub_key(u8 in_pub[])
{
	int i;
	for (i = 0; i < 32; i++) {
#if (defined(CONFIG_ARCH_PENTAGRAM) && !defined(CONFIG_TARGET_PENTAGRAM_I143_C)) || \
		(defined(CONFIG_TARGET_PENTAGRAM_I143_P) || defined(CONFIG_TARGET_PENTAGRAM_I143_C))
			read_otp_data(HB_GP_REG, SP_OTPRX_REG, i+64,(char *)&in_pub[i]);
#elif defined(CONFIG_TARGET_PENTAGRAM_Q645)
			read_otp_data(KEY_HB_GP_REG, KEY_OTPRX_REG, i+64,(char *)&in_pub[i]);
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
			read_otp_key(OTP_KEY_REG, KEY_OTPRX_REG, i+64,(char *)&in_pub[i]);
#endif
	}
	puts("uboot  OTP pub-key:\n");
	prn_dump_buffer(in_pub,32);
}

// verify_kernel_signature
int do_verify(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int sig_size = 64;
	int sig_flag_size = 8;
	int ret = -1;
	u8 in_pub[32] = {0};
	u8 sig_flag[8] = {0};
	u8 *data=NULL, *sig=NULL;
	int imgsize = 0;
	int tv1=0,tv2=0;
	unsigned int data_size=0;
	image_header_t  *hdr;

	if (argc == 1)
	{
		return CMD_RET_USAGE;
	}

	int kernel_addr =  simple_strtoul(argv[1], NULL, 0);

	hdr = (image_header_t  *)kernel_addr;
	printf("\nkernel_hdr addr = %x\n",(unsigned int)hdr);
	if(hdr == NULL)
		goto out;

	if (!image_check_magic(hdr)) {
		puts("Bad Magic Number\n");
		return (int)NULL;
	}

	if ((read_sb_flag() & 0x01) == 0) {
		puts("\n ******OTP Secure Boot is OFF, return success******\n");
		return 0;
	}

	/* Load public key */
	imgsize = image_get_data_size(hdr);

	/* load signature from image end */
	if (imgsize < sig_size) {
		puts("image size error, too small img\n");
		goto out;
	}
	puts("Verify signature...(Uboot-->Kernel)");

	data = ((u8 *)hdr);
	data_size = imgsize + sizeof(struct image_header);
	sig = data + data_size + sig_flag_size;

	/* Load sign flag data  */
	memcpy(sig_flag,data + data_size,4);// get sig magic data
	u32 sig_magic_data = (sig_flag[0]<<24)|(sig_flag[1]<<16)|(sig_flag[2]<<8)|(sig_flag[3]);
	printf("\n sig_magic_data=0x%x\n",sig_magic_data);

	if(sig_magic_data != VERIFY_SIGN_MAGIC_DATA)
	{
		puts("\n imgdata no sign data \n");
		goto out;
	}
	load_otp_pub_key(in_pub);
	/* verify signature */
#ifdef FOR_ZEBU_CSIM
	tv1 = get_ticks();
#else
	tv1 = get_timer(0);
#endif
	ret = !ed25519_verify(sig, data, data_size, in_pub);
	if (ret) {
		puts("\nsignature verify FAIL!!!!");
		puts("\nsignature:");
		prn_dump_buffer(sig, sig_size);
	} else {
		puts("\nsignature verify OK !!!!");
	}
#ifdef FOR_ZEBU_CSIM
	tv2 = get_ticks();
#else
	tv2 = get_timer(tv1);
#endif
	printf("\n time %dms\n",tv2);

out:
	if(ret)
	{
		printf("veify kernel fail !!");
		while(1);
	}
	return ret;
}

#ifdef RSATEST
#define KEY_ARRAY_SIZE   (256)
unsigned char g_base[2048/8] = {                                                                         
0x8b,0x8c,0x10,0x1b,0xb3,0xe1,0xb3,0x40,0xb0,0x4d,0xa1,0xb8,0xa0,0x6a,0x2c,0x05,
0x25,0xde,0xc2,0xe5,0x5d,0x43,0xb5,0xda,0xea,0x3f,0x73,0xa2,0xea,0x6c,0x4e,0xbf,
0xff,0x70,0xb4,0xef,0x47,0xe5,0xf7,0xb4,0x64,0x71,0x85,0xcc,0x74,0xae,0xb0,0xb9,
0x19,0x42,0xe6,0x39,0x71,0xc7,0x79,0xce,0x1e,0xe3,0xd7,0x36,0x3e,0x30,0x52,0xf3,
0x73,0x54,0x58,0xc3,0xdb,0xe9,0x3b,0x28,0x18,0x95,0x69,0xe0,0x48,0xf2,0x34,0x6d,
0x0d,0xa6,0x0a,0x8d,0x85,0x4b,0x3d,0xc2,0x52,0x87,0x3b,0xca,0x92,0xf4,0x56,0x27,
0xe7,0x38,0xfc,0x97,0x6f,0xed,0x7f,0x9c,0xcc,0xb9,0x4d,0xf4,0x1c,0x36,0xb8,0x21,
0x01,0x0a,0x2e,0xe1,0x99,0xcf,0x01,0xb6,0x86,0x2b,0x9f,0x5e,0xe6,0xb8,0x5a,0x5b,
0x5b,0x1c,0xa0,0x6b,0x03,0xf1,0xc3,0x10,0x80,0xdd,0x31,0x08,0xf0,0x7a,0x3c,0xd5,
0xf5,0x6e,0x52,0x35,0xad,0x53,0xc5,0xaa,0xba,0xcf,0x03,0xf2,0x3a,0x7c,0x5e,0x8f,
0xcf,0x00,0x44,0x3f,0x97,0xf5,0x07,0x84,0x34,0x01,0x15,0x1c,0xc4,0xbe,0xc0,0x89,
0xe9,0xd2,0x76,0x89,0xc1,0xd7,0x89,0x9e,0xee,0x73,0x67,0x86,0x8e,0x40,0x62,0xc3,
0x43,0xe4,0xe8,0x13,0x2b,0xf9,0x4b,0xf8,0xe8,0x25,0xf9,0x30,0x98,0x02,0x44,0x3d,
0xdd,0x36,0x9a,0xdd,0xd5,0x5b,0x4d,0x92,0x22,0x17,0xcb,0x1a,0xe2,0x04,0x66,0xf7,
0xb7,0xc8,0x8c,0xe7,0xbf,0xfd,0x8f,0x6c,0x9c,0x49,0xdd,0x44,0x6c,0x46,0xc8,0xf1,
0xd1,0x9a,0xbe,0x31,0xe9,0xdf,0x11,0x86,0x56,0xbb,0x2f,0xae,0x36,0xc8,0x6a,0x2b,
};  
unsigned char key_E_65537[2048/8] = { 0x01, 0x00, 0x01 };
unsigned char key_D_2048[2048/8] = { 
	0x41, 0x2a, 0xc0, 0x34, 0x13, 0x69, 0x05, 0xbe, 0x69, 0xaa, 0x41, 0xb7, 0x27, 0x77, 0x5c, 
	0x68, 0x44, 0x90, 0x1b, 0x6b, 0xc6, 0x28, 0xd7, 0x32, 0x0e, 0x95, 0xee, 0x7c, 0x1a, 0xa8, 
	0x13, 0x8e, 0xb3, 0xb6, 0x47, 0xaf, 0x48, 0xce, 0xd4, 0x03, 0xea, 0xd5, 0xed, 0x21, 0x77, 
	0xb2, 0x39, 0x2c, 0xc6, 0x3b, 0x85, 0xdd, 0xe7, 0xd3, 0x4e, 0x29, 0xe0, 0x0e, 0xdd, 0x52, 
	0x49, 0xd9, 0x2a, 0x92, 0xf3, 0xdd, 0x0d, 0x99, 0x14, 0x86, 0xb8, 0x89, 0x68, 0x71, 0x57, 
	0xc6, 0x47, 0x45, 0x4e, 0xf7, 0xa8, 0xf0, 0x1d, 0xad, 0x4e, 0x01, 0x1a, 0x14, 0xc2, 0x89, 
	0xe4, 0x08, 0xbc, 0xcd, 0x86, 0x6a, 0x6a, 0xc3, 0x24, 0xc5, 0xd5, 0x6a, 0xef, 0x30, 0xac, 
	0x7a, 0xc6, 0xd3, 0x4c, 0x51, 0xd1, 0xe9, 0x2a, 0xbf, 0x79, 0xfa, 0x1d, 0x91, 0x81, 0xb4, 
	0x79, 0x73, 0x39, 0x0f, 0xdd, 0x38, 0xa7, 0xb2, 0x8b, 0x00, 0xe3, 0xc7, 0xdc, 0x47, 0x46, 
	0x0c, 0x27, 0x7d, 0x99, 0xc1, 0x1d, 0xfd, 0x36, 0x16, 0x06, 0xaa, 0x76, 0x5a, 0x5a, 0x42, 
	0x5c, 0xdb, 0x75, 0x4d, 0xe0, 0xe0, 0xe0, 0xc0, 0x09, 0x2f, 0x57, 0xe9, 0x4c, 0x9a, 0xc0, 
	0xcf, 0x24, 0xcb, 0xaf, 0xe4, 0xda, 0x15, 0x3a, 0x74, 0x5f, 0x8e, 0xc2, 0x38, 0xfc, 0x9c, 
	0x2a, 0xd3, 0x74, 0x20, 0xce, 0x28, 0x4e, 0x53, 0xe3, 0xe6, 0xf1, 0xff, 0x34, 0x56, 0x7c, 
	0xf4, 0x45, 0xa2, 0x56, 0xdb, 0xcf, 0x18, 0x45, 0xf3, 0x0c, 0x7d, 0xd0, 0xa6, 0x6c, 0xd9, 
	0xcf, 0x5c, 0xfc, 0xab, 0x87, 0x35, 0x06, 0x40, 0xf4, 0xb8, 0x46, 0x16, 0xee, 0x31, 0x50, 
	0xf5, 0x24, 0xdc, 0xca, 0xf0, 0xec, 0xd4, 0x04, 0x52, 0x7b, 0x3c, 0xb5, 0x40, 0x2c, 0xb3, 
	0x07, 0x6f, 0x7a, 0x82, 0x39, 0x1b, 0x8e, 0xaa, 0x36, 0xfd, 0x39, 0x86, 0xde, 0xef, 0xbc, 
	0x04 };
unsigned char key_N_2048[] = { 
			 0x4b, 0x86, 0xf8, 0x84, 0xfc, 0x68, 0xcc, 0xd2, 0x01, 0x14, 0xc7,
			 0xcb, 0x34, 0x48, 0xf0, 0xce, 0x9e, 0xd9, 0xae, 0x02, 0x19, 0xee,
			 0xe9, 0xaf, 0xb5, 0xcf, 0xcf, 0xba, 0xec, 0x39, 0x40, 0x03, 0xd2,
			 0x61, 0x50, 0x1f, 0x25, 0x04, 0x82, 0x44, 0x98, 0x45, 0xef, 0xed,
			 0xf5, 0xcd, 0x9d, 0x61, 0xa1, 0x30, 0x3e, 0xf6, 0x47, 0x37, 0x27,
			 0x2d, 0x56, 0xed, 0x3a, 0xe0, 0x6a, 0x69, 0x8a, 0xe2, 0x00, 0x06,
			 0x8d, 0x81, 0x07, 0x5e, 0xb0, 0xe9, 0xa1, 0x75, 0xf4, 0x62, 0x2f,
			 0xfe, 0x84, 0xce, 0x29, 0x8f, 0x6a, 0xe2, 0x9b, 0x4a, 0x3a, 0x62,
			 0xfc, 0x58, 0xb9, 0x6c, 0x0d, 0xd0, 0x18, 0x5d, 0x00, 0x13, 0xd4,
			 0x8a, 0x80, 0xda, 0xe1, 0xfa, 0x04, 0x3e, 0xd7, 0x49, 0x17, 0x3d,
			 0xdd, 0x2b, 0xa2, 0x22, 0x67, 0xf1, 0x8b, 0x5c, 0x79, 0x4d, 0x8e,
			 0xc4, 0x21, 0x95, 0x79, 0x7a, 0x8c, 0xfd, 0xaa, 0x21, 0xa8, 0x70,
			 0x6b, 0x67, 0x9a, 0x28, 0xf5, 0xda, 0x60, 0xa3, 0x5b, 0xcb, 0x75,
			 0x10, 0xc6, 0x00, 0xae, 0x06, 0x5a, 0xf5, 0x0d, 0x62, 0x2e, 0x6b,
			 0x29, 0xac, 0x28, 0x9a, 0x1e, 0x65, 0x55, 0xab, 0x52, 0x52, 0x9d,
			 0xe1, 0xa7, 0xbd, 0xc2, 0x39, 0x7a, 0xab, 0x0e, 0xec, 0x5c, 0xe2,
			 0xc5, 0xf2, 0x63, 0x3c, 0x05, 0x34, 0xd8, 0xa9, 0xec, 0xaf, 0xf1,
			 0x8c, 0x14, 0xf7, 0xea, 0x89, 0x86, 0x8f, 0x50, 0x9f, 0xdc, 0x7b,
			 0x21, 0x0c, 0x7f, 0x2a, 0xa4, 0x8f, 0x78, 0xcb, 0x07, 0x15, 0xb7,
			 0x0f, 0xff, 0xd6, 0x61, 0x0c, 0x5e, 0xa8, 0x19, 0x46, 0x7b, 0xb9,
			 0xa2, 0xea, 0x3d, 0xdb, 0xc7, 0x9f, 0x28, 0x86, 0x7e, 0x69, 0x6c,
			 0xb3, 0x21, 0xa6, 0x59, 0x4a, 0x8a, 0x73, 0x4f, 0xab, 0xd6, 0x87,
			 0x10, 0x0b, 0x7c, 0xa1, 0x2a, 0x70, 0xdd, 0xa6, 0xfc, 0x75, 0x09,
			 0x97, 0x87, 0xc5
};

//static unsigned char key_E_65537[] = {
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,
//};
//static unsigned char key_D_2048[] = {
//0x80,0x40,0xe6,0x52,0x94,0x0d,0x71,0x9f,0xda,0xf3,0x89,0x33,0x72,0x6d,0xcd,0x1f,
//0x7c,0xaa,0xe9,0xac,0x70,0xbe,0x97,0xc1,0x60,0x1f,0xba,0xc9,0x49,0x7f,0xa1,0xbd,
//0xa3,0x75,0x0f,0x5c,0x2a,0x5f,0x49,0x4a,0x93,0xea,0x9f,0x54,0xab,0x6f,0xd6,0x92,
//0x04,0x71,0x72,0xc7,0xc4,0x69,0xe6,0xef,0xf0,0xc6,0x02,0xa5,0x8d,0x05,0x21,0x47,
//0x25,0xf2,0x27,0x78,0xc5,0xee,0x74,0xbb,0xdb,0x80,0x25,0xb8,0x9c,0xe2,0x38,0xca,
//0x66,0xfa,0x31,0xd0,0x44,0x29,0x26,0x74,0x92,0x0f,0x5e,0x93,0xec,0x1c,0x7f,0x04,
//0x56,0xe6,0x50,0x85,0xcc,0x25,0x87,0x2e,0x6d,0xc8,0x27,0x89,0xaa,0x5f,0x0c,0xe0,
//0x7d,0xb1,0x28,0xa9,0x7a,0x13,0x4c,0xb2,0x8e,0xc9,0xbc,0x95,0xe7,0xd3,0x79,0x85,
//0xa5,0x73,0xa6,0x47,0x59,0xc5,0xca,0x00,0x13,0x3a,0xd2,0x49,0xbe,0xec,0xc5,0x1b,
//0xdf,0xe8,0xaa,0xb8,0xc6,0xe1,0xb1,0xc1,0x13,0x15,0xf8,0xba,0xce,0x56,0x48,0xce,
//0x72,0x40,0x87,0x2c,0xda,0x90,0xee,0x10,0x38,0x35,0x30,0x5c,0x7b,0x11,0x6b,0xad,
//0x4a,0xe7,0xd3,0x02,0x06,0x83,0x79,0xee,0xd8,0xe5,0xec,0xeb,0xef,0xfb,0xc4,0xcb,
//0x96,0xaf,0x5a,0x47,0x0e,0x52,0x48,0x49,0x85,0x58,0xb5,0x55,0x6b,0x6e,0x40,0xd2,
//0x97,0xcc,0xd9,0x9e,0x7c,0xd3,0x32,0x2d,0x3b,0xc2,0x91,0x16,0xb7,0x63,0xf5,0xde,
//0x32,0xb6,0xbd,0xbe,0xda,0x8b,0xcc,0x8e,0xe2,0xaf,0xa7,0xd7,0x27,0x45,0x10,0xc9,
//0x04,0x7e,0x88,0x67,0xe8,0x78,0xba,0xed,0x08,0xb8,0x45,0x53,0x4f,0xed,0xe5,0xc9,
//};
//static unsigned char key_N_2048[] = {
//0xdf,0x2f,0x39,0xdd,0x09,0x4a,0xd4,0xcf,0x92,0x64,0xab,0xf8,0x2a,0xb2,0x59,0x6c,
//0x3f,0x9d,0x12,0x5e,0x83,0xd9,0xdb,0x1d,0xdc,0x4b,0x25,0x43,0x8f,0xe6,0xf3,0xf1,
//0x6f,0x30,0xf0,0xcd,0x67,0x8e,0x51,0x0d,0x60,0xfc,0xe3,0x70,0x5f,0xd9,0x13,0xe1,
//0xb4,0x26,0x6a,0x02,0x1a,0x0b,0xb3,0x6b,0x22,0x16,0xcb,0x79,0x0d,0xd9,0xe1,0xb9,
//0xd4,0xc3,0x65,0x7f,0xe0,0x82,0x42,0x04,0xef,0xc2,0xc6,0xcd,0xe9,0xd8,0x8b,0x91,
//0xf5,0xf5,0x16,0xaf,0xb3,0x7f,0x46,0x01,0x60,0x68,0x85,0xbe,0x11,0xb0,0x9c,0xf2,
//0x2a,0xa3,0x64,0xda,0xe2,0x2c,0xc3,0x7e,0xcc,0xcc,0x6c,0x41,0x67,0x9e,0x6f,0xfa,
//0x86,0x6c,0xb5,0x2f,0x36,0xa9,0xf1,0x3c,0x22,0x42,0xe5,0x85,0x78,0xab,0xe7,0x68,
//0x75,0x64,0x83,0xc9,0xed,0xe4,0xd9,0xbd,0xd6,0x71,0x67,0x8c,0x20,0xd7,0xbd,0x52,
//0x47,0x8a,0x7f,0x5f,0x7d,0x00,0x8c,0x1c,0x30,0x60,0xe1,0x4e,0x45,0xe1,0x12,0x25,
//0xd0,0x31,0xd4,0x0c,0x6f,0x62,0x5e,0x95,0x95,0x04,0x91,0xce,0xbe,0x60,0x73,0x49,
//0x9d,0x2e,0xa0,0x90,0xc4,0x7f,0xfd,0x0d,0xee,0x3b,0x46,0xda,0x03,0x91,0x71,0x2e,
//0x7d,0xa6,0x9c,0xd4,0xb0,0x9d,0x30,0x9f,0xc3,0x4b,0x4a,0xf3,0x36,0x42,0x5c,0xb2,
//0xb8,0x02,0x30,0xe2,0x94,0x57,0x80,0xb0,0x13,0xc1,0xf6,0xce,0xfe,0x66,0x33,0x65,
//0x07,0x1f,0x45,0x84,0x08,0xcb,0x4f,0x5d,0x5b,0x4a,0x5a,0x0f,0x86,0xc5,0x2e,0x38,
//0x13,0x6c,0xa0,0x18,0xd2,0x98,0xb8,0x53,0xc8,0xc7,0x75,0x81,0x28,0x57,0xe6,0x9f,
//};
static int calc_valid_size(unsigned char *data, int size, int big_endian)
{
	int i = 0;
	int bytes = 0;

	if (!big_endian) {
		for(i=size-1; i>=0 && !data[i]; i--);
		bytes = i+1;
	} else {
		for(i=0; i<size && !data[i]; i++);
		bytes = size-i;
	}
	return bytes;
}
static struct crypto_dev_s sp_crypto_dev = {
	.reg = (volatile struct sp_crypto_reg *)SP_CRYPTO_REG_BASE,
	.irq = SP_CRYPTO_IRQ,
};
struct crypto_dev_s *get_crypto_dev(void)
{
	return &sp_crypto_dev;
}
long long crypto_mont_w(unsigned char *mod)
{
	long long t = 1;
	long long mode;
	int i;

	memcpy(&mode, mod, sizeof(mode));
	for (i = 1; i < 63; i++) {
		t = (t * t * mode);
	}

	return (-t);
}

/*
 *  compare array 'a' with array 'b'.
 *  @param reversed - say true, if they have different endian mode.
 *  @returen        - 0 if equals, -1 if not equals.
 */
int crypto_compare_array(unsigned char *a,unsigned char *b,unsigned int size,unsigned char reverse)
{
	unsigned int i,j;
	unsigned int jstep;

	if (reverse) {
		j = size - 1;
		jstep = -1;
	} else {
		j = 0;
		jstep = 1;
	}
	for (i=0; i<size; i++, j+=jstep) {
		if (a[i] != b[j])
			return -1;
	}
	return 0;
}

/*
 *  copy array 'src' to array 'dst'.
 *  @param reversed - say true, if wanna 'dst' equals to reversed 'src'.
 */
void crypto_copy_array(unsigned char *dst,unsigned char *src,unsigned int size,unsigned char reverse)
{
	unsigned int i,j;
	unsigned int jstep;

	if (reverse) {
		j = size - 1;
		jstep = -1;
	} else {
		j = 0;
		jstep = 1;
	}
	for (i=0; i<size; i++, j+=jstep) {
		dst[i] = src[j];
	}
}

void crypto_rand_array(unsigned char array[], unsigned int size)
{
	unsigned char seed[4];
	unsigned int rtc_val;
	unsigned int i = 0;

	rtc_val = get_timer(0);
	memcpy(seed, &rtc_val, sizeof(seed));

	for (i=0; i<size; i++) {
		array[i] = seed[0] + seed[1] + seed[2] + seed[3];
		seed[0] += i;
		seed[1] = 1 << (i%8);
		seed[2] = seed[0] + seed[2];
		seed[3] = array[i];
	}
}

void crypto_dump_buf(const char *name, void *buf, int size)
{
	unsigned char *p = buf;
	int i = 0;

	printf("%s {\n",name);
	for (i=0; i<size; i++) {
		printf("0x%02x,",p[i]);
		if((i+1)%16 == 0)
			printf("\n");
	}
	printf("};\n");
}

unsigned char N[MAX_RSA_NBYTES*5];
int crypto_init(void)
{
	struct crypto_dev_s *dev = get_crypto_dev();
	volatile struct sp_crypto_reg *reg = dev->reg;
//	trb_ring_t *ring;
//	unsigned int pa;
	int ret = 0;
	
	printf("============SP_CRYPTO_ENGINE===============\n");
	printf("hw version:%x \n",reg->VERSION);
	printf("register addr:%p \n",reg);
	printf("irq number: %d \n", dev->irq);

	/* RSA initialize */
	dev->rsa_n = N;
	ERR_OUT(!dev->rsa_n, fail, "malloc memory for rsa fail");
	memset(dev->rsa_n, 0, MAX_RSA_NBYTES*5);
	dev->rsa_a = dev->rsa_n + MAX_RSA_NBYTES;
	dev->rsa_e = dev->rsa_a + MAX_RSA_NBYTES;
	dev->rsa_x = dev->rsa_e + MAX_RSA_NBYTES;
	dev->rsa_p2 = dev->rsa_x + MAX_RSA_NBYTES;
	reg->RSANPTR = (unsigned int)(dev->rsa_n);
	reg->RSASPTR = (unsigned int)(dev->rsa_a);
	reg->RSAYPTR = (unsigned int)(dev->rsa_e);
	reg->RSADPTR = (unsigned int)(dev->rsa_x);
	reg->RSAP2PTR = (unsigned int)(dev->rsa_p2);
//	printf("reg->RSANPTR:%08x \n",reg->RSANPTR);
//	printf("dev->rsa_n:%s \n",dev->rsa_n);

	/* HASH initialize */
//	ring = trb_ring_new(HASH_RING_SIZE);
//	ERR_OUT(!ring, fail, "");
//	dev->hash_ring = ring;
//	pa = crypto_physical_addr(ring->trb);
//	reg->HASHDMA_CRCR  = pa | AUTODMA_CRCR_FLAGS;
//	reg->HASHDMA_ERBAR = pa;
//	reg->HASHDMA_ERDPR = pa + HASH_RING_SIZE * sizeof(trb_t);
//	reg->HASHDMA_RCSR  = (HASH_RING_SIZE - 1);
//	reg->HASHDMA_RCSR |= AUTODMA_RCSR_ERF;
//	reg->HASHDMA_RCSR |= AUTODMA_RCSR_EN; /* note: it's HW issue that autodma enable must be alone*/
//	CRYPTO_LOGI("HASH_RING === VA:%p PA:%08x\n", ring->trb, pa);
//	CRYPTO_LOGI("HASHDMA_CRCR  : %08x\n", reg->HASHDMA_CRCR);
//	CRYPTO_LOGI("HASHDMA_ERBAR : %08x\n", reg->HASHDMA_ERBAR);
//	CRYPTO_LOGI("HASHDMA_ERDPR : %08x\n", reg->HASHDMA_ERDPR);
//	CRYPTO_LOGI("HASHDMA_RCSR  : %08x\n", reg->HASHDMA_RCSR);

//	ret = crypto_register_irq();
//	ERR_OUT(ret != SPCRYPTO_RET_SUCCESS, fail, "");
//	reg->SECIE = RSA_DMA_IE | AES_TRB_IE | HASH_TRB_IE;
	dev->initialized = 1;
	printf("Crypto HW Init Success!\n");
	return SPCRYPTO_RET_SUCCESS;

//fail:
//	if (dev->rsa_n)
//		crypto_free_uncached(dev->rsa_n);
//	trb_ring_free(dev->aes_ring);
//	trb_ring_free(dev->hash_ring);
	return ret;
}

int crypto_do_expmod(unsigned char *x, unsigned char *a, unsigned char *e,
		unsigned char *n, unsigned int size, unsigned char big_endian)
{
	struct crypto_dev_s *dev = get_crypto_dev();
	volatile struct sp_crypto_reg *reg = dev->reg;
	int ret;
//	unsigned int *adr;

	printf("crypto_do_expmod\n");

	memset(dev->rsa_x, 0, size);
	memset(dev->rsa_a, 0, size);
	memset(dev->rsa_e, 0, size);
	crypto_copy_array(dev->rsa_a, a, size, big_endian);
	crypto_copy_array(dev->rsa_e, e, size, big_endian);

	printf("size=%x\n",size);
	printf("dev->rsa_nbytes=%x\n",dev->rsa_nbytes);
			
	if ( (size != dev->rsa_nbytes)
		|| crypto_compare_array(dev->rsa_n,n,size,big_endian)) {
		long long w;
		memset(dev->rsa_n, 0, size);
		crypto_copy_array(dev->rsa_n, n, size, big_endian);
		dev->rsa_nbytes = size;
		w = crypto_mont_w(dev->rsa_n);
		reg->RSAWPTRL = w & 0xffffffff;
		reg->RSAWPTRH = (w >> 32) & 0xffffffff;
		reg->RSAPAR0 = RSA_SET_PARA_D(size<<3) | RSA_PARA_PRECAL_P2;
	} else {
		reg->RSAPAR0 = RSA_SET_PARA_D(size<<3) | RSA_PARA_FETCH_P2;
	}
	
//			adr=reg->RSANPTR;
//			printf("RSANPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);
//			adr=reg->RSASPTR;
//			printf("RSASPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);
//			adr=reg->RSAYPTR;
//			printf("RSAYPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);
//			adr=reg->RSADPTR;
//			printf("RSADPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);

	flush_dcache_all();
	dev->rsa_done = false;
	//printf("reg->RSADMACS=%x\n ",reg->RSADMACS);         //show register value
	//printf("dev->reg->SECIF=%x\n ",dev->reg->SECIF);
	printf("before rsa done:dev->reg->SECIF=%d,dev->rsa_done=%d \n",dev->reg->SECIF,dev->rsa_done);
	reg->RSADMACS = RSA_DMA_SIZE(size) | RSA_DMA_ENABLE;
	
	//printf("reg->RSADMACS=%x\n ",reg->RSADMACS);         //show register value
	while(!((dev->reg->SECIF) & RSA_DMA_IF)) {
		printf("during rsa:dev->reg->SECIF=%d,dev->rsa_done=%d \n",dev->reg->SECIF,dev->rsa_done);
	}
	if(dev->reg->SECIF == 1)
		dev->rsa_done = true;
		
	printf("rsa done::dev->reg->SECIF=%d,dev->rsa_done=%d \n",dev->reg->SECIF,dev->rsa_done);
	dev->reg->SECIF = 1; //write 1 to clear flag
	//printf("clear interrupt flag:dev->reg->SECIF=%d\n",dev->reg->SECIF);
	reg->RSADMACS = 0;
	//printf("clear reg->RSADMACS=%x\n ",reg->RSADMACS);         //show register value
	
//			adr=reg->RSANPTR;
//			printf("RSANPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);
//			adr=reg->RSASPTR;
//			printf("RSASPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);
//			adr=reg->RSAYPTR;
//			printf("RSAYPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);
//			adr=reg->RSADPTR;
//			printf("RSADPTR=%x\n",adr);
//			printf("*adr=%x\n",*adr);

	if (dev->rsa_done) {
		crypto_copy_array(x, dev->rsa_x, size, big_endian);
		ret = SPCRYPTO_RET_SUCCESS;
		printf("SPCRYPTO_RET_SUCCESS\n");
	} else {
		ret = SPCRYPTO_RET_DEV_ERROR;
	}

	return ret;
}

int sp_expmod(unsigned char *x, unsigned char *a, unsigned char *e,
		unsigned char *n, unsigned int size, unsigned char big_endian)
{
	int ret = 0;
	printf("sp_expmod start\n");
	if(!x || !a || !e || !n)
		return SPCRYPTO_RET_INVALID_ARG;
	if(size < 24 || size > 256)
		return SPCRYPTO_RET_INVALID_ARG;
	ret = crypto_do_expmod(x, a, e, n, size, big_endian);
	printf("sp_expmod end\n");

	return ret;
//	return crypto_do_expmod(x, a, e, n, size, big_endian);
}

int expmod_test(void)
{
	//unsigned char rand_data[KEY_ARRAY_SIZE];
	unsigned char A[KEY_ARRAY_SIZE];
	unsigned char B[KEY_ARRAY_SIZE];
	unsigned char C[KEY_ARRAY_SIZE];
	unsigned char *E1, *E2, *N;
	unsigned int Abytes, Bbytes, E1bytes, E2bytes, Nbytes;
	unsigned int i;
	unsigned long long enc_time0, enc_time1, dec_time0, dec_time1;  /* unit:ms */
	int ret = 0;

	printf("expmod_test()\n");
	ret=crypto_init();
	
	//crypto_rand_array(rand_data, sizeof(rand_data));
	E2 = key_D_2048;
	E1 = key_E_65537;
	N  = key_N_2048;
	E1bytes = calc_valid_size(E1, KEY_ARRAY_SIZE, 1);
	E2bytes = calc_valid_size(E2, KEY_ARRAY_SIZE, 1);
	Nbytes  = calc_valid_size(N, KEY_ARRAY_SIZE, 1);
	Abytes=256;
	memset(A, 0, KEY_ARRAY_SIZE);
	memset(B, 0, KEY_ARRAY_SIZE);
	memset(C, 0, KEY_ARRAY_SIZE);
	//memcpy(A, rand_data, Abytes);
	memcpy(A, g_base, sizeof(g_base));

	/* private key encrypt */
	enc_time0 = get_timer(0);
	//crypto_dump_buf("A = ", A, sizeof(A));
	//crypto_dump_buf("B = ", B, sizeof(B));
	ret = sp_expmod(B, A, E1, N, KEY_ARRAY_SIZE, 0); //1
	//ERR_OUT(ret!=SPCRYPTO_RET_SUCCESS, fout, "ret=%d", ret);
	enc_time1 = get_timer(enc_time0);
	Bbytes =  calc_valid_size(B, KEY_ARRAY_SIZE, 1); //1

	/* public key decrypt */
	dec_time0 = get_timer(enc_time1);
	//crypto_dump_buf("A = ", A, sizeof(A));
	//crypto_dump_buf("B = ", B, sizeof(B));
	ret = sp_expmod(C, B, E2, N, KEY_ARRAY_SIZE, 0); //1
	//crypto_dump_buf("B = ", B, sizeof(B));
	//crypto_dump_buf("C = ", C, sizeof(C));
	//ERR_OUT(ret!=SPCRYPTO_RET_SUCCESS, fout, "ret=%d", ret);
	dec_time1 = get_timer(dec_time0);

	if(memcmp(A, C, Abytes)) {
		printf("expmod test fail\n");
		crypto_dump_buf("expect = ", A, sizeof(A));
		crypto_dump_buf("result = ", C, sizeof(C));
		return -1;
	}
  printf("memcmp ok!\n");
	//crypto_dump_buf("expect = ", A, sizeof(A));
	//crypto_dump_buf("result = ", C, sizeof(C));

	printf("key N(bits) E1(bits) E2(bits) A(bits) B(bits) enc(ms) dec(ms)\n");
	printf("%d\t%d\t%d\t%d\t%d\t%d\t%lld\t%lld\n",i,(Nbytes<<3),(E1bytes<<3),(E2bytes<<3),(Abytes<<3),(Bbytes<<3),enc_time1,dec_time1);

//fout:
	return 0;
}
#endif

#ifdef RSATEST
int do_expmod(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;

	printf("do_expmod start\n");
//	printf("argv[1]=%s \n",argv[1]);
	ret = expmod_test();

	printf("do_expmod end\n");
	return 0;
}
#endif

U_BOOT_CMD(
	verify, 2, 1, do_verify,
	"verify command",
	"verify kernel signature.\n"
	"\taddr: kernel addr, include uImage header.\n"
	"\tverify 0x307fc0\n"
);
#ifdef RSATEST
U_BOOT_CMD(
	expmod, 2, 1, do_expmod,
	"expmod command",
	"expmod kernel signature.\n"
	"\taddr: kernel addr, include uImage header.\n"
);
#endif
