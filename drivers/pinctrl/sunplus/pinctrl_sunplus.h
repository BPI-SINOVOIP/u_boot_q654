#ifndef __PINCTRL_SUNPLUS_H__
#define __PINCTRL_SUNPLUS_H__

#include <common.h>


#define pctl_err(fmt, arg...)            printf(fmt, ##arg)
#if 0
#define pctl_info(fmt, arg...)           printf(fmt, ##arg)
#else
#define pctl_info(fmt, arg...)
#endif


#define GPIO_PINGRP(x)  moon1_regs[x]
#ifdef CONFIG_PINCTRL_SUNPLUS
#define GPIO_PINMUX(x)  moon2_regs[x]
#endif

#if defined(CONFIG_PINCTRL_SUNPLUS_Q645) || defined(CONFIG_PINCTRL_SUNPLUS_Q654)
#define GPIO_MASTER(x)  gpioxt_regs[x]
#define GPIO_OE(x)      gpioxt_regs[x+13]
#define GPIO_OUT(x)     gpioxt_regs[x+26]
#define GPIO_IN(x)      gpioxt_regs[x+32+7]
#define GPIO_I_INV(x)   gpioxt_regs[x+32+15]
#define GPIO_O_INV(x)   gpioxt_regs[x+32+28]
#define GPIO_OD(x)      gpioxt_regs[x+64+9]
#else
#define GPIO_MASTER(x)  gpioxt_regs[x]
#define GPIO_OE(x)      gpioxt_regs[x+8]
#define GPIO_OUT(x)     gpioxt_regs[x+16]
#define GPIO_IN(x)      gpioxt_regs[x+24]
#define GPIO_I_INV(x)   gpioxt2_regs[x]
#define GPIO_O_INV(x)   gpioxt2_regs[x+8]
#define GPIO_OD(x)      gpioxt2_regs[x+16]
#endif
#define GPIO_FIRST(x)   first_regs[x]


#ifdef CONFIG_PINCTRL_SUNPLUS
#define MAX_PINS        99
#elif defined (CONFIG_PINCTRL_SUNPLUS_Q645)
#define MAX_PINS        108
#elif defined (CONFIG_PINCTRL_SUNPLUS_Q654)
#define MAX_PINS        108
#else
#define MAX_PINS        108
#endif
#define D(x,y)          ((x)*8+(y))

typedef enum {
	fOFF_0, // nowhere
	fOFF_M, // in mux registers
	fOFF_G, // mux group registers
	fOFF_I, // in iop registers
} fOFF_t;

#define EGRP(n,v,p) { \
	.name = n, \
	.gval = (v), \
	.pins = (p), \
	.pnum = ARRAY_SIZE(p) \
}

#define FNCE(n,r,o,bo,bl,g) { \
	.name = n, \
	.freg = r, \
	.roff = o, \
	.boff = bo, \
	.blen = bl, \
	.grps = (g), \
	.gnum = ARRAY_SIZE(g) \
}

#define FNCN(n,r,o,bo,bl) { \
	.name = n, \
	.freg = r, \
	.roff = o, \
	.boff = bo, \
	.blen = bl, \
	.grps = NULL, \
	.gnum = 0, \
}

typedef struct {
	const char * const name;
	const u8 gval;                  // value for register
	const unsigned * const pins;    // list of pins
	const unsigned pnum;            // number of pins
} sppctlgrp_t;

typedef struct {
	const char * const name;
	const fOFF_t freg;          // function register type
	const u8 roff;                  // register offset
	const u8 boff;                  // bit offset
	const u8 blen;                  // number of bits
	const u8 gnum;                  // number of groups
	const sppctlgrp_t * const grps; // list of groups
} func_t;

extern func_t list_funcs[];
extern const int list_funcsSZ;

extern volatile u32 *moon1_regs;
extern volatile u32 *moon2_regs;
extern volatile u32 *gpioxt_regs;
#if defined(CONFIG_PINCTRL_SUNPLUS)
extern volatile u32 *gpioxt2_regs;
#endif
extern volatile u32 *first_regs;

extern int gpio_pin_mux_set(u32 func, u32 pin);
extern int gpio_pin_mux_get_val(u32 func, u32 *pin);
extern u32 gpio_pin_mux_get(u32 func);
extern int gpio_input_invert_set(u32 bit, u32 val);
extern int gpio_input_invert_get(u32 bit, u32 *val);
extern u32 gpio_input_invert_val_get(u32 bit);
extern int gpio_output_invert_set(u32 bit, u32 val);
extern int gpio_output_invert_get(u32 bit, u32 *val);
extern u32 gpio_output_invert_val_get(u32 bit);
extern int gpio_open_drain_set(u32 bit, u32 val);
extern int gpio_open_drain_get(u32 bit, u32 *val);
extern u32 gpio_open_drain_val_get(u32 bit);
extern int gpio_first_set(u32 bit, u32 val);
extern int gpio_first_get(u32 bit, u32 *val);
extern u32 gpio_first_val_get(u32 bit);
extern int gpio_master_set(u32 bit, u32 val);
extern int gpio_master_get(u32 bit, u32 *val);
extern u32 gpio_master_val_get(u32 bit);
extern int gpio_oe_set(u32 bit, u32 val);
extern int gpio_oe_get(u32 bit, u32 *val);
extern u32 gpio_oe_val_get(u32 bit);
extern int gpio_out_set(u32 bit, u32 val);
extern int gpio_out_get(u32 bit, u32 *val);
extern u32 gpio_out_val_get(u32 bit);
extern int gpio_in(u32 pin, u32 *val);
extern u32 gpio_in_val(u32 bit);
extern u32 gpio_para_get(u32 pin);
extern int gpio_debug_set(u32 bit, u32 val);

#endif
