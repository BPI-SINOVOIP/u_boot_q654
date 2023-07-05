// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Sunplus Technology Co., Ltd.
 *       All rights reserved.
 */
#include <linux/mtd/rawnand.h>
#include <linux/sizes.h>

/*
 * Chip ID list
 * name, device ID, page size, chip size in MiB, eraseblock size, options
 */
const struct nand_flash_dev sp_pnand_ids[] = {
	{"K9F2G08XXX 256MiB ZEBU 8-bit",
		{ .id = {0xec, 0xaa, 0x80, 0x15, 0x50} },
		  SZ_2K, SZ_256, SZ_128K, 0, 5, 64, NAND_ECC_INFO(2, SZ_512), },
	{"GD9AU4G8F3AMGI 512MiB 3.3V 8-bit",
		{ .id = {0xc8, 0xdc, 0x90, 0x95, 0xd6} },
		  SZ_2K, SZ_512, SZ_128K, 0, 5, 64, NAND_ECC_INFO(4, SZ_512), },
	{"GD9FU4G8F4BMGI 512MiB 3.3V 8-bit",
		{ .id = {0xc8, 0xdc, 0x90, 0xa6, 0x57} },
		  SZ_4K, SZ_512, SZ_256K, 0, 5, 256, NAND_ECC_INFO(8, SZ_512), },
	{"K9GBG08U0B 4G 3.3V 8-bit",
		{ .id = {0xec, 0xd7, 0x94, 0x7e, 0x64} },
		  SZ_8K, SZ_4K, SZ_1M, 0, 5, 1024, NAND_ECC_INFO(40, SZ_1K), },

	{NULL}
};
