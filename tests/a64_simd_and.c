#include <stdio.h>
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"

/*
fn:
	mov	w2, #0
loop:
	ldrsw	w3, [x0, #1]!
	cmp	x0, num + len
	add	w2, w2, w3
	b.ne	loop
end:
	ret

SIMD_B,
	SIMD_H,
	SIMD_S,
	SIMD_D
} a64_simd_sz_t;

typedef enum a64_simd_q {
	SIMD_HALF,
	SIMD_FULL


*/
void a64_write_bin(FILE *fp, const a64_jit_t *prog)
{
	fwrite(prog->code, sizeof(a64_t), prog->length, fp);
}

int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	FILE *out;

	a64_jit_init(&jit, 34);
	a64_jit_push(&jit, a64_simd_and(SIMD_HALF, R0, R1, R2));
	a64_jit_push(&jit, a64_simd_and(SIMD_FULL, R0, R1, R2));

	a64_jit_push(&jit, a64_ret());

	out = fopen("files/simd_and.bin", "w");
	a64_write_bin(out, &jit);
	fclose(out);

	a64_jit_mkexec(&jit);

	a64_jit_destroy(&jit);

	return 0;
}