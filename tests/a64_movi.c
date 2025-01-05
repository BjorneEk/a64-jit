
#include "../headers/a64_jit.h"
#include "../headers/a64_ins.h"

static const u64_t expected = 69;


int main(void)
{
	a64_jit_t jit;
	u64_t res;
	u64_t (*function)(void);

	a64_jit_init(&jit, 10);
	jit.code[0] = a64_movi(R0, expected);
	jit.code[1] = a64_ret();

	a64_jit_mkexec(&jit);

	function = (u64_t (*)(void))(uintptr_t)jit.code;

	res = function();

	a64_jit_destroy(&jit);

	return res == expected ? 0 : -1;
}