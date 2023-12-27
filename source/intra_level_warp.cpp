#include "include/SM64DS_2.h"
// #include "MOM/source/MOM_IDs.h"

asm(R"(
nsub_020b0be4_ov_02:
	push  {r0-r3, r12, r14}
	mov   r0, r6
	mov   r1, r5
	bl    ShouldExitNormally
	cmp   r0, #0
	pop   {r0-r3, r12, r14}
	movne r0, #0
	bne   0x020b0be8
	b     0x020b0d50

func_020ca1b8 = 0x020ca1b8
func_0200d048 = 0x0200d048
)");

extern "C" void func_020ca1b8(Player& player, int param_2, int param_3, int param_4);
extern "C" void func_0200d048(Camera& camera);

uint8_t entranceID = 0;
uint8_t warpFrameCounter = 0;

constexpr uint8_t framesBeforeSpawn = 25;
constexpr uint8_t framesFromSpawnToSound = 25;
constexpr uint8_t framesBeforeSound = framesBeforeSpawn + framesFromSpawnToSound;

extern char unk_0209f2fc;
asm("unk_0209f2fc = 0x0209f2fc");

static bool IsActorNear(const Vector3& pos, uint16_t actorID)
{
	Actor* warpPipe = nullptr;
	while ((warpPipe = Actor::FindWithActorID(actorID, warpPipe)) && warpPipe->pos.Dist(pos) >= 300._f);
	return warpPipe;
}

static bool IsWarpPipeNear(const Vector3& pos)
{
	// return IsActorNear(pos, 298) || IsActorNear(pos, MOM_IDs::COLORED_PIPE);

	return IsActorNear(pos, 298);
}

extern "C" bool ShouldExitNormally(const Actor& exit, Player& player)
{
	if (exit.param1 >> 0x18 != LEVEL_ID) [[likely]]
		return true;

	unk_0209f2fc = 0;

	func_020ca1b8(player, 6, -1, 0);
	func_0200d048(*CAMERA);

	if (IsWarpPipeNear(exit.pos))
		Sound::UnkPlaySoundFunc(22);
	else
		Sound::UnkPlaySoundFunc(NEXT_LEVEL_ID == 13 ? 30 : 25); // play the painting sound when entering HMC?
	
	entranceID = exit.param1 >> 0x10;
	warpFrameCounter = framesBeforeSound;

	return false;
}

asm(R"(
repl_020e4d28_ov_02:
	sub   r13,r13,#0x18
	b     UpdateIntraLevelWarp

SpawnActorBase = 0x02042ffc
)");

extern "C" ActorBase* SpawnActorBase(unsigned actorID, ActorBase* parent, unsigned param1, int unk03);

extern "C" Player& UpdateIntraLevelWarp(Player& player)
{
	if (warpFrameCounter == 0 || (--warpFrameCounter != 0 && warpFrameCounter != framesFromSpawnToSound))
		return player;

	const LevelFile::Entrance& entrance = (*ENTRANCE_ARR_PTR)[entranceID];
	const Vector3 pos = entrance.pos;

	if (warpFrameCounter == 0)
	{
		if (IsWarpPipeNear(pos)) Sound::UnkPlaySoundFunc(22);

		return player;
	}

	const unsigned viewID = entrance.unk0e >> 3 & 0xf;
	const unsigned areaID = entrance.unk0e & 7;
	const unsigned entranceMode = entrance.unk0e >> 7 & 0xf;

	const int param1 = player.realChar | (player.param1 & 3) << 3 | player.playerID << 6 | entranceMode << 8;

	PLAYER_ARR[0] = static_cast<Player*>(Actor::Spawn(player.actorID, param1, pos, &entrance.rot, areaID, -1));

	if (player.eggPtrArr && player.eggPtrArr[0])
		player.eggPtrArr[0]->Destroy();

	player.KillAndTrackInDeathTable();

	CAMERA->Destroy();
	CAMERA = static_cast<Camera*>(SpawnActorBase(CAMERA->actorID, ROOT_ACTOR_BASE, viewID, 0));

	return player;
}

/*
extern "C" Actor* FindWarpPipe(unsigned warpPipeActorID, Actor* searchStart)
{
	static bool searchingVanillaPipes = true;
	if (!searchStart) searchingVanillaPipes = true;

	if (searchingVanillaPipes)
	{
		Actor* vanillaPipe = Actor::FindWithActorID(warpPipeActorID, searchStart);

		if (vanillaPipe)
			return vanillaPipe;
		else
		{
			searchingVanillaPipes = false;

			return Actor::FindWithActorID(MOM_IDs::COLORED_PIPE, nullptr);
		}
	}

	return Actor::FindWithActorID(MOM_IDs::COLORED_PIPE, searchStart);
}

asm(R"(
repl_020b0c84_ov_02 = FindWarpPipe
repl_020b0cb4_ov_02 = FindWarpPipe
)");
*/
