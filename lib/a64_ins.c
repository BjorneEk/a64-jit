
#include "../headers/types.h"
#include "../headers/a64_ins.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>



static a64_t a64_uncond_branch(i32_t imm)
{
	const a64_t imask		= 0b00010100000000000000000000000000;
	const a64_t getbits_mask	= 0b00000011111111111111111111111111;

	return imask | (imm & getbits_mask);
}


a64_t a64_b(a64_cond_t cond, i32_t imm)
{
	if (cond == B)
		return a64_uncond_branch(imm);
	return (0b01010100 << 24) | (imm & 0b1111111111111111111) << 5 | (cond & 0xF);
}

a64_t a64_br(a64_reg_t r)
{
	const a64_t imask = 0b11010110000111110000000000000000;
	return imask | (r << 5);
}

a64_t a64_bl(a64_t imm)
{
	return (1 << 31) | a64_uncond_branch(imm);
}

a64_t a64_bli(a64_t imm)
{
	const a64_t imask	= 0b10010100000000000000000000000000;
	if (imm & 0b111111 << 26) {
		fprintf(stderr, "%li: immidiate value out of range +/-128MB\n", (long)imm);
		exit(-1);
	}
	return imask | imm;
}

a64_t a64_blr(a64_reg_t r)
{
	const a64_t imask	= 0b11010110001111110000000000000000;

	return imask | (r << 5);
}


static a64_t a64_imm(i32_t imm)
{
	const
	a64_t imm_mask = 0b00000000011111111111110000000000;
	if (imm > 4096 || imm < -4096) {
		fprintf(stderr, "%d: immidiate value out of range [-4096 4096]\n", imm);
		exit(-1);
	}
	return (imm << 10) & imm_mask;
}



static void err_range(i64_t imm, i64_t min, i64_t max, char *ty)
{
	fprintf(stderr, "%li: immidiate %s out of range [%li %li]\n", (long)imm, ty, (long)min, (long)max);
	exit(-1);
}
static void err_mod(i64_t imm, u64_t mul, char *ty)
{
	fprintf(stderr, "%li: immidiate %s out not a multiple of %lu\n", (long)imm, ty, (unsigned long)mul);
	exit(-1);

}

static a64_t a64_mov_(a64_reg_t dst, a64_reg_t src, u32_t word)
{
	const a64_t imask	= 0b00101010000000000000001111100000;

	return (src << 16) | dst | (imask | (word << 31));
}
a64_t a64_mov(a64_reg_t dst, a64_reg_t src)
{
	return a64_mov_(dst, src, 1);
}
a64_t a64_movw(a64_reg_t dst, a64_reg_t src)
{
	return a64_mov_(dst, src, 0);
}

static a64_t a64_movi_(a64_reg_t dst, u64_t imm, u32_t word)
{
	const a64_t imask	= 0b01010010100000000000000000000000;
	if (imm < 0x10000) {
		return (imask | (word << 31)) | (imm << 5) | dst;
	}
	imm >>= 16;
	if (imm < 0x10000)
		return ((imask | (word << 31)) ^ (1 << 21)) | (imm << 5) | dst;
	if ((imm & (~0 ^ 0xFFFF)) != imm) {
		fprintf(stderr, "value '0x%lX' cannot be moved in a single instruction\n", (unsigned long)imm << 16);
		exit(-1);
	}
	imm >>= 16;
	if (imm < 0x10000)
		return ((imask | (word << 31)) ^ (2 << 21)) | (imm << 5) | dst;
	if ((imm & (~0 ^ 0xFFFF)) != imm) {
		fprintf(stderr, "value '0x%lX' cannot be moved in a single instruction\n", (unsigned long)imm << 32);
		exit(-1);
	}
	imm >>= 16;
	if (imm < 0x10000)
		return ((imask | (word << 31)) ^ (3 << 21)) | (imm << 5) | dst;
	fprintf(stderr, "value '0x%lX' cannot be moved in a single instruction\n", (unsigned long)imm << 48);
	exit(-1);
}

a64_t a64_movi(a64_reg_t dst, u64_t imm)
{
	return a64_movi_(dst, imm, 1);
}
a64_t a64_moviw(a64_reg_t dst, u64_t imm)
{
	return a64_movi_(dst, imm, 0);
}

a64_t a64_cmpiw(a64_reg_t reg, i32_t imm)
{
	const a64_t imask	= 0b01110001000000000000000000011111;

	return (reg << 5) | a64_imm(imm) | imask;
}

a64_t a64_cmpi(a64_reg_t reg, i32_t imm)
{
	return a64_cmpiw(reg, imm) | (1 << 31);
}

a64_t a64_addi(a64_reg_t dst, a64_reg_t src, i32_t imm)
{
	const a64_t imask	= 0b10010001000000000000000000000000;
	return (src << 5) | dst | a64_imm(imm) | imask;
}

a64_t a64_subi(a64_reg_t dst, a64_reg_t src, i32_t imm)
{
	const a64_t imask	= 0b11010001000000000000000000000000;
	return (src << 5) | dst | a64_imm(imm) | imask;
}

a64_t a64_adr(a64_reg_t dst, i32_t imm21)
{
	const a64_t imask	= 0b00010000000000000000000000000000;

	if (imm21 > 0xFFFFFF || -imm21 > 0xFFFFFF) {
		fprintf(stderr, "%li: immidiate value out of range +/-1MB\n", (long)imm21);
		exit(-1);
	}

	return dst | (0x7FFFF & imm21) << 5 | ((0x60000000 & imm21) << 5) | imask;
}

static const a64_t ADC_SBC_imask = 0b00011010000000000000000000000000;
static const a64_t ADD_SUB_imask = 0b00001011000000000000000000000000;
enum {
	ADD = 0,
	SUB = 1,
};
enum {
	W = 0,
	X = 1,
	H,
	BY
};
/*
and
orr
eor
mvn

*/
a64_t a64_add_sub(a64_t ins, u32_t sz, u32_t op, u32_t flags, a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return (sz << 31) | (op << 30) | (flags << 29) | dst | (s1 << 5) | (s2 << 16) | ins;
}

a64_t a64_adc(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, X, ADD, 0, dst, s1, s2);
}
a64_t a64_adcw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, W, ADD, 0, dst, s1, s2);
}
a64_t a64_adcs(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, X, ADD, 1, dst, s1, s2);
}
a64_t a64_adcsw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, W, ADD, 1, dst, s1, s2);
}
a64_t a64_sbc(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, X, SUB, 0, dst, s1, s2);
}
a64_t a64_sbcw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, W, SUB, 0, dst, s1, s2);
}
a64_t a64_sbcs(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, X, SUB, 1, dst, s1, s2);
}
a64_t a64_sbcsw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADC_SBC_imask, W, SUB, 1, dst, s1, s2);
}
a64_t a64_add(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, X, ADD, 0, dst, s1, s2);
}
a64_t a64_addw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, W, ADD, 0, dst, s1, s2);
}
a64_t a64_adds(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, X, ADD, 1, dst, s1, s2);
}
a64_t a64_addsw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, W, ADD, 1, dst, s1, s2);
}
a64_t a64_sub(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, X, SUB, 0, dst, s1, s2);
}
a64_t a64_subw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, W, SUB, 0, dst, s1, s2);
}
a64_t a64_subs(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, X, SUB, 1, dst, s1, s2);
}
a64_t a64_subsw(a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	return a64_add_sub(ADD_SUB_imask, W, SUB, 1, dst, s1, s2);
}

a64_t a64_cmp(a64_reg_t r1, a64_reg_t r2)
{
	return a64_subs(0x1F, r1, r2);
}
a64_t a64_cmpw(a64_reg_t r1, a64_reg_t r2)
{
	return a64_subsw(0x1F, r1, r2);
}




static a64_t imm_addr(a64_sz_t sz, u64_t imm)
{
	switch (sz) {
		case SZX:
			if (imm > 32760)
				err_range(imm, 0, 32760, "offset");
			if (imm % 8 != 0)
				err_mod(imm, 8, "offset");
			return imm / 8;
		case SZW:
			if (imm > 16380)
				err_range(imm, 0, 16380, "offset");
			if (imm % 4 != 0)
				err_mod(imm, 4, "offset");
			return imm / 4;
		case SZH:
			if (imm > 8190)
				err_range(imm, 0, 8190, "offset");
			if (imm % 2 != 0)
				err_mod(imm, 2, "offset");
			return imm / 2;
		case SZB:
			if (imm > 4095)
				err_range(imm, 0, 4095, "offset");
			return imm;
	}
	return 0;
}
static a64_t post_pre_addr(i64_t imm)
{
	if (imm > 255 || imm < -256)
		err_range(imm, -256, 255, "index");
	return imm & 0x1FF;
}
/*
static const a64_t LDRX_IMM_imask   = 0b11111001010000000000000000000000;
static const a64_t LDRW_IMM_imask   = 0b10111001010000000000000000000000;
static const a64_t LDRH_IMM_imask   = 0b01111001010000000000000000000000;
static const a64_t LDRB_IMM_imask   = 0b00111001010000000000000000000000;

static const a64_t LDRSW_IMM_imask  = 0b10111001100000000000000000000000;
static const a64_t LDRSH_IMM_imask  = 0b01111001110000000000000000000000;
static const a64_t LDRSB_IMM_imask  = 0b00111001110000000000000000000000;

static const a64_t LDRX_PRE_imask   = 0b11111000010000000000110000000000;
static const a64_t LDRW_PRE_imask   = 0b10111000010000000000110000000000;
static const a64_t LDRH_PRE_imask   = 0b01111000010000000000110000000000;
static const a64_t LDRB_PRE_imask   = 0b00111000010000000000110000000000;

static const a64_t LDRSW_PRE_imask  = 0b10111000100000000000110000000000;
static const a64_t LDRSH_PRE_imask  = 0b01111000110000000000110000000000;
static const a64_t LDRSB_PRE_imask  = 0b00111000110000000000110000000000;

static const a64_t LDRX_POST_imask  = 0b11111000010000000000010000000000;
static const a64_t LDRW_POST_imask  = 0b10111000010000000000010000000000;
static const a64_t LDRH_POST_imask  = 0b01111000010000000000010000000000;
static const a64_t LDRB_POST_imask  = 0b00111000010000000000010000000000;

static const a64_t LDRSW_POST_imask = 0b10111000100000000000010000000000;
static const a64_t LDRSH_POST_imask = 0b01111000110000000000010000000000;
static const a64_t LDRSB_POST_imask = 0b00111000110000000000010000000000;

static const a64_t STRX_IMM_imask   = 0b11111001000000000000000000000000;
static const a64_t STRW_IMM_imask   = 0b10111001000000000000000000000000;
static const a64_t STRH_IMM_imask   = 0b01111001000000000000000000000000;
static const a64_t STRB_IMM_imask   = 0b00111001000000000000000000000000;

static const a64_t STRX_PRE_imask   = 0b11111000000000000000110000000000;
static const a64_t STRW_PRE_imask   = 0b10111000000000000000110000000000;
static const a64_t STRH_PRE_imask   = 0b01111000000000000000110000000000;
static const a64_t STRB_PRE_imask   = 0b00111000000000000000110000000000;

static const a64_t STRX_POST_imask  = 0b11111000000000000000010000000000;
static const a64_t STRW_POST_imask  = 0b10111000000000000000010000000000;
static const a64_t STRH_POST_imask  = 0b01111000000000000000010000000000;
static const a64_t STRB_POST_imask  = 0b00111000000000000000010000000000;
*/
//0b11111000000111111000011111111110
//0b11111000000000000000010000000000;

enum oc {
	LD_IMM		= 0b101,
	LD_SW_IMM	= 0b110,
	LD_SBH_IMM	= 0b111,
	LD_PP		= 0b001,
	LD_SW_PP	= 0b010,
	LD_SBH_PP	= 0b011,
	ST_IMM		= 0b100,
	ST_PP		= 0b000
};

static a64_t get_oc(a64_addressing_mode_t mode, a64_sz_t sz, bool sign, ldr_str_t ty)
{
	switch (ty) {
		case LOAD:
			switch (mode) {
				case IMM:
					if (sign)
						return sz == SZW ? LD_SW_IMM : LD_SBH_IMM;
					return LD_IMM;
				case PRE:
				case POST:
					if (sign)
						return sz == SZW ? LD_SW_PP : LD_SBH_PP;
					return LD_PP;
			}
		case STORE:
			switch (mode) {
				case IMM:
					return ST_IMM;
				case PRE:
				case POST:
					return ST_PP;
			}
	}
	return 0;
}
/*
ldr x, [x, #imm]
ldr w, [w, #imm]
ldrh w, [w, #imm]
ldrb w, [w, #imm]

ldr x, [x, #imm]!
ldr w, [w, #imm]!
ldrh w, [w, #imm]!
ldrb w, [w, #imm]!

ldr x, [x], #imm
ldr w, [w], #imm
ldrh w, [w], #imm
ldrb w, [w], #imm
*/
a64_t a64_load_store(
	ldr_str_t		ldst,
	a64_addressing_mode_t	mode,
	a64_sz_t		sz,
	bool			sign_extend,
	a64_reg_t		dst,
	a64_reg_t		addr,
	i64_t			imm)
{
	a64_t imm_;
	a64_t oc;
	a64_t b11_10;

	b11_10 = 0;

	if (sz == SZX && sign_extend) {
		fprintf(stderr, "cannot sign extend: x%d\n", dst);
		exit(-1);
	}

	oc = get_oc(mode, sz, sign_extend, ldst);

	if (mode == IMM) {
		imm_ = imm_addr(sz, imm) << 10;
	} else {
		imm_ = post_pre_addr(imm) << 12;
		b11_10 = mode << 10;
	}
	return (oc << 22) |
		(0b11100 << 25) |
		(sz << 30) |
		b11_10 |
		imm_ |
		(addr << 5) |
		dst;
}

a64_t a64_brk(u32_t imm16)
{
	return 0xD4200000 | (imm16 << 5);
}

a64_t a64_simd_ld1(a64_simd_ld1_mode_t mode, a64_simd_sz_t size, a64_simd_q_t q, u32_t cnt, a64_reg_t dst, a64_reg_t addr, ...)
{
	const u32_t opcode[] = {7, 10, 6, 2};
	const a64_t mask = 0b00001100010000000000000000000000;
	a64_t res;
	va_list args;
	va_start(args, addr);

	res = (size << 10) | (opcode[cnt - 1] << 12) | (q << 30) | (addr << 5) | dst | mask;
	switch (mode) {
		case LD1_NONE: return res;
		case LD1_IMM: return res | (1 << 23) | (31 << 16);
		default: return res | (1 << 23) | ((31 & va_arg(args, a64_reg_t)) << 16);
	}

}

a64_t a64_simd_and(a64_simd_q_t q, a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	const a64_t mask = 0b00001110001000000001110000000000;
	return (q << 30) | (s2 << 16) | (s1 << 5) | dst | mask;
}

a64_t a64_simd_cmeq(a64_simd_sz_t size, a64_simd_q_t q, a64_reg_t dst, a64_reg_t s1, a64_reg_t s2)
{
	const a64_t mask = 0b00101110001000001000110000000000;
	return (size << 22) | (q << 30) | (s2 << 16) | (s1 << 5) | dst | mask;
}

a64_t a64_simd_umaxv(a64_simd_sz_t size, a64_simd_q_t q, a64_reg_t dst, a64_reg_t src)
{
	const a64_t mask = 0b00101110001100001010100000000000;
	return (size << 22) | (q << 30) | (src << 5) | dst | mask;
}

a64_t a64_simd_uminv(a64_simd_sz_t size, a64_simd_q_t q, a64_reg_t dst, a64_reg_t src)
{
	const a64_t mask = 0b00101110001100011010100000000000;
	return (size << 22) | (q << 30) | (src << 5) | dst | mask;
}

a64_t a64_simd_umov(a64_simd_sz_t size, u32_t idx, a64_reg_t dst, a64_reg_t src)
{
	a64_t imm5_q;
	const a64_t mask = 0b00001110000000000011110000000000;
	switch (size) {
		case SIMD_B: imm5_q = (1 | (idx << 1)) << 16; break;
		case SIMD_H: imm5_q = (2 | (idx << 2)) << 16; break;
		case SIMD_S: imm5_q = (4 | (idx << 3)) << 16; break;
		case SIMD_D: imm5_q = ((8 | (idx << 4)) << 16) | (1 << 30); break;
	}

	return (src << 5) | imm5_q | dst | mask;
}
a64_t a64_simd_movi(a64_simd_q_t q, u32_t op, u32_t cmode, u8_t imm, a64_reg_t dst)
{
	const a64_t mask = 0b00001111000000000000010000000000;
	return (q << 30) | (op << 29) |
		(((imm >> 5) & 7) << 16) |
		((cmode & 15) << 12) | ((imm & 31) << 5) | dst | mask;
}
a64_t a64_simd_movib(a64_simd_q_t q, a64_reg_t dst, u8_t imm8)
{
	return a64_simd_movi(q, 0, 14, imm8, dst);
}
