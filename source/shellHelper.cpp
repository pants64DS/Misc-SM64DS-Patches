#include "SM64DS_PI.h"

// see shellSpeedAndRotation.s
extern "C" Fix12i GetShellSpeedLimit(Fix12i limit, Player& player)
{
	if (player.wmClsn.sphere.floorResult.surfaceInfo.clps.isWater)
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