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
	a64_jit_push(&jit, a64_adc(R0, R0, R1));
	a64_jit_push(&jit, a64_adcw(R0, R0, R1));
	a64_jit_push(&jit, a64_adcs(R0, R0, R1));
	a64_jit_push(&jit, a64_adcsw(R0, R0, R1));
	a64_jit_push(&jit, a64_sbc(R0, R0, R1));
	a64_jit_push(&jit, a64_sbcw(R0, R0, R1));
	a64_jit_push(&jit, a64_sbcs(R0, R0, R1));
	a64_jit_push(&jit, a64_sbcsw(R0, R0, R1));
	a64_jit_push(&jit, a64_add(R0, R0, R1));
	a64_jit_push(&jit, a64_addw(R0, R0, R1));
	a64_jit_push(&jit, a64_adds(R0, R0, R1));
	a64_jit_push(&jit, a64_addsw(R0, R0, R1));
	a64_jit_push(&jit, a64_sub(R0, R0, R1));
	a64_jit_push(&jit, a64_subw(R0, R0, R1));
	a64_jit_push(&jit, a64_subs(R0, R0, R1));
	a64_jit_push(&jit, a64_subsw(R0, R0, R1));
	a64_jit_push(&jit, a64_cmp(R0, R1));
	a64_jit_push(&jit, a64_cmpw(R0, R1));
	a64_jit_push(&jit, a64_ret());



	out = fopen("files/add.bin", "w");
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