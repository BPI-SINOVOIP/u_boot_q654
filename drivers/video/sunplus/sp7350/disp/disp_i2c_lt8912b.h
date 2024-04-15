// SPDX-License-Identifier: GPL-2.0+
/*
 * Author(s): Hammer Hsieh <hammer.hsieh@sunplus.com>
 */

#ifndef __DISP_I2C_LT8912B_H__
#define __DISP_I2C_LT8912B_H__

#include <i2c.h>
//#include <linux/delay.h>

#define LT8912B_I2C_ADDR_MAIN 0x48
#define LT8912B_I2C_ADDR_CEC_DSI 0x49

/*
 * lt8912_input_timing[x][y]
 * y = 0-1, LT8912 width & height
 * y = 2-9, LT8912 HSA & HFP & HBP & HACT & VSA & VFP & VBP & VACT
 */
static const u32 lt8912_input_timing[11][10] = {
	/* (w   h)   HSA   HFP   HBP   HACT  VSA  VFP  VBP   VACT */
	{ 720,  480, 0x3e, 0x3c, 0x10,  720, 0x6, 0x9, 0x1E,  480}, /* 480P */
	{ 720,  576, 0x80, 0x28, 0x58,  720, 0x4, 0x1, 0x17,  576}, /* 576P */
	{1280,  720, 0x28, 0xdc, 0x6e, 1280, 0x5, 0x5, 0x14,  720}, /* 720P */
	{1920, 1080, 0x2c, 0x94, 0x58, 1920, 0x5, 0x4, 0x24, 1080}, /* 1080P */
	{  64,   64, 0x14, 0x28, 0x58,   64, 0x5, 0x5, 0x24,   64}, /* 64x64 */
	{ 128,  128, 0x14, 0x28, 0x58,  128, 0x5, 0x5, 0x24,  128}, /* 128x128 */
	{  64, 2880, 0x14, 0x28, 0x58,   64, 0x5, 0x5, 0x24, 2880}, /* 64x2880 */
	{3840,   64, 0x14, 0x28, 0x58, 3840, 0x5, 0x5, 0x24,   64}, /* 3840x64 */
	{3840, 2880, 0x14, 0x28, 0x58, 3840, 0x5, 0x5, 0x24, 2880}, /* 3840x2880 */
	{ 800,  480, 0x14, 0x28, 0x58,  800, 0x5, 0x5, 0x24,  480}, /* 800x480 */
	{1024,  600, 0x14, 0x28, 0x58, 1024, 0x5, 0x5, 0x24,  600}  /* 1024x600 */
};

void lt8912_write_init_config(struct udevice *p1)
{
    //printf("lt8912_write_init_config\n");
	/* Digital clock en*/
	dm_i2c_reg_write(p1, 0x08, 0xff); /* power down */
	dm_i2c_reg_write(p1, 0x09, 0xff); /* HPD override */
	dm_i2c_reg_write(p1, 0x0a, 0xff); /* color space */
	dm_i2c_reg_write(p1, 0x0b, 0x7c); /* fixed */
	dm_i2c_reg_write(p1, 0x0c, 0xff); /* HDCP */
	dm_i2c_reg_write(p1, 0x42, 0x04);
	/*Tx Analog*/
	dm_i2c_reg_write(p1, 0x31, 0xb1); /* fixed a1 or b1 ? */
	dm_i2c_reg_write(p1, 0x32, 0xb1); /* V1P2 */
	dm_i2c_reg_write(p1, 0x33, 0x0e); /* fixed 0e or 03*/
	dm_i2c_reg_write(p1, 0x37, 0x00); /* fixed */
	dm_i2c_reg_write(p1, 0x38, 0x22); /* fixed */
	dm_i2c_reg_write(p1, 0x60, 0x82); /* fixed */
	/*Cbus Analog*/
	dm_i2c_reg_write(p1, 0x39, 0x45); /* fixed */
	dm_i2c_reg_write(p1, 0x3a, 0x00); /* fixed for improve hdmi output signal */
	dm_i2c_reg_write(p1, 0x3b, 0x00); /* fixed */
	/*HDMI Pll Analog*/
	dm_i2c_reg_write(p1, 0x44, 0x31); /* fixed */
	dm_i2c_reg_write(p1, 0x55, 0x44); /* fixed */
	dm_i2c_reg_write(p1, 0x57, 0x01); /* fixed */
	dm_i2c_reg_write(p1, 0x5a, 0x02); /* fixed */
	/*MIPI Analog*/
	//org
	dm_i2c_reg_write(p1, 0x3e, 0xd6); /* fixed d6 or c6 or 8e ? */
	dm_i2c_reg_write(p1, 0x3f, 0xd4); /* fixed d4 or not setting */
	dm_i2c_reg_write(p1, 0x41, 0x3c); /* fixed 3c or 7c ? */
	dm_i2c_reg_write(p1, 0xB2, 0x00); /* fixed 00 or not setting */
	dm_i2c_reg_write(p1, 0x42, 0x05); /* fixed 04 or 05 ? */
	#if 0
	//test
	dm_i2c_reg_write(p1, 0x3e, 0xc6);
	//dm_i2c_reg_write(p1, 0x3f, 0xd4);
	dm_i2c_reg_write(p1, 0x41, 0x7c);
	dm_i2c_reg_write(p1, 0xB2, 0x00);
	#endif

}
void lt8912_write_mipi_basic_config(struct udevice *p2)
{
    //printf("lt8912_write_mipi_basic_config\n");
	dm_i2c_reg_write(p2, 0x12, 0x04);
	dm_i2c_reg_write(p2, 0x14, 0x00);
	dm_i2c_reg_write(p2, 0x15, 0x00); /* fixed 00 or 06 ? */
	dm_i2c_reg_write(p2, 0x1a, 0x03);
	dm_i2c_reg_write(p2, 0x1b, 0x03);

}
void lt8912_write_param_by_resolution(struct udevice *p2, int w, int h)
{
	u32 hactive, h_total, hpw, hfp, hbp;
	u32 vactive, v_total, vpw, vfp, vbp;
	u8 settle = 0x08;
	int i;

    //printf("lt8912_write_param_by_resolution\n");

	for (i = 0; i < 11; i++) {
		if ((lt8912_input_timing[i][0] == w) &&
			(lt8912_input_timing[i][1] == h)) {
				//time_cnt = i;
				break;
		}
	}
	//printf("Disp: i %d\n", i);

	if (i >= 11) i = 0;

	hactive = lt8912_input_timing[i][5];
	hfp = lt8912_input_timing[i][3];
	hpw = lt8912_input_timing[i][2];
	hbp = lt8912_input_timing[i][4];
	h_total = hactive + hfp + hpw + hbp;

	vactive = lt8912_input_timing[i][9];
	vfp = lt8912_input_timing[i][7];
	vpw = lt8912_input_timing[i][6];
	vbp = lt8912_input_timing[i][8];
	v_total = vactive + vfp + vpw + vbp;

	dm_i2c_reg_write(p2, 0x10, 0x01); //fix
	if (vactive <= 600)
		settle = 0x04;
	else if (vactive == 1080)
		settle = 0x0a;
	dm_i2c_reg_write(p2, 0x11, settle); //0x04 or 0x0a
	dm_i2c_reg_write(p2, 0x18, hpw);
	dm_i2c_reg_write(p2, 0x19, vpw);
	dm_i2c_reg_write(p2, 0x1c, hactive & 0xff);
	dm_i2c_reg_write(p2, 0x1d, hactive >> 8);

	//dm_i2c_reg_write(p2, 0x1e, 0x67);
	dm_i2c_reg_write(p2, 0x2f, 0x0c);

	dm_i2c_reg_write(p2, 0x34, h_total & 0xff);
	dm_i2c_reg_write(p2, 0x35, h_total >> 8);

	dm_i2c_reg_write(p2, 0x36, v_total & 0xff);
	dm_i2c_reg_write(p2, 0x37, v_total >> 8);

	dm_i2c_reg_write(p2, 0x38, vbp & 0xff);
	dm_i2c_reg_write(p2, 0x39, vbp >> 8);

	dm_i2c_reg_write(p2, 0x3a, vfp & 0xff);
	dm_i2c_reg_write(p2, 0x3b, vfp >> 8);

	dm_i2c_reg_write(p2, 0x3c, hbp & 0xff);
	dm_i2c_reg_write(p2, 0x3d, hbp >> 8);

	dm_i2c_reg_write(p2, 0x3e, hfp & 0xff);
	dm_i2c_reg_write(p2, 0x3f, hfp >> 8);

}
void lt8912_write_dds_config(struct udevice *p2)
{
    //printf("lt8912_write_dds_config\n");

	dm_i2c_reg_write(p2, 0x4e, 0xff);
	dm_i2c_reg_write(p2, 0x4f, 0x56);
	dm_i2c_reg_write(p2, 0x50, 0x69);
	dm_i2c_reg_write(p2, 0x51, 0x80);
	dm_i2c_reg_write(p2, 0x1f, 0x5e);
	dm_i2c_reg_write(p2, 0x20, 0x01);
	dm_i2c_reg_write(p2, 0x21, 0x2c);
	dm_i2c_reg_write(p2, 0x22, 0x01);
	dm_i2c_reg_write(p2, 0x23, 0xfa);
	dm_i2c_reg_write(p2, 0x24, 0x00);
	dm_i2c_reg_write(p2, 0x25, 0xc8);
	dm_i2c_reg_write(p2, 0x26, 0x00);
	dm_i2c_reg_write(p2, 0x27, 0x5e);
	dm_i2c_reg_write(p2, 0x28, 0x01);
	dm_i2c_reg_write(p2, 0x29, 0x2c);
	dm_i2c_reg_write(p2, 0x2a, 0x01);
	dm_i2c_reg_write(p2, 0x2b, 0xfa);
	dm_i2c_reg_write(p2, 0x2c, 0x00);
	dm_i2c_reg_write(p2, 0x2d, 0xc8);
	dm_i2c_reg_write(p2, 0x2e, 0x00);
	dm_i2c_reg_write(p2, 0x42, 0x64);
	dm_i2c_reg_write(p2, 0x43, 0x00);
	dm_i2c_reg_write(p2, 0x44, 0x04);
	dm_i2c_reg_write(p2, 0x45, 0x00);
	dm_i2c_reg_write(p2, 0x46, 0x59);
	dm_i2c_reg_write(p2, 0x47, 0x00);
	dm_i2c_reg_write(p2, 0x48, 0xf2);
	dm_i2c_reg_write(p2, 0x49, 0x06);
	dm_i2c_reg_write(p2, 0x4a, 0x00);
	dm_i2c_reg_write(p2, 0x4b, 0x72);
	dm_i2c_reg_write(p2, 0x4c, 0x45);
	dm_i2c_reg_write(p2, 0x4d, 0x00);
	dm_i2c_reg_write(p2, 0x52, 0x08);
	dm_i2c_reg_write(p2, 0x53, 0x00);
	dm_i2c_reg_write(p2, 0x54, 0xb2);
	dm_i2c_reg_write(p2, 0x55, 0x00);
	dm_i2c_reg_write(p2, 0x56, 0xe4);
	dm_i2c_reg_write(p2, 0x57, 0x0d);
	dm_i2c_reg_write(p2, 0x58, 0x00);
	dm_i2c_reg_write(p2, 0x59, 0xe4);
	dm_i2c_reg_write(p2, 0x5a, 0x8a);
	dm_i2c_reg_write(p2, 0x5b, 0x00);
	dm_i2c_reg_write(p2, 0x5c, 0x34);
	dm_i2c_reg_write(p2, 0x1e, 0x4f);
	dm_i2c_reg_write(p2, 0x51, 0x00);

}
void lt8912_write_rxlogicres_config(struct udevice *p1)
{
    //printf("lt8912_write_rxlogicres_config\n");

	dm_i2c_reg_write(p1, 0x03, 0x7f);
	udelay(10000); //delay 10ms ~ 20ms
	dm_i2c_reg_write(p1, 0x03, 0xff);

}
void lt8912_write_lvds_config(struct udevice *p2)
{
    //printf("lt8912_write_lvds_config\n");
#if 0
	dm_i2c_reg_write(p2, 0x44, 0x30);
	dm_i2c_reg_write(p2, 0x51, 0x05);
	dm_i2c_reg_write(p2, 0x50, 0x24);
	dm_i2c_reg_write(p2, 0x51, 0x2d);
	dm_i2c_reg_write(p2, 0x52, 0x04);
	dm_i2c_reg_write(p2, 0x69, 0x0e);
	dm_i2c_reg_write(p2, 0x69, 0x8e);
	dm_i2c_reg_write(p2, 0x6a, 0x00);
	dm_i2c_reg_write(p2, 0x6c, 0xb8);
	dm_i2c_reg_write(p2, 0x6b, 0x51);
	dm_i2c_reg_write(p2, 0x04, 0xfb);
	dm_i2c_reg_write(p2, 0x04, 0xff);
	dm_i2c_reg_write(p2, 0x7f, 0x00);
	dm_i2c_reg_write(p2, 0xa8, 0x13);
	dm_i2c_reg_write(p2, 0x02, 0xf7);
	dm_i2c_reg_write(p2, 0x02, 0xff);
	dm_i2c_reg_write(p2, 0x03, 0xcf);
	dm_i2c_reg_write(p2, 0x03, 0xff);
#endif

}

void lt8912_write_test_pattern(struct udevice *p2)
{
	#if 0
	/* test pattern for 480P */
	dm_i2c_reg_write(p2, 0x72, 0x12);
	dm_i2c_reg_write(p2, 0x73, 0x7a); //RGD_PTN_DE_DLY[7:0]
	dm_i2c_reg_write(p2, 0x74, 0x00); //RGD_PTN_DE_DLY[11:8]
	dm_i2c_reg_write(p2, 0x75, 0x24); //RGD_PTN_DE_TOP[6:0]
	dm_i2c_reg_write(p2, 0x76, 0xd0); //RGD_PTN_DE_CNT[7:0]
	dm_i2c_reg_write(p2, 0x77, 0xe0); //RGD_PTN_DE_LIN[7:0]
	dm_i2c_reg_write(p2, 0x78, 0x12); //RGD_PTN_DE_LIN[11:8],RGD_PTN_DE_CNT[11:8]
	dm_i2c_reg_write(p2, 0x79, 0x5a); //RGD_PTN_H_TOTAL[7:0]
	dm_i2c_reg_write(p2, 0x7a, 0x0d); //RGD_PTN_V_TOTAL[7:0]
	dm_i2c_reg_write(p2, 0x7b, 0x23); //RGD_PTN_V_TOTAL[11:8],RGD_PTN_H_TOTAL[11:8]
	dm_i2c_reg_write(p2, 0x7c, 0x3e); //RGD_PTN_HWIDTH[7:0]
	dm_i2c_reg_write(p2, 0x7d, 0x06); //RGD_PTN_HWIDTH[9:8],RGD_PTN_VWIDTH[5:0]

	dm_i2c_reg_write(p2, 0x70, 0x80);
	dm_i2c_reg_write(p2, 0x71, 0x51);
	//DDS ?? CLK
	dm_i2c_reg_write(p2, 0x4e, 0x66);
	dm_i2c_reg_write(p2, 0x4f, 0x66);
	dm_i2c_reg_write(p2, 0x50, 0x26);

	dm_i2c_reg_write(p2, 0x51, 0x80); //pattern_en
	#endif
	#if 0
	/* test pattern for 1080P */
	dm_i2c_reg_write(p2, 0x72, 0x12);
	dm_i2c_reg_write(p2, 0x73, 0xc0); //RGD_PTN_DE_DLY[7:0]
	dm_i2c_reg_write(p2, 0x74, 0x00); //RGD_PTN_DE_DLY[11:8]
	dm_i2c_reg_write(p2, 0x75, 0x29); //RGD_PTN_DE_TOP[6:0]
	dm_i2c_reg_write(p2, 0x76, 0x80); //RGD_PTN_DE_CNT[7:0]
	dm_i2c_reg_write(p2, 0x77, 0x38); //RGD_PTN_DE_LIN[7:0]
	dm_i2c_reg_write(p2, 0x78, 0x47); //RGD_PTN_DE_LIN[11:8],RGD_PTN_DE_CNT[11:8]
	dm_i2c_reg_write(p2, 0x79, 0x98); //RGD_PTN_H_TOTAL[7:0]
	dm_i2c_reg_write(p2, 0x7a, 0x65); //RGD_PTN_V_TOTAL[7:0]
	dm_i2c_reg_write(p2, 0x7b, 0x48); //RGD_PTN_V_TOTAL[11:8],RGD_PTN_H_TOTAL[11:8]
	dm_i2c_reg_write(p2, 0x7c, 0x2c); //RGD_PTN_HWIDTH[7:0]
	dm_i2c_reg_write(p2, 0x7d, 0x05); //RGD_PTN_HWIDTH[9:8],RGD_PTN_VWIDTH[5:0]

	dm_i2c_reg_write(p2, 0x70, 0x80);
	dm_i2c_reg_write(p2, 0x71, 0x76);
	//DDS 148.5M CLK
	dm_i2c_reg_write(p2, 0x4e, 0x33);
	dm_i2c_reg_write(p2, 0x4f, 0x33);
	dm_i2c_reg_write(p2, 0x50, 0xd3);

	dm_i2c_reg_write(p2, 0x51, 0x80); //pattern_en
	#endif
}

#endif	//__DISP_I2C_LT8912B_H__
