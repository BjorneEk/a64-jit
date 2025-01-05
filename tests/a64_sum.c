#include <stdio.h>
#include <string.h>
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"

/*
v1,,vn
fn:
	adr	x0, -n
	adr	x1, 0
	mov	w2, #0
loop:
	ldrsw	w3, [x0, #1]!
	cmp	x0, x1
	add	w2, w2, w3
	b.ne	loop
end:
	mov	w0, w2
	ret




*/
void a64_write_bin(FILE *fp, const a64_jit_t *prog)
{
	fwrite(prog->code, sizeof(a64_t), prog->length, fp);
}

int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	u64_t res;
	u64_t (*function)(void);
	u64_t arr[] = {1, 2, 3, 4};
	size_t rlen;
	FILE *out;

	a64_jit_init(&jit, 30);
	rlen = a64_jit_push_data(&jit, arr, sizeof arr);
	a64_jit_push(&jit, a64_adr(R0, -rlen));
	a64_jit_push(&jit, a64_adr(R1, -1));
	a64_jit_push(&jit, a64_moviw(R2, 0));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZX, 0, R3, R0, 0));
	a64_jit_push(&jit, a64_add(R2, R2, R3));
	a64_jit_push(&jit, a64_addi(R0, R0, 8));
	a64_jit_push(&jit, a64_cmp(R0, R1));
	a64_jit_push(&jit, a64_branch(EQ, 2));
	a64_jit_push(&jit, a64_branch(B, -5));
	a64_jit_push(&jit, a64_mov(R0, R2));
	a64_jit_push(&jit, a64_ret());

	out = fopen("files/sum.bin", "w");
	fwrite(jit.code, sizeof(a64_t), jit.length, out);
	fclose(out);

	a64_jit_mkexec(&jit);

	function = (u64_t(*)(void))(uintptr_t)&(jit.code[rlen]);
	res = function();

	if (argc > 1 && strcmp(argv[1], "-v"))
		printf("result: %lu\n", (unsigned long)res);

	a64_jit_destroy(&jit);

	return res == 10 ? 0 : -1;
}