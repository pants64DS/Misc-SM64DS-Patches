#include <cstring>

asm("MainLoop = 0x020197b8");
extern "C" [[noreturn]] void MainLoop();

constinit char a[10] = {};
constinit char b[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

constinit unsigned s = sizeof(a);

void copy()
{
	std::memcpy(a, b, s);
}

void move()
{
	std::memmove(a + 5, a, 5);
	std::memmove(b, b + 5, 5);
}

void set()
{
	std::memset(a + 1, 0xff, 8);
}

void repl_02007040()
{
	asm("mov r11, r11");

	copy();
	move();
	set();

	MainLoop();
}
