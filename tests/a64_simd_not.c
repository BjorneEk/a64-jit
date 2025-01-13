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
	a64_jit_push(&jit, a64_simd_not(SIMD_HALF, R0, R2));
	a64_jit_push(&jit, a64_simd_not(SIMD_FULL, R0, R2));


	a64_jit_push(&jit, a64_ret());

	out = fopen("files/simdnot.bin", "w");
	a64_write_bin(out, &jit);
	fclose(out);

	a64_jit_mkexec(&jit);

	a64_jit_destroy(&jit);

	return 0;
}