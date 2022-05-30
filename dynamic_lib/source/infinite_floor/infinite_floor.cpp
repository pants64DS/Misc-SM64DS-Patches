#include "infinite_floor.h"

SpawnInfo InfiniteFloor::spawnData =
{
	+[]() -> ActorBase* { return new InfiniteFloor; },
	staticActorID, // behavPriority
	staticActorID, // renderPriority
	0,      // flags
	0_f,    // rangeOffsetY
	-1._f,  // range
	0_f,    // drawDist
	0_f     // unkc0
};

const CLPS& InfiniteFloor::GetCLPS() const
{
	if (LevelFile::CLPS_BLOCK_PTR)
		return (*LevelFile::CLPS_BLOCK_PTR)[param1];

	static constexpr CLPS defaultCLPS = {};
	return defaultCLPS;
}

int InfiniteFloor::InitResources   () { Enable();  return 1; }
int InfiniteFloor::CleanupResources() { Disable(); return 1; }

bool InfiniteFloor::BeforeBehavior() { return false; }
bool InfiniteFloor::BeforeRender  () { return false; }

void InfiniteFloor::GetSurfaceInfo(short triangleID, SurfaceInfo& res)
{
	GetNormal(triangleID, res.normal);
	res.clps = GetCLPS();
}

void InfiniteFloor::GetNormal(short triangleID, Vector3& res)
{
	res = Vector3::Temp(0._f, 1._f, 0._f);
}

void InfiniteFloor::GetTriangleOrigin(short triangleID, Vector3& res)
{
	res = this->pos;
}

bool InfiniteFloor::DetectClsn(RaycastGround& ray)
{
	if (ray.ShouldPassThrough(GetCLPS(), false)) return false;

	if (ray.pos.y < this->pos.y) return false;

	GetSurfaceInfo(0, ray.result.surfaceInfo);
	ray.result.triangleID = 0;
	ray.clsnPosY = this->pos.y;
	ray.hadCollision = true;

	return true;
}

bool InfiniteFloor::DetectClsn(RaycastLine& ray)
{
	if (ray.ShouldPassThrough(GetCLPS(), false)) return false;

	if (ray.line.pos0.y < this->pos.y) return false;
	if (ray.line.pos1.y > this->pos.y) return false;

	const Vector3 v = ray.line.pos1 - ray.line.pos0;

	if (Abs(v.y) < 1._f)
		ray.clsnPos = ray.line.pos0 + ray.line.pos1 >> 1;
	else
	{
		const Fix12i t = (this->pos.y - ray.line.pos0.y) / v.y;

		ray.clsnPos.x = ray.line.pos0.x + v.x * t;
		ray.clsnPos.y = this->pos.y;
		ray.clsnPos.z = ray.line.pos0.z + v.z * t;
	}

	GetSurfaceInfo(0, ray.result.surfaceInfo);
	ray.result.triangleID = 0;
	ray.hadCollision = true;

	return true;
}

bool InfiniteFloor::DetectClsn(SphereClsn& sphere)
{
	if (sphere.ShouldPassThrough(GetCLPS(), false)) return false;

	if (!(sphere.pos.y - this->pos.y < sphere.radius)) return false;
	if (!(sphere.pos.y - this->pos.y > -5._f)) return false;

	GetSurfaceInfo(0, sphere.result.surfaceInfo);
	sphere.result.triangleID = 0;

	const Fix12i sphereBottomY = sphere.pos.y - sphere.radius;
	sphere.pushback1 = Vector3::Temp(0._f, this->pos.y - sphereBottomY, 0._f);
	sphere.resultFlags |= SphereClsn::COLLISION_EXISTS | SphereClsn::ON_GROUND;

	GetSurfaceInfo(0, sphere.floorResult.surfaceInfo);
	sphere.floorResult.triangleID = 0;

	sphere.storedNormal = Vector3::Temp(0._f, 1._f, 0._f);

	return true;
}

void init()
{
	ACTOR_SPAWN_TABLE[InfiniteFloor::staticActorID] = reinterpret_cast<unsigned>(&InfiniteFloor::spawnData);
	OBJ_TO_ACTOR_ID_TABLE[InfiniteFloor::staticActorID] = InfiniteFloor::staticActorID;
}

void cleanup() {}
