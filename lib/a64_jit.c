
#include "../headers/a64_jit.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#if defined(__APPLE__) && defined(__MACH__)
	#include <libkern/OSCacheControl.h>
#endif

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

void a64_jit_init(a64_jit_t *jit, size_t length)
{
	size_t page_size;
	size_t real_size;
	int prot;
	int flags;



	page_size = sysconf(_SC_PAGESIZE);
	real_size = (length * sizeof(a64_t)) / page_size + page_size;

	jit->length = 0;
	jit->real_size = real_size;

#if defined(__linux__)
	prot	= PROT_READ | PROT_WRITE | PROT_EXEC;
	flags	= MAP_PRIVATE | MAP_ANONYMOUS;
#elif defined(__APPLE__) && defined(__MACH__)
	prot	= PROT_READ | PROT_WRITE;
	flags	= MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT;
#endif

	jit->code = mmap(
		NULL,
		real_size,
		prot,
		flags,
		-1,
		0);

	if (jit->code == MAP_FAILED) {
		fprintf(stderr, A64_JIT_ERROR ": mmap failed (%s)\n", strerror(errno));
		exit(-1);
	}
#if defined(__linux__)
	jit->executable = true;
#elif defined(__APPLE__) && defined(__MACH__)
	jit->executable = false;
#endif
}

void a64_jit_mkexec(a64_jit_t *jit)
{
	if (jit->executable)
		return;
#if defined(__linux__)
	fprintf(stderr, A64_JIT_ERROR ": jit not allocated\n");
	exit(-1);
#elif defined(__APPLE__) && defined(__MACH__)
	if (mprotect(jit->code, jit->real_size, PROT_READ | PROT_EXEC) == -1) {
		fprintf(stderr, A64_JIT_ERROR ": mprotect failed (%s)\n", strerror(errno));
		munmap(jit->code, jit->real_size);
		exit(-1);
	}
	jit->executable = true;
	sys_icache_invalidate(jit->code, jit->length * sizeof(a64_t));
#endif
}

void a64_jit_destroy(a64_jit_t *jit)
{
	munmap(jit->code, jit->real_size);
}

void a64_jit_push(a64_jit_t *jit, a64_t ins)
{
	if ((jit->length + 1) * 4 >= jit->real_size) {
		fprintf(stderr, A64_JIT_ERROR ": no room for instruction\n");
	}
	jit->code[jit->length++] = ins;
}

size_t a64_jit_push_data(a64_jit_t *jit, void *data, size_t length)
{
	size_t count;

	count = length / 4 + ((length % 4 != 0) ? 1 : 0);
	if ((jit->length + count) * 4 >= jit->real_size) {
		fprintf(stderr, A64_JIT_ERROR ": no room for data\n");
	}
	memcpy(&jit->code[jit->length], data, length);
	jit->length += count;
	return count;
}