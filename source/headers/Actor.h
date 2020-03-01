#ifndef SM64DS_ACTOR_INCLUDED
#define SM64DS_ACTOR_INCLUDED

#include "SM64DS_Common.h"


struct Player;
struct CylinderClsn;
struct ShadowModel;


struct ActorBase		//internal name: fBase
{
	enum AliveState
	{
		ALIVE = 1,
		DEAD = 2
	};

	struct SceneNode
	{
		SceneNode* parent;
		SceneNode* firstChild;
		SceneNode* prevSibling;
		SceneNode* nextSibling;
		ActorBase* actor;

		void Reset();
		SceneNode* ResetSceneNode(); //Calls Reset
	};

	struct ProcessingListNode
	{
		ProcessingListNode* prev;
		ProcessingListNode* next;
		ActorBase* actor;
		uint16_t priority;
		uint16_t priorityCopy;

		//void RemoveFromGlobal();	//at 0x020440E8 and 0x02044104, removes if at 0x020A4B68
	};

	enum VirtualFuncSuccess
	{
		VS_FAIL_BEFORE = 0,
		VS_FAIL = 1,
		VS_SUCCESS = 2,
		VS_RETURN_MINUS_1 = 3
	};

	void* operator new(size_t count); //actor bases have their own heap
	inline void operator delete(void* ptr) { FreeHeapAllocation(ptr, *(unsigned**)0x020a0eac); }

	virtual int  InitResources();
	virtual bool BeforeInitResources();
	virtual void AfterInitResources(unsigned vfSuccess);
	virtual int  CleanupResources();
	virtual bool BeforeCleanupResources();
	virtual void AfterCleanupResources(unsigned vfSuccess);
	virtual int  Behavior();
	virtual bool BeforeBehavior();
	virtual void AfterBehavior(unsigned vfSuccess);
	virtual int  Render();
	virtual bool BeforeRender();
	virtual void AfterRender(unsigned vfSuccess);
	virtual void Virtual30();
	virtual bool Virtual34(unsigned arg0, unsigned arg1);
	virtual bool Virtual38(unsigned arg0, unsigned arg1);
	virtual bool Virtual3c();
	virtual ~ActorBase();

	void Destroy();

	//vTable;
	unsigned uniqueID;
	int      param1;
	uint16_t actorID;
	uint8_t aliveState;
	bool shouldBeKilled;
	uint8_t unk10;
	uint8_t unk11;
	uint8_t unk12;
	uint8_t unk13;
	SceneNode sceneNode;
	ProcessingListNode behavNode;
	ProcessingListNode renderNode;
	unsigned unk48;
	unsigned unk4c;
};



struct ActorDerived : public ActorBase		//internal name: dBase
{

	virtual void AfterInitResources(unsigned vfSuccess) override;		//Destroys Actor (ActorBase::Destroy) on vfunc failure, then calls ActorBase::AfterInitResources(unsigned)
	virtual ~ActorDerived() override;

};





struct Actor : public ActorBase				//internal name: dActor			
{

	enum Flags : int
	{
		NO_BEHAVIOR_IF_OFF_SCREEN = 1 << 0,
		NO_RENDER_IF_OFF_SCREEN = 1 << 1, //offscreen can mean too far away.
		OFF_SCREEN = 1 << 3,
		OFF_SHADOW_RANGE = 1 << 4,
		WRONG_AREA = 1 << 5,
		GOING_TO_YOSHI_MOUTH = 1 << 17,
		IN_YOSHI_MOUTH = 1 << 18,
		CAN_SQUISH = 1 << 25,
		AIMABLE_BY_EGG = 1 << 28
	};

	struct ListNode
	{
		ListNode* prev;
		ListNode* next;
		Actor* actor;
	};

	enum OnYoshiEatReturnVal : int
	{
		YE_DONT_EAT = 0,
		YE_KEEP_IN_MOUTH = 1,
		YE_SPIT_AND_GET_HURT = 2,
		YE_SPIT = 3,
		YE_SWALLOW = 4,
		YE_GAIN_FIRE_BREATH = 5,
		YE_KEEP_AND_CAN_MAKE_EGG = 6,
	};

	ListNode listNode;
	Vector3_Q12 pos;
	Vector3_Q12 prevPos;
	Vector3_Q12 camSpacePos;
	Vector3_Q12 scale;
	Vector3_16 ang;
	Vector3_16 motionAng;
	Fix12i horzSpeed;
	Fix12i vertAccel;
	Fix12i termVel;
	Vector3_Q12 speed;
	unsigned flags;
	Fix12i rangeOffsetY;
	Fix12i rangeAsr3;
	Fix12i drawDistAsr3;
	Fix12i unkc0Asr3;
	unsigned unkc4;
	unsigned unkc8;
	char areaID; //it is signed
	uint8_t unkcd;
	short deathTableID;
	Player* eater;

	Actor();

	virtual int  InitResources();
	virtual bool BeforeInitResources() override;
	virtual void AfterInitResources(unsigned vfSuccess) override;
	virtual int  CleanupResources();
	virtual bool BeforeCleanupResources() override;
	virtual void AfterCleanupResources(unsigned vfSuccess) override;
	virtual int  Behavior();
	virtual bool BeforeBehavior() override;
	virtual void AfterBehavior(unsigned vfSuccess) override;
	virtual int  Render();
	virtual bool BeforeRender() override;
	virtual void AfterRender(unsigned vfSuccess) override;
	virtual ~Actor();
	virtual unsigned OnYoshiTryEat();
	virtual void OnTurnIntoEgg(Player& player);
	virtual bool Virtual50();
	virtual void OnGroundPounded(Actor& groundPounder);
	virtual void OnAttacked1(Actor& attacker);
	virtual void OnAttacked2(Actor& attacker);
	virtual void OnKicked(Actor& kicker);
	virtual void OnPushed(Actor& pusher);
	virtual void OnHitByCannonBlastedChar(Actor& blaster);
	virtual void OnHitByMegaChar(Player& megaChar);
	virtual void Virtual70(Actor& attacker);
	virtual Fix12i OnAimedAtWithEgg();
	virtual Vector3_Q12 OnAimedAtWithEggReturnVec();

	bool IsTooFarAwayFromPlayer(Fix12i tooFar);
	void MakeVanishLuigiWork(CylinderClsn& cylClsn);
	void SpawnSoundObj(unsigned soundObjParam);

	void KillAndTrackInDeathTable();
	void UntrackInDeathTable();
	bool GetBitInDeathTable();

	void BigLandingDust(bool doRaycast);
	void LandingDust(bool doRaycast);
	void DisappearPoofDustAt(const Vector3_Q12& vec);
	void SmallPoofDust();
	void PoofDustAt(const Vector3_Q12& vec);
	void PoofDust(); //calls the two above function

	void UntrackStar();
	Actor* UntrackAndSpawnStar(unsigned trackStarID, unsigned starID, const Vector3_Q12& spawnPos, unsigned howToSpawnStar);
	unsigned TrackStar(unsigned starID, unsigned starType); //starType=1: silver star, 2: star //returns star ID or 0xff if starID != STAR_ID

	void Earthquake(const Vector3_Q12& source, Fix12i magnitude);
	short ReflectAngle(Fix12i normalX, Fix12i normalZ, short angToReflect);

	bool BumpedUnderneathByPlayer(Player& player); //assumes there is a collision in the first place
	bool JumpedOnByPlayer(CylinderClsn& cylClsn, Player& player);
	void Unk_0201061c(Player& player, unsigned numCoins, unsigned coinType);

	Fix12i DistToCPlayer();
	Player* ClosestPlayer();
	short HorzAngleToCPlayer();
	short HorzAngleToCPlayerOrAng(short ang);
	Player* FarthestPlayer();
	Fix12i DistToFPlayer();

	void DropShadowScaleXYZ(ShadowModel& arg1, const Matrix4x3& arg2, Fix12i scaleX, Fix12i scaleY, Fix12i scaleZ, unsigned transparency);
	void DropShadowRadHeight(ShadowModel& arg1, const Matrix4x3& arg2, Fix12i radius, Fix12i depth, unsigned transparency); //last argument is on a scale of 1 to 16.

	void UpdatePos(CylinderClsn* clsn); //Applies motion direction, vertical acceleration, and terminal velocity.
	void UpdatePosWithOnlySpeed(CylinderClsn* clsn);//IMPORTANT!: When spawning a Super Mushroom, make sure to already have the model loaded before the player goes super!
	//You cannot afford to spawn a Super Mushroom if there are 0 uses of the model's SharedFilePtr and the player already went super.
	//If you do, particle color glitches will happen!

	Actor* SpawnNumber(const Vector3_Q12& pos, unsigned number, bool isRed, unsigned arg3, unsigned arg4);
	static Actor* Spawn(unsigned actorID, unsigned param1, const Vector3_Q12& pos, const Vector3_16* rot, int areaID, int deathTableID);
	Actor* Next(); //next in the linked list. Returns the 1st object if given a nullptr. Returns a nullptr if given the last actor
	static Actor* FindWithID(unsigned id);
	static Actor* FindWithActorID(unsigned actorID, Actor* searchStart); //searchStart is not included.

};




#endif // SM64DS_ACTOR_INCLUDED
