#include <stdio.h>
#include <string.h>
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
	u64_t res;
	u64_t (*function)(void);
	FILE *out;

	a64_jit_init(&jit, 30);

	a64_jit_push(&jit, a64_addi(R0, R0, 10));
	a64_jit_push(&jit, a64_addsi(R0, R0, 10));
	a64_jit_push(&jit, a64_subi(R0, R0, 10));
	a64_jit_push(&jit, a64_subsi(R0, R0, 10));
	a64_jit_push(&jit, a64_addiw(R0, R0, 10));
	a64_jit_push(&jit, a64_addsiw(R0, R0, 10));
	a64_jit_push(&jit, a64_subiw(R0, R0, 10));
	a64_jit_push(&jit, a64_subsiw(R0, R0, 10));
	#define LOGFUNC(name, opcode, negate)		\
	a64_jit_push(&jit, a64_ ## name (R0, R0, R1));	\
	a64_jit_push(&jit, a64_ ## name ## w (R0, R0, R1));
	LOGOPS(LOGFUNC)
	#undef LOGFUNC
	a64_jit_push(&jit, a64_ret());



	out = fopen("files/logop.bin", "w");
	a64_write_bin(out, &jit);
	fclose(out);

	a64_jit_mkexec(&jit);

	function = (u64_t (*)(void))(uintptr_t)jit.code;

	res = function();

	if (argc > 1 && strcmp(argv[1], "-v"))
		printf("result: 0x%lX, code start: %p\n", (unsigned long)res, (void*)jit.code);

	a64_jit_destroy(&jit);

	return 0;
}