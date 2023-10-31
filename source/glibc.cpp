#include "include/SM64DS_Common.h"

extern "C"
{
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
