#include "include/SM64DS_Common.h"

static void CopyBackwards(char* dest, const char* src, size_t size)
{
	dest += size;
	src += size;

	while (size--) *--dest = *--src;
}

extern "C"
{
	[[gnu::section(".memmove")]]
	void* memmove(void* dest, const void* src, size_t size)
	{
		char* d = static_cast<char*>(dest);
		const char* s = static_cast<const char*>(src);

		if (d < s || d >= s + size)
			CpuCopy8(src, dest, size);
		else if (d > s)
			CopyBackwards(d, s, size);

		return dest;
	}

	[[gnu::section(".memcpy")]]
	void* memcpy(void* dest, const void* src, size_t size)
	{
		CpuCopy8(src, dest, size);

		return dest;
	}

	[[gnu::section(".memset")]]
	void* memset(void* dest, int val, size_t size)
	{
		CpuFill8(dest, val, size);

		return dest;
	}
}
