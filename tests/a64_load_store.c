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
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZX, 0, R0, R1, 8));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZW, 0, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZH, 0, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZB, 0, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZW, 1, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZH, 1, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZB, 1, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZX, 0, R0, R1, 8));
	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZW, 0, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZH, 0, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZB, 0, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZW, 1, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZH, 1, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(LOAD, PRE, SZB, 1, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZX, 0, R0, R1, 8));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZW, 0, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZH, 0, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZB, 0, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZW, 1, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZH, 1, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(LOAD, POST, SZB, 1, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(STORE, IMM, SZX, 0, R0, R1, 8));
	a64_jit_push(&jit, a64_load_store(STORE, IMM, SZW, 0, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(STORE, IMM, SZH, 0, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(STORE, IMM, SZB, 0, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(STORE, PRE, SZX, 0, R0, SP, 8));
	a64_jit_push(&jit, a64_load_store(STORE, PRE, SZW, 0, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(STORE, PRE, SZH, 0, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(STORE, PRE, SZB, 0, R0, R1, 1));

	a64_jit_push(&jit, a64_load_store(STORE, POST, SZX, 0, R0, R1, 8));
	a64_jit_push(&jit, a64_load_store(STORE, POST, SZW, 0, R0, R1, 4));
	a64_jit_push(&jit, a64_load_store(STORE, POST, SZH, 0, R0, R1, 2));
	a64_jit_push(&jit, a64_load_store(STORE, POST, SZB, 0, R0, R1, 1));
	a64_jit_push(&jit, a64_ret());

	out = fopen("files/ldst.bin", "w");
	a64_write_bin(out, &jit);
	fclose(out);

	a64_jit_mkexec(&jit);

	a64_jit_destroy(&jit);

	return 0;
}