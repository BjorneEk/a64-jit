#include <stdio.h>
#include <string.h>
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"


int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	u64_t res;
	u64_t (*function)(void);

	a64_jit_init(&jit, 10);
	jit.code[0] = a64_nop();
	jit.code[1] = a64_nop();
	jit.code[2] = a64_nop();
	jit.code[3] = a64_adr(R0, -3);
	jit.code[4] = a64_ret();

	a64_jit_mkexec(&jit);

	function = (u64_t (*)(void))(uintptr_t)jit.code;

	res = function();

	if (argc > 1 && strcmp(argv[1], "-v"))
		printf("result: 0x%lX, code start: %p\n", (unsigned long)res, (void*)jit.code);

	a64_jit_destroy(&jit);

	return res == ((u64_t)jit.code) ? 0 : -1;
}