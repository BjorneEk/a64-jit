#include <stdio.h>
#include <dlfcn.h>
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

u64_t ret68(void)
{
	//puts("in func");
	return 68;
}

void mov_fp(a64_jit_t *jit, a64_reg_t rdst, a64_reg_t rinter, uintptr_t fp)
{
	a64_jit_push(jit, a64_movi(rdst, 0));
	a64_jit_push(jit, a64_movi(rinter, (0xFFFFL << 0) & fp));
	a64_jit_push(jit, a64_add(rdst, rdst, rinter));
	a64_jit_push(jit, a64_movi(rinter, (0xFFFFL << 16) & fp));
	a64_jit_push(jit, a64_add(rdst, rdst, rinter));
	a64_jit_push(jit, a64_movi(rinter, (0xFFFFL << 32) & fp));
	a64_jit_push(jit, a64_add(rdst, rdst, rinter));
	a64_jit_push(jit, a64_movi(rinter, (0xFFFFL << 48) & fp));
	a64_jit_push(jit, a64_add(rdst, rdst, rinter));
}

int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	u64_t (*function)(void);
	u64_t res;

	//char *str = "Hello, World!\n";
	//size_t str_p;
	FILE *out;
	a64_jit_init(&jit, 10);
	a64_jit_push(&jit, a64_movi(R2, 0));
	mov_fp(&jit, R2, R1, (uintptr_t)&ret68);
	a64_jit_push(&jit, a64_movi(R0, 1));
	a64_jit_push(&jit, a64_mov(R4, R30));
	a64_jit_push(&jit, a64_blr(R2));
	a64_jit_push(&jit, a64_mov(R30, R4));
	a64_jit_push(&jit, a64_addi(R0, R0, 1));
	a64_jit_push(&jit, a64_ret());

	out = fopen("files/fcall.bin", "w");
	fwrite(jit.code, sizeof(a64_t), jit.length, out);
	fclose(out);

	a64_jit_mkexec(&jit);
	//u64_t calculated = ((0xFFFFL << 0) & (uintptr_t)&ret68) + ((0xFFFFL << 16) & (uintptr_t)&ret68) + ((0xFFFFL << 32) & (uintptr_t)&ret68) + ((0xFFFFL << 48) & (uintptr_t)&ret68);
	//printf("fp: %p, calulated~: %p\n", (void*)(uintptr_t)ret68, (void*)(uintptr_t)(void(*)(void))(uintptr_t)calculated);
	function = (u64_t(*)(void))(uintptr_t)(jit.code);
	res = function();

	//if (argc > 1 && strcmp(argv[1], "-v"))
	printf("result: %lu\n", (unsigned long)res);

	a64_jit_destroy(&jit);

	return res == 69 ? 0 : -1;
}