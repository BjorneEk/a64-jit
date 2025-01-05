#include <stdio.h>
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"


void a64_write_bin(FILE *fp, const a64_jit_t *prog)
{
	fwrite(prog->code, sizeof(a64_t), prog->length, fp);
}
/*
long fn(long i) { return i + 1; }
long main() { return fn(68); }
*/
int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	u64_t (*function)(void);
	u64_t res;

	//char *str = "Hello, World!\n";
	//size_t str_p;
	FILE *out;
	a64_jit_init(&jit, 10);
	a64_jit_push(&jit, a64_addi(R0, R0, 1));
	a64_jit_push(&jit, a64_ret());
	a64_jit_push(&jit, a64_load_store(STORE, PRE, SZX, 0, R30, SP, -8));
	a64_jit_push(&jit, a64_load_store(STORE, PRE, SZX, 0, R29, SP, -8));
	a64_jit_push(&jit, a64_movi(R0, 68));
	a64_jit_push(&jit, a64_bl(-5));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZX, 0, R29, SP, 8));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZX, 0, R30, SP, 8));
	a64_jit_push(&jit, a64_ret());
	out = fopen("files/bl.bin", "w");
	fwrite(&jit.code[2], sizeof(a64_t), jit.length-2, out);
	fclose(out);

	a64_jit_mkexec(&jit);

	function = (u64_t(*)(void))(uintptr_t)(&jit.code[2]);
	res = function();

	//if (argc > 1 && strcmp(argv[1], "-v"))
	printf("result: %li\n", (long)res);

	a64_jit_destroy(&jit);

	return res == 69 ? 0 : -1;
}