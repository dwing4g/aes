#include <stdlib.h>
#include <jni.h>
#include "aes.h"

typedef struct // 244+8 = 252; must be aligned with 16 bytes
{
	aes_encrypt_ctx ctx; // 4 * 60 + 4
	void* base;
} test_aes_ctx;

int main()
{
	test_aes_ctx* aes;
	int i;
	unsigned char key[16] = {0};
	unsigned char buf[16] = {0};

	void* base = malloc(sizeof(test_aes_ctx) + 15);
	if(!base) return -1;
	aes = (test_aes_ctx*)(((uintptr_t)base + 15) & ~(uintptr_t)15);
	aes->base = base;

	aes_init();
	aes_encrypt_key128(key, &aes->ctx);

	for(i = 0; i < 10000000; ++i)
		aes_encrypt(buf, buf, &aes->ctx);

	free(aes->base);
	return 0;
}
