#include <stdio.h>
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"


void a64_write_bin(FILE *fp, const a64_jit_t *prog)
{
	fwrite(prog->code, sizeof(a64_t), prog->length, fp);
}

int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	FILE *out;

	a64_jit_init(&jit, 34);
	a64_jit_push(&jit, a64_simd_umaxp(SIMD_H, SIMD_HALF, R0, R1, R1));
	a64_jit_push(&jit, a64_simd_umaxp(SIMD_B, SIMD_FULL, R0, R1, R1));
	a64_jit_push(&jit, a64_simd_umaxp(SIMD_S, SIMD_FULL, R0, R1, R1));
	a64_jit_push(&jit, a64_simd_uminp(SIMD_H, SIMD_HALF, R0, R1, R1));
	a64_jit_push(&jit, a64_simd_uminp(SIMD_B, SIMD_FULL, R0, R1, R1));
	a64_jit_push(&jit, a64_simd_uminp(SIMD_S, SIMD_FULL, R0, R1, R1));

	a64_jit_push(&jit, a64_ret());

	out = fopen("files/umaxp.bin", "w");
	a64_write_bin(out, &jit);
	fclose(out);

	a64_jit_mkexec(&jit);

	a64_jit_destroy(&jit);

	return 0;
}