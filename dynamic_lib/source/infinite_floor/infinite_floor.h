#ifndef INFINITE_FLOOR_INCLUDED
#define INFINITE_FLOOR_INCLUDED

#include "SM64DS_2.h"

struct InfiniteFloor : public Actor, public MeshColliderBase
{
	static constexpr uint16_t staticActorID = 562;
	static SpawnInfo spawnData;
	const CLPS& GetCLPS() const;

	virtual int InitResources() final override;
	virtual int CleanupResources() final override;
	virtual bool BeforeBehavior() final override;
	virtual bool BeforeRender() final override;
	virtual void GetSurfaceInfo(short triangleID, SurfaceInfo& res) final override;
	virtual void GetNormal(short triangleID, Vector3& res) final override;
	virtual void GetTriangleOrigin(short triangleID, Vector3& res) final override;
	virtual unsigned DetectClsn(RaycastGround& ray) final override;
	virtual unsigned DetectClsn(RaycastLine& ray) final override;
	virtual unsigned DetectClsn(SphereClsn& sphere) final override;
};

#endif