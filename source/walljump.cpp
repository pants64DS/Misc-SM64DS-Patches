#include "SM64DS_PI.h"

extern "C" bool ShouldNotWallSlide(Player& player)
{
	static short angle = 0;

	if (player.param1 & 1 &&  // if the player is Luigi or Yoshi
		player.currState == &Player::ST_WALL_JUMP &&
		AngleDiff(player.ang.y, angle) < 22.5_deg)
	{
		player.ang.y += 180_deg;

		return true;
	}

	angle = player.ang.y;

	return false;
}

asm(R"(
repl_020c1dbc:
	mov     r0, r5
	b 		ShouldNotWallSlide
)");
