#include "include/SM64DS_2.h"

// see shellSpeedAndRotation.s
extern "C" Fix12i GetShellSpeedLimit(Fix12i limit, Player& player)
{
	if (player.wmClsn.sphere.floorResult.clps.IsWater())
	{
		if (player.horzSpeed < 0x18'000_f)
			player.horzSpeed = 0x18'000_f;

		return limit;
	}
	else
	{
		if (player.horzSpeed > 0x10'000_f)
			player.horzSpeed = 0x10'000_f;

		return limit >> 2;
	}
}