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
	u64_t (*function)(void);
	u64_t res;

	FILE *out;
	a64_jit_init(&jit, 10);
	a64_jit_push(&jit, a64_mov(R2, R30));
	a64_jit_push(&jit, a64_movi(R0, 68));
	a64_jit_push(&jit, a64_bl(3));
	a64_jit_push(&jit, a64_mov(R30, R2));
	a64_jit_push(&jit, a64_ret());
	a64_jit_push(&jit, a64_addi(R0, R0, 1));
	a64_jit_push(&jit, a64_ret());

	out = fopen("files/bl.bin", "w");
	fwrite(jit.code, sizeof(a64_t), jit.length, out);
	fclose(out);

	a64_jit_mkexec(&jit);

	function = (u64_t(*)(void))(uintptr_t)(jit.code);
	res = function();

	printf("result: %li\n", (long)res);

	a64_jit_destroy(&jit);

	return res == 69 ? 0 : -1;
}