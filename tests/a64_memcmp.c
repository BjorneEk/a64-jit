#include <stdio.h>
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"

/*

FN(memcmp_comb)
	.cfi_startproc

LBL(loop64b):
	cmp	x2, #64
	b.lt	LBL(loop1b)

	ld1	{v0.16b, v1.16b, v2.16b, v3.16b}, [x0], #64
	ld1	{v4.16b, v5.16b, v6.16b, v7.16b}, [x1], #64

	sub	x2, x2, #64

	cmeq	v8.16b, v0.16b, v4.16b
	cmeq	v9.16b, v1.16b, v5.16b
	cmeq	v10.16b, v2.16b, v6.16b
	cmeq	v11.16b, v3.16b, v7.16b

	and	v8.16b, v8.16b, v9.16b
	and	v8.16b, v8.16b, v10.16b
	and	v8.16b, v8.16b, v11.16b

	uminv	B0, v8.16b
	umov	w3, v0.b[0]
	cmp	w3, #0xFF
	b.eq	LBL(loop64b)
LBL(miss64b):
	add	x2, x2, #64
	sub	x0, x0, #64
	sub	x1, x1, #64
LBL(loop1b):
	cbz	x2, LBL(match)
	ldrb	w3, [x0], #1
	ldrb	w4, [x1], #1
	sub	x2, x2, #1
	subs	w5, w3, w4
	b.eq	LBL(loop1b)
	mov	w0, w5
	b	LBL(end)
LBL(match):
	mov	w0, #0
LBL(end):
	ret
	.cfi_endproc
*/

void a64_write_bin(FILE *fp, const a64_jit_t *prog)
{
	fwrite(prog->code, sizeof(a64_t), prog->length, fp);
}
void mk_memcmp(a64_jit_t *jit)
{
	a64_jit_init(jit, 34);
	a64_jit_push(jit, a64_cmpi(R2, 64));
	a64_jit_push(jit, a64_b(LT, 18));
	a64_jit_push(jit, a64_simd_ld1(LD1_IMM, SIMD_B, SIMD_FULL, 4, R0, R0));
	a64_jit_push(jit, a64_simd_ld1(LD1_IMM, SIMD_B, SIMD_FULL, 4, R4, R1));

	a64_jit_push(jit, a64_subi(R2, R2, 64));

	a64_jit_push(jit, a64_simd_cmeq(SIMD_B, SIMD_FULL, R8, R0, R4));
	a64_jit_push(jit, a64_simd_cmeq(SIMD_B, SIMD_FULL, R9, R1, R5));
	a64_jit_push(jit, a64_simd_cmeq(SIMD_B, SIMD_FULL, R10, R2, R6));
	a64_jit_push(jit, a64_simd_cmeq(SIMD_B, SIMD_FULL, R11, R3, R7));

	a64_jit_push(jit, a64_simd_and(SIMD_FULL, R8, R8, R9));
	a64_jit_push(jit, a64_simd_and(SIMD_FULL, R8, R8, R10));
	a64_jit_push(jit, a64_simd_and(SIMD_FULL, R8, R8, R11));

	a64_jit_push(jit, a64_simd_uminv(SIMD_B, SIMD_FULL, R0, R8));

	a64_jit_push(jit, a64_simd_umov(SIMD_B, 0, R3, R0));
	a64_jit_push(jit, a64_cmpiw(R3, 0xFF));
	a64_jit_push(jit, a64_b(EQ, -15));

	a64_jit_push(jit, a64_addi(R2, R2, 64));
	a64_jit_push(jit, a64_subi(R1, R1, 64));
	a64_jit_push(jit, a64_subi(R0, R0, 64));


	a64_jit_push(jit, a64_cmpi(R2, 0));
	a64_jit_push(jit, a64_b(EQ, 8));
	a64_jit_push(jit, a64_load_store(LOAD, POST, SZB, 0, R3, R0, 1));
	a64_jit_push(jit, a64_load_store(LOAD, POST, SZB, 0, R4, R1, 1));
	a64_jit_push(jit, a64_subi(R2, R2, 1));
	a64_jit_push(jit, a64_subsw(R5, R3, R4));
	a64_jit_push(jit, a64_b(EQ, -6));
	a64_jit_push(jit, a64_movw(R0, R5));
	a64_jit_push(jit, a64_ret());
	a64_jit_push(jit, a64_subw(R0, R0, R0));
	//a64_jit_push(jit, a64_moviw(R0, 0));
	a64_jit_push(jit, a64_ret());
}
int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	FILE *out;
	int r1, r2, r3;
	int (*memcmp_fn)(const void *, const void *, size_t);

	char *s1 = "Hello, World!\n";
	char *s2 = "Hello, World!\n";
	char *s3 = "Hello,	world!\n";

	char *s4 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
	char *s5 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab\n";

	mk_memcmp(&jit);

	out = fopen("files/memcmp.bin", "w");
	a64_write_bin(out, &jit);
	fclose(out);

	a64_jit_mkexec(&jit);

	a64_jit_mkexec(&jit);

	memcmp_fn = (int (*)(const void *, const void *, size_t))(uintptr_t)(jit.code);

	r3 = memcmp_fn(s4, s5, 65);
	r1 = memcmp_fn(s1, s2, 14);
	r2 = memcmp_fn(s1, s3, 14);
	a64_jit_destroy(&jit);

	if (r1 != 0)
		fprintf(stderr, "r1 == %d != 0: for s1 == s2 == %s\n", r1, s1);
	if (r2 != ' ' - '	')
		fprintf(stderr, "r2 == %d != ' ' - '	': for s1 != s3 == %s\n", r2, s3);
	if (r3 != 'a' - 'b')
		fprintf(stderr, "r3 == %d != 'a' - 'b': for s4 != s5\n%s\n%s\n\n", r3, s4, s5);
	return (r1 != 0 || r2 != ' ' - '	' || r3 != 'a' - 'b') * -1;
}