#include "headers/SM64DS_2.h"

extern "C" bool wallSlideOrNot()
{
	static short angle;

	if ((unsigned)(PLAYER_ARR[0]->currState) != Player::ST_WALL_JUMP ||
		PLAYER_ARR[0]->ang.y != angle ||
		(PLAYER_ARR[0]->param1 & 0xff) == 0)
	{
		angle = PLAYER_ARR[0]->ang.y;
		return true;
	}
	
	PLAYER_ARR[0]->ang.y += 0x8000;
	return false;
}

void nsub_020c1dbc()
{
	asm
	(
		"push	{r1-r12, r14}		\n\t"
		"bl		wallSlideOrNot		\n\t"
		"pop 	{r1-r12, r14}		\n\t"
		"cmp	r0, #0				\n\t"
		"beq	0x020c1e28			\n\t"
		"bne	0x020c1dc8			\n\t"
	);
}