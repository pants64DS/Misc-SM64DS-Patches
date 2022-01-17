#ifndef SM64DS_CLSN_INCLUDED
#define SM64DS_CLSN_INCLUDED

#include "SM64DS_Common.h"

struct MovingCylinderClsn;
struct WithMeshClsn;
struct RaycastGround;
struct RaycastLine;
struct MeshCollider;
struct ClsnResult;
extern "C"
{
	extern MeshCollider* ACTIVE_MESH_CLSNS[0x18];
}

struct CLPS
{
	unsigned low;
	unsigned high;
	
	enum _TextureID
	{
		TX_NONE = 0x00,
		TX_PATH = 0x01,
		TX_GRASS = 0x02,
		TX_ROCK = 0x04,
		TX_WOOD = 0x05,
		TX_SNOW = 0x06,
		TX_ICE = 0x07,
		TX_SAND = 0x08,
		TX_FLOWERS = 0x09,
		TX_GRATE = 0x0c
	};
		
	enum _TractionID
	{
		TR_NORMAL = 0x0,
		TR_UNSLIPPABLE = 0x1,
		TR_UNSLIPPABLE_2 = 0x2,
		TR_SLIPPERY_SLOPE = 0x3,
		TR_SLIPPERY = 0x4,
		TR_SLIP_NO_WALL_JUMP = 0x5,
	};
			
	enum _CamBehavID
	{
		CA_NO_CHANGE = 0x0,
		CA_GO_BEHIND = 0x1,
		CA_ZOOM_OUT_GO_BEHIND = 0x2,
		CA_GO_BEHIND_3 = 0x3,
		CA_GO_BEHIND_4 = 0x4,
		CA_NORMAL = 0x6,
		CA_GO_BEHIND_7 = 0x7,
		CA_GO_BEHIND_8 = 0x8,
		CA_EIGHT_DIRECTIONS = 0x9,
		CA_NO_ROTATING = 0xa,
		CA_ZOOM_REALLY_FAR_IN = 0xb,
		CA_GO_BEHIND_C = 0xc,
		CA_GO_BEHIND_D = 0xd,
		CA_GO_BEHIND_E = 0xe,
		CA_GO_BEHIND_F = 0xf,
	};
			
	enum _BehaviorID
	{
		BH_NORMAL = 0x00,
		BH_LAVA = 0x01,
		BH_HANG_CEILING = 0x03,
		BH_DEATH = 0x04,
		BH_DEATH_2 = 0x05,
		BH_LOW_JUMPS = 0x06,
		BH_SLOW_QUICKSAND = 0x07,
		BH_SLOW_QUICKSAND_2 = 0x08,
		BH_INSTANT_QUICKSAND = 0x09,
		BH_HARD = 0x0e,
		BH_RACE_START = 0x0f,
		BH_RACE_END = 0x10,
		BH_VANISH_LUIGI_GRATE = 0x11,
		BH_WEIRD_TELEPORT = 0x12,
		BH_WIND_GUST = 0x13,
	};
	
	constexpr CLPS() : low(0xfc0), high(0xff) {}

	constexpr CLPS (
		unsigned texture, unsigned water, unsigned viewID, unsigned traction, unsigned camBehav,
		unsigned behav, unsigned camTrans, unsigned toxic, unsigned unk, unsigned windID
	):
		low(texture | water << 5 | viewID << 6 | traction << 12 | camBehav << 15 |
			behav << 19 | camTrans << 24 | toxic << 25 | unk << 26),
		high(windID)
	{}
	
	constexpr unsigned TextureID()       const { return low & 0x1f; }
	constexpr bool     IsWater()         const { return low & 0x20; }
	constexpr unsigned ViewID()          const { return low >> 6 & 0x3f; }
	constexpr unsigned TractionID()      const { return low >> 12 & 0x07; }
	constexpr unsigned CamBehavID()      const { return low >> 15 & 0x0f; }
	constexpr unsigned BehaviorID()      const { return low >> 19 & 0x1f; }
	constexpr bool     CanCamGoThrough() const { return low & 0x01000000; }
	constexpr bool     IsToxic()         const { return low & 0x02000000; }
	constexpr bool     IsCameraWall()    const { return low & 0x04000000; }
	constexpr unsigned Padding()         const { return low >> 27; }
	constexpr unsigned WindID()          const { return high & 0xff; }

	constexpr bool operator==(const CLPS&) const = default;
};
	
struct CLPS_Block
{
	char magic[4]; // = {'C', 'L', 'P', 'S'};
	uint16_t unk04;
	uint16_t num; // = Size;
	CLPS clpses[];
};

namespace LevelFile
{
	extern "C" CLPS_Block* CLPS_BLOCK_PTR;
}

template<int Size> struct FixedSizeCLPS_Block // flexible arrays cannot be static-initialized.
{
	char magic[4]; // = {'C', 'L', 'P', 'S'};
	uint16_t unk04;
	uint16_t num; // = Size;
	CLPS clpses[Size];
};

struct SurfaceInfo
{
	CLPS clps;
	Vector3 normal;
};

struct MeshCollider
{
	//vtable
	Actor* actor;
	unsigned actorUniqueID;
	Fix12i range;
	Fix12i rangeOffsetY;
	unsigned clsnID;
	//clsnActor is the mesh collider's actor for some reason
	//beforeClsnCallback is called if there was a collision the previous frame.
	void(*beforeClsnCallback)(MeshCollider& clsn, Actor* clsnActor, ClsnResult& wmClsnResult, Vector3& posToUpdate, Vector3_16* motionAngToUpdate, Vector3_16* angToUpdate);
	void(*afterClsnCallback)(MeshCollider& clsn, Actor* clsnActor, Actor* otherActor);
	char* clsnFile;
	CLPS_Block* clpsBlock;
	Vector3 unkVec28;
	Vector3 unkVec34;
	unsigned unk40;
	Fix12i unk44;
	unsigned unk48;
	unsigned unk4c;
	
	MeshCollider();
	virtual ~MeshCollider();
	virtual void Virtual08(); // both known implementations are bx lr
	virtual void GetSurfaceInfo(short triangleID, SurfaceInfo& res);
	virtual void GetNormal(short triangleID, Vector3& res);

	[[gnu::always_inline]]
	auto GetNormal(const short& triangleID)
	{
		return Vector3::Proxy([this, &triangleID]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			this->GetNormal(triangleID, res);
		});
	}
	
	bool Disable();
	bool Enable(Actor* actor = nullptr);
	bool IsEnabled();

	void SetFile(char* clsnFile, CLPS_Block& clps);
};

struct MovingMeshCollider : public MeshCollider
{
	unsigned unk50;
	Matrix4x3 newTransform;
	Matrix4x3 invMat4x3_084;
	Matrix4x3 scMat4x3_0b4;
	Matrix4x3 invMat4x3_0e4;
	unsigned unk114;
	unsigned unk118;
	unsigned unk11c;
	unsigned unk120;
	unsigned unk124;
	unsigned unk128;
	unsigned unk12c;
	unsigned unk130;
	Matrix4x3 ledgeMat;
	unsigned unk164;
	Matrix4x3 clsnInvMat;
	Matrix4x3 sc2InvMat4x3_198;
	
	MovingMeshCollider();
	virtual ~MovingMeshCollider();
	virtual void Virtual08();
	virtual void GetSurfaceInfo(short triangleID, SurfaceInfo& res);
	virtual void GetNormal(short triangleID, Vector3& res);
	using MeshCollider::GetNormal;
	
	static char* LoadFile(SharedFilePtr& filePtr);
	void SetFile(char* clsnFile, const Matrix4x3& mat, Fix12i scale, short angleY, CLPS_Block& clps);
	void Transform(const Matrix4x3& mat, short rotY);
};

struct CylinderClsn
{
	enum Flags1 : unsigned
	{
		F1_DISABLED = 1 << 0, //at least against the player.
		F1_PLAYER_CAN_GO_THROUGH = 1 << 1,
		
		F1_BEING_SPIT = 1 << 17
	};
	enum HitFlags : unsigned
	{
		HIT_BY_CHAR_MOVE = 1 << 1,
		HIT_BY_CHAR_PROJ = 1 << 2,
		HIT_BY_CHAR_BODY = 1 << 3,
		HIT_BY_MEGA_CHAR = 1 << 4,
		HIT_BY_SPIN_OR_GROUND_POUND = 1 << 5,
		HIT_BY_PUNCH = 1 << 6,
		HIT_BY_KICK = 1 << 7,
		HIT_BY_BREAKDANCE = 1 << 8,
		HIT_BY_SLIDE_KICK = 1 << 9,
		HIT_BY_DIVE = 1 << 10,
		
		
		HIT_BY_EGG = 1 << 13,
		
		
		
		HIT_BY_REGURG_GOOMBA = 1 << 17,
		HIT_BY_FIRE = 1 << 18,
		
		HIT_BY_SHELL = 1 << 20,
		
		HIT_BY_PLAYER = 1 << 27
	};

	Fix12i radius;
	Fix12i height;
	Vector3 pushback;
	unsigned flags1;
	unsigned vulnerableFlags;
	unsigned hitFlags;
	unsigned otherObjID;
	CylinderClsn* prev;
	CylinderClsn* next;
	
	CylinderClsn();
	virtual ~CylinderClsn();
	virtual Vector3& GetPos() = 0;
	virtual unsigned GetOwnerID() = 0;

	CylinderClsn(const CylinderClsn&) = delete;
	CylinderClsn(CylinderClsn&&) = delete;
	CylinderClsn& operator=(const CylinderClsn&) = delete;
	CylinderClsn& operator=(CylinderClsn&&) = delete;

	void Init(Fix12i radius, Fix12i height, unsigned flags, unsigned vulnFlags);
	
	void Update();
	void Clear();

	static CylinderClsn* last;
};

static_assert(sizeof(CylinderClsn) == 0x30);

struct MovingCylinderClsn : public CylinderClsn
{
	Actor* owner;
	
	MovingCylinderClsn();
	virtual ~MovingCylinderClsn();
	virtual Vector3& GetPos() override;
	virtual unsigned GetOwnerID() override;
	
	void Init(Actor* actor, Fix12i radius, Fix12i height, unsigned flags, unsigned vulnFlags);
};

struct CylinderClsnWithPos : public MovingCylinderClsn
{
	Vector3 pos;
	
	CylinderClsnWithPos();
	virtual ~CylinderClsnWithPos();
	virtual Vector3& GetPos() override;
	
	void Init(Actor* actor, const Vector3& pos, Fix12i radius, Fix12i height, unsigned flags, unsigned vulnFlags); // calls SetPosRelativeToActor
	void SetPosRelativeToActor(const Vector3& pos); // pos is transformed by the object's Y angle
};

struct ClsnResult
{
	unsigned* vTable;
	SurfaceInfo surfaceInfo;
	short triangleID; //0xffff if in air
	short clsnID; //not constant per object, 0x18 if in air (only 24 mesh colliders can be active at a time)
	unsigned objID;
	Actor* obj;
	MovingMeshCollider* meshClsn;
	
	ClsnResult();
	~ClsnResult();
	ClsnResult& operator=(const ClsnResult&);
};

struct BgCh //That's the internal name, and I didn't know what else to call it since I already used WithMeshClsn
{	
	enum Flags : unsigned
	{
		DETECT_FRONT_FACES = 1 << 0, //guess
		DETECT_WATER = 1 << 1,
		DETECT_BACK_FACES = 1 << 3, //guess
	};
	// <value on construction>, notes
	unsigned* vTable; // 0x02099264
	unsigned flags; // 1, probably a flag field
	unsigned objID; // -1, id of game object
	Actor* objPtr; // 0, pointer to game object
	ClsnResult result;
	
	void SetFlag_8();
	void SetFlag_2();
	void ClearFlag_1();
};

struct RaycastGround : public BgCh
{
	Vector3 pos; // 0x38
	Fix12i clsnPosY; // 0x44
	bool hadCollision;
	Fix12i unk4c; // 0x1f4000
	
	RaycastGround();
	~RaycastGround();
	void SetObjAndPos(const Vector3& pos, Actor* obj);
	bool DetectClsn();
};

struct RaycastLine : public BgCh
{
	struct Line
	{
		Vector3 pos0; // 0x38
		Vector3 pos1; // 0x44

		Line& Set(const Vector3& pos0, const Vector3& pos1);
	}
	line;

	bool hadCollision; // 0x50
	Vector3 clsnPos; // set to pos1 before collision
	Fix12i length; // of line
	unsigned* vTable3;
	Vector3 average;
	Fix12i halfLength;
	
	RaycastLine();
	~RaycastLine();
	void SetObjAndLine(const Vector3& pos0, const Vector3& pos1, Actor* obj);
	bool DetectClsn();
	/* WARNING: This function definition is wrong, it seems to be static, void and takes two args
	Vector3 GetClsnPos();
	*/
};

struct SphereClsn : public BgCh
{
	enum ResultFlags
	{
		COLLISION_EXISTS = 1 << 0,
		
		ON_GROUND = 1 << 2,
		ON_WALL = 1 << 3,
		
		MOVING_UP = 1 << 5
	};
	
	SphereClsn();
	~SphereClsn();
	void SetObjAndSphere(const Vector3& pos, Fix12i radius, Actor* obj);
	bool DetectClsn();
	ClsnResult& SetFloorResult(const ClsnResult& result);
	
	unsigned* sphVTable;
	Vector3 pos;
	Fix12i radius;
	Vector3 pushback;
	Vector3 pushback0;
	Vector3 pushback1;
	unsigned resultFlags;   // 0x70
	ClsnResult floorResult; // 0x74
	ClsnResult wallResult;  // 0x9c
	ClsnResult unkResult;   // 0xc4
	Fix12i unk0ec;
	unsigned unk0f0;
	unsigned unk0f4;
	unsigned unk0f8;
	Fix12i unk0fc;
	Fix12i unk100;
	Fix12i unk104;
	Fix12i unk108;
};

struct WithMeshClsn
{
	enum Flags : int
	{
		ON_GROUND = 1 << 4,
		JUST_HIT_GROUND = 1 << 5,
		JUST_LEFT_GROUND = 1 << 6,
		LIMITED_MOVEMENT = 1 << 7, //does not set speed.y to 0 if hit ground. The historical reason for the name is that not doing so disables a Goomba's horz. movement
		
		NO_UPDATE_POS_Y = 1 << 12, //assuming this is a state flag as well.
		NO_UPDATE_POS = 1 << 13 //this is a state flag. It will not work if you set it.
	};
	
	unsigned* vTable;
	unsigned unk04;
	unsigned unk08;
	unsigned unk0c;
	unsigned flags;
	Actor* actor;
	Fix12i radius;
	Fix12i vertOffset;
	SphereClsn sphere;
	Vector3_16* motionDirPtr; //0x12c
	Vector3_16* angPtr;
	RaycastLine line;
	unsigned unk1ac;
	Fix12i unk1b0;
	unsigned unk1b4;
	Fix12i unk1b8;
	
	WithMeshClsn();
	~WithMeshClsn();
	int  ShouldUpdatePosY() const;
	int  ShouldUpdatePos() const;
	int  IsOnWall() const;
	void ClearLimMovFlag();
	void SetLimMovFlag();
	int  IsOnGround() const;
	void ClearGroundFlag();
	void SetGroundFlag();
	int  JustHitGround() const;
	void ClearJustHitGroundFlag();
	void ClearAllGroundFlags();
	void SetFlag_2();
	void Unk_0203589c();
	void Init(Actor* owner, Fix12i radius, Fix12i vertOffset, Vector3_16* motionDirPtr, Vector3_16* angPtr);
	
	void UpdateContinuous();
	void UpdateExtraContinous();
	void UpdateContinuousNoLava();
	void UpdateDiscreteNoLava();
	void UpdateDiscreteNoLava_2();
};


#endif