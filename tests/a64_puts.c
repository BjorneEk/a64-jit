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
void putc_(int c) {putchar(c);}
void hello(void) {printf("Hello, Function\n");return;}
int main(int argc, const char *argv[])
{
	a64_jit_t jit;
	void (*function)(void);

	//char *str = "Hello, World!\n";
	//size_t str_p;
	size_t pc_p;
	FILE *out;
	void *pca = dlsym(RTLD_DEFAULT, "putchar");
	a64_jit_init(&jit, 30);
	//pc_p = a64_jit_push_data(&jit, pca, sizeof(&putc_));
	*(void(**)(void))(uintptr_t*)jit.code = hello;
	jit.length += 2;
	pc_p = 2;
	a64_jit_push(&jit, a64_adr(R22, -pc_p));
	a64_jit_push(&jit, a64_load_store(LOAD, IMM, SZX, 0, R23, R22, 0));
	//a64_jit_push(&jit, a64_moviw(R0, 'A'));
	a64_jit_push(&jit, a64_mov(R19, R28));
	a64_jit_push(&jit, a64_mov(R20, R29));
	a64_jit_push(&jit, a64_mov(R21, R30));

	a64_jit_push(&jit, a64_blr(R23));
	a64_jit_push(&jit, a64_mov(R28, R19));
	a64_jit_push(&jit, a64_mov(R29, R20));
	a64_jit_push(&jit, a64_mov(R30, R21));
	a64_jit_push(&jit, a64_ret());
	/*
	str_p = a64_jit_push_data(&jit, str, 15);
	pc_p = a64_jit_push_data(&jit, dlsym(RTLD_DEFAULT, "putchar"), sizeof(&putc_));
	a64_jit_push(&jit, a64_adr(R4, -pc_p));
	a64_jit_push(&jit, a64_adr(R5, -(pc_p + str_p + 1)));
	a64_jit_push(&jit, a64_ldr(LDRX, R6, R4, 0));

	a64_jit_push(&jit, a64_ldr(LDRB, R0, R5, 0));
	a64_jit_push(&jit, a64_cmpiw(R0, '\0'));

	a64_jit_push(&jit, a64_branch(EQ, 4));
	a64_jit_push(&jit, a64_blr(R6));
	a64_jit_push(&jit, a64_addi(R5, R5, 1));
	a64_jit_push(&jit, a64_branch(B, -5));
	a64_jit_push(&jit, a64_ret());
	*/



	printf("pca: 0x%p\n", pca);

	out = fopen("files/puts.bin", "w");
	fwrite(jit.code, sizeof(a64_t), jit.length, out);
	fclose(out);

	a64_jit_mkexec(&jit);

	function = (void(*)(void))(uintptr_t)&(jit.code[pc_p]);
	function();

	a64_jit_destroy(&jit);

	return 0;
}