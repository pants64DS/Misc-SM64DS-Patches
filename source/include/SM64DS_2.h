#ifndef SM64DS_2_H_INCLUDED
#define SM64DS_2_H_INCLUDED

#include "SM64DS_Common.h"
#include "NDSCore.h"
#include "Actor.h"
#include "Model.h"
#include "Collision.h"
#include "Level.h"
#include "Message.h"
#include "Particle.h"
#include "Sound.h"
#include "Save.h"
#include "Input.h"
#include "Memory.h"

#define OBJ_TO_ACTOR_ID_TABLE ((volatile uint16_t*)0x02004b00)
#define ACTOR_SPAWN_TABLE     ((volatile unsigned*)0x02006590)

/*
unsigned vtable[] =
	{
		(unsigned)&InitResources,
		0x02011268,
		0x02011244,
		(unsigned)&CleanupResources,
		0x02011220,
		0x02011214,
		(unsigned)&Behavior,
		0x02010fd4,
		0x02010fc8,
		(unsigned)&Func24,
		0x02010f78,
		0x02010f6c,
		0x02043ac0,
		0x0204357c,
		0x0204349c,
		0x02043494,
		(unsigned)&Destructor,
		(unsigned)&DestructAndFree,
		void(*OnYoshiTryEat)(Actor*),
		0x02010154,
		0x0201014c,
		void(*OnGroundPounded)(Actor*, Actor* attacker),
		void(*OnAttacked1)(Actor*, Actor* attacker),
		void(*OnAttacked2)(Actor*, Actor* attacker),
		void(*OnKicked)(Actor*, Actor* attacker),
		0x02010138,
		void(*OnHitByCannonBlastedChar)(Actor*, Player*),
		void(*OnHitByMegaChar)(Actor*, Player*),
		0x0201012c,
		0x02010124,
		0x020100dc,
		(unsigned)&Kill
	};
*/



struct CapIcon
{
	enum Flags
	{
		ACTIVE = 1 << 1
	};
	
	unsigned* vTable;
	Actor* actor;
	unsigned objID;
	unsigned unk0c;
	unsigned unk10;
	unsigned flags;
	unsigned unk18;
};

struct Enemy : public Actor
{
	enum CoinType
	{
		CN_NONE = 0,
		CN_YELLOW = 1,
		CN_RED = 2,
		CN_BLUE = 3
	};
	
	enum DefeatMethod
	{
		DF_NOT = 0,
		DF_SQUASHED = 1,
		DF_PUNCHED = 2,
		DF_KICKED = 3,
		DF_BURNED = 4,
		DF_DIVED = 5,
		DF_UNK_6 = 6,
		DF_HIT_REGURG = 7,
		DF_GIANT_CHAR = 8 //this is definitely the end of the list.
	};
	
	enum InvCharKillFlags
	{
		IK_SPAWN_COINS = 1 << 0,
		IK_KILL = 1 << 1
	};
	
	enum UpdateYoshiEatReturn
	{
		UY_NOT = 0,
		UY_EATING = 1,
		UY_IN_MOUTH = 2,
		UY_SPITTING = 3
	};
	
	Vector3 floorNormal;
	Vector3 wallNormal;
	Vector3 unkNormal; //ceiling?
	unsigned unk0f8;
	unsigned unk0fc;
	uint16_t stateTimer;
	uint16_t deathTimer;
	uint16_t spitTimer;
	bool isAtCliff;
	bool isBeingSpit;
	uint8_t coinType;
	uint8_t unk109;
	uint8_t numCoinsMinus1;
	uint8_t unk10b;
	unsigned defeatMethod;
	
	Enemy();
	virtual ~Enemy();
	
	unsigned UpdateKillByInvincibleChar(WithMeshClsn& wmClsn, ModelAnim& rigMdl, unsigned flags); //returns 0 with 0 side effects if N/A.
	void KillByInvincibleChar(const Vector3_16& rotation, Player& player);
	void SpawnMegaCharParticles(Actor& attacker, char* arg2);
	bool SpawnParticlesIfHitOtherObj(CylinderClsn& cylClsn); //returns whether there was a collision with an object that isn't a coin
	unsigned UpdateYoshiEat(WithMeshClsn& wmClsn);
	//returns whether the angle was redirected
	//if hitting wall, reflect angle; if at cliff, set angle to the opposite one.
	bool AngleAwayFromWallOrCliff(WithMeshClsn& wmClsn, short& ang);
	bool UpdateDeath(WithMeshClsn& wmClsn); //returns whether the object is actually dying and the death function returned true
	bool IsGoingOffCliff(WithMeshClsn& wmClsn, Fix12i arg2, int arg3, unsigned arg4, unsigned arg5, Fix12i stepToleranceY);
	void KillByAttack(Actor& other, WithMeshClsn& wmc);
	void SpawnCoin();
	void UpdateWMClsn(WithMeshClsn& wmClsn, unsigned arg2);
	
	
};

struct CapEnemy : public Enemy
{
	uint8_t capParam;
	bool hasNotSpawned;
	char spawnCapFlag;
	char capID;
	Model capModel;
	CapIcon capIcon;
	
	CapEnemy();
	virtual ~CapEnemy();
	
	void Unk_02005d94();
	bool DestroyIfCapNotNeeded(); //returns true if the cap is needed (player is different character than cap or 0x0209f2d8 has a 0)
	int GetCapState(); //returns 2 if obj+0x111 = 0, else 0 if dead or capID == player character, else 1
	CapEnemy* RespawnIfHasCap(); //nullptr if failed
	bool GetCapEatenOffIt(const Vector3& offset); //returns whether there was a cap and the cap is not the original object
	Actor* ReleaseCap(const Vector3& offset); //returns the ptr to the cap if cap was released, ptr to original obj else.
	void RenderCapModel(const Vector3* scale);
	void UpdateCapPos(const Vector3& offsetPos, const Vector3_16& rot);
	Actor* AddCap(unsigned capID);
	void UnloadCapModel();
	
};

struct Platform : public Actor
{
	Model model;
	MovingMeshCollider clsn;
	Matrix4x3 clsnNextMat;
	unsigned unk31c;
	
	Platform();
	virtual ~Platform();
	virtual void Kill();
	
	void KillByMegaChar(Player& player);
	bool UpdateKillByMegaChar(short rotSpeedX, short rotSpeedY, short rotSpeedZ, Fix12i speed); //true if killed by mega char
	void UpdateClsnPosAndRot(); //make sure the mesh collider is at 0x124 first! Also, call this after updating the model's matrix
	void UpdateModelPosAndRotY(); //make sure the model is at 0x0d4 first!

	// enables collision if in range, and disables it otherwise
	// if clsnRange        == 0._f, Actor::rangeAsr3 << 3 is used instead
	// if clsnRangeOffsetY == 0._f, Actor::rangeOffsetY   is used instead
	bool IsClsnInRange(Fix12i clsnRange = 0._f, Fix12i clsnRangeOffsetY = 0._f);

	// enables collision if on screen and in range, or if on screen and clsnRange == 0._f
	// disables the collision otherwise
	// if clsnRangeOffsetY == 0._f, Actor::rangeOffsetY is used instead
	bool IsClsnInRangeOnScreen(Fix12i clsnRange = 0._f, Fix12i clsnRangeOffsetY = 0._f);
};

static_assert(sizeof(MovingMeshCollider) > 0xe4);

struct CameraDef
{
	static const unsigned SIZE = 0x28;
	
	int unk00; //something to do with going behind the player...
	int unk04;
	int camVertAngIsh;
	unsigned unk0c;
	int vertOffset;
	int offsetIsh; //???
	unsigned jumpWithPlayerIsh;
	int dist0; //???
	int dist1;
	uint16_t fovOver2;
	uint16_t unk26;
};


//vtable at 0x02092720
struct View : public ActorDerived		//internal name: dView; done
{
	Matrix4x3 camMat;					//View Matrix to use when rendering

	virtual int Render() override;		//Sets the global view matrix to camMat and calculates the global inverse view matrix
	virtual ~View();
};


//vtable at 0x0208E730
struct Clipper
{
	Vector3 unk04;
	Vector3 unk10;
	Vector3 unk1c;
	Vector3 unk28;
	unsigned unk34;
	unsigned unk38;
	unsigned unk3c;
	unsigned unk40;
	unsigned unk44;
	unsigned unk48;
	Fix12i aspectRatio;					//Aspect ratio
	unsigned unk50;
	unsigned unk54;
	uint16_t unk58;

	void Func_020150E8();
	void Func_02015560();
	void Func_0201559C();	//noargs
	void Func_020156DC();

	Clipper();
	virtual ~Clipper();

};




// vtable at 0x02086F84, ctor at 0x0200E444
struct Camera : public View // internal name: dCamera
{
	static constexpr unsigned cameraDefTable = 0x02086FCC;
	static constexpr unsigned stateTableBase = 0x0209B008;
	/*
	0: Default
	1: Bottom camera, close (swimming on surface)
	2: Bottom camera, far (diving)
	3: Fly (feather, cannon shoot)
	4: Top view (owl)
	6: Air-driven (wind, Balloon Mario)
	7: Climbing
	8: Fixed back sliding
	9: First person
	B: Enter cannon
	C: Cannon view
	D: Talking
	E: Door enter
	F: Painting zoom
	11: Front zoom (character introduction)
	*/

	enum Flags
	{
		UNDERWATER = 1 << 0,
		ZOOMED_OUT = 1 << 2,
		BOSS_TALK = 1 << 3,
		ROTATING_LEFT = 1 << 5,
		ROTATING_RIGHT = 1 << 6,
		ARROWS_ALLOWED = 1 << 12,
		TALK = 1 << 14,
		ZOOM_OUT_FROM_TALK = 1 << 15,
		ZOOMED_IN = 1 << 19
	};

	struct State // Executes view specific camera behaviour
	{
		bool (Camera::* OnUpdate)();            // Nested call by Camera::Behaviour()
		bool (Camera::* OnPlayerChangeState)(); // Nested call by Player::ChangeState()
	};
	
	Vector3 lookAt;
	Vector3 pos;            // 0x8C
	Vector3 ownerPos;       // 0x98
	Vector3 lookAtOffset;   // An offset from ownerPos to lookAt
	Vector3 savedLookAt;    // Saved to at talk
	Vector3 savedPos;       // Saved to at talk
	Vector3 unk0c8;         // Player's front lookAt?
	Vector3 unk0d4;         // Player's front pos?
	Vector3 unk0e0;         // Raycast result save (when the player becomes invisible to the camera)
	Vector3 unk0ec;         // Raycast result save (when the player becomes invisible to the camera)
	Fix12i aspectRatio;     // Aspect ratio, default = 1.33 (4:3)
	unsigned unk0fc;        // Clipper related (near+far)
	unsigned unk100;        // Clipper related
	unsigned unk104;        // Clipper related
	unsigned unk108;        // Clipper related
	uint8_t viewportLeft;   // Viewport x for left border
	uint8_t viewportBottom; // Viewport y for bottom border
	uint8_t viewportRight;  // Viewport x for right border
	uint8_t viewportTop;    // Viewport y for top border
	Actor* owner;           // The player stalked by the camera
	Actor* unk114;          // Set at special camera scene? Set to King Bomb-Omb for example
	Actor* unk118;          // Another unknown actor
	unsigned unk11c;        // Related to unk118, set to 0xDFE60 at 0x02009F3C
	Vector3 unk120;         // unk118's or (if unk118 == 0) unk114's position vector
	Fix12i unk12c;          // Distance to unk114?
	Fix12i unk130;          // Linear camera movement interpolator (only for unk114?) that (when entering a different camera view like at the top of BoB) interpolates from 0x0 to 0x100 and backwards when leaving. As a result, it also indicates whether the owner is in a special camera scene. unk114 is linked later during interpolation.
	Fix12i unk134;          // Ground pound camera jitter offset. Starts at 0xC000 and vibrates back and forth with alternating signs until it reaches 0.
	State* currState;       // Pointer to the current setting behaviour, which in turn sets the CameraDef's
	CameraDef* defaultCamDef;
	CameraDef* currCamDef;
	LevelFile::View* currView;
	Vector3* pausePos;
	unsigned unk14c;
	unsigned unk150;
	unsigned flags;
	unsigned unk158;
	unsigned unk15c;
	unsigned unk160;
	unsigned unk164;
	unsigned unk168;
	unsigned unk16c;
	unsigned unk170;
	unsigned unk174;
	short zAngle;
	Vector3_16 angle; // probably not really except for y
	short eightDirAngleY;
	short eightDirStartAngle;
	short eightDirDeltaAngle;
	short unk186;
	uint16_t unk188;
	short zShakeAngleOscillator;
	short zShakeMaxAngle;
	uint16_t unk18e;
	unsigned unk190;
	uint16_t unk194;
	uint16_t unk196;
	unsigned unk198;
	unsigned unk19c;
	unsigned unk1a0;
	uint16_t unk1a4;
	uint16_t unk1a6;

	Camera();
	virtual ~Camera();

	virtual int  InitResources() override;
	virtual int  CleanupResources() override;
	virtual int  Behavior() override;
	virtual int  Render() override;
	virtual void Virtual30() override;

	void SaveCameraStateBeforeTalk(); // Saves the current camera state
	int CallKuppaScriptInstruction(char* instruction, short minFrame, short maxFrame);

	//Func_0200D954
	//Func_0200D8C8
	//All funcs between Camera() and ~Camera() should belong to this object, but I couldn't prove it since they're never really called.
};

struct Area
{
	TextureTransformer* texSRT;
	bool showing;
	uint8_t unk5;
	uint16_t unk6;
	unsigned unk8;
};

extern "C" void ChangeArea(int areaID);

//vtable at 0x0210C2C8, ctor at 0x020FE154
struct HUD : public ActorDerived		//internal name: dMeter, ActorID = 0x14e
{
	unsigned unk50;
	unsigned unk54;
	unsigned unk58;
	unsigned unk5c;
	unsigned unk60;
	unsigned unk64;
	short meterYOffset;			//y-offset, counts downwards to 0x19, if 0x19 > yOffset or yOffset > 0x7FFF, then it is immediately set to 0x19
	uint16_t unk6a;				//unknown counter
	uint16_t unk6c;				//unknown counter
	uint16_t lifeXOffset;		//life counter xPos (default: 0x10)
	uint16_t starXOffset;		//star counter xPos (default: 0xF0)
	uint8_t unk72;				//unknown
	uint8_t meterState;			//health meter state (0-6: 0=stopAnim, 1=update, 2=locked?, 4=disappear), read and updated at 0x020FD218 before the branch table
	char currNumInDecimal[3];
	uint8_t unk77;
	unsigned unk78;

	virtual int	InitResources() override;
	virtual int CleanupResources() override;
	virtual int Behavior() override;
	virtual int Render() override;
	virtual void Virtual30() override;

	HUD();
	virtual ~HUD();

};



//vtable at 0x2092680
struct Scene : public ActorDerived		//internal name: dScene
{

};

//vtable at 0x02091528
struct SceneBoot : public Scene			//internal name: dScBoot
{
	unsigned unk50;
	unsigned unk54;
};

//vtable at 0x020943C4, ctor at 0x020352B4
struct SceneMB : public Scene			//internal name: dScMB
{
	//size 0x68
	//ColorFader?
};


//vtable at 020921c0, constructor at 0202e088
struct Stage : public Scene				//internal name: dScStage, ActorID = 0x003
{
	
	Particle::SysTracker particleSysTracker;
	Model model;
	Area areas[0x08];
	MeshCollider clsn;
	uint8_t fogTable[0x20];
	bool enableFog;
	uint8_t fogInfo;
	uint16_t fogOffset;
	uint16_t fogColor;
	uint16_t unk992;
	uint8_t unk994[0x20];
	unsigned unk9b4;
	unsigned unk9b8;
	Model* skyBox;
	unsigned unk9c0;
	unsigned unk9c4;
};

//vtable at 0210c1c0, constructor at 020fb8bc, dtor at 0x020F975C
//Code address to init 256x256 map: 020fb568
//Code address to init 128x128 map: 020fb694
/*
Conditions for a 256x256 map:
In Peach's Castle but not the castle graunds or the backyard OR
In Big Boo's Haunt OR
In Big Boo Battle (the map, not the fight) OR
In a test stage
*/
struct Minimap : public ActorDerived //ActorID = 0x14f
{
	enum ArrowType
	{
		AR_NONE = 0,
		AR_DONT_ROTATE_WITH_MINIMAP = 1,
		AR_ROTATE_WITH_MINIMAP = 2
	};
	
	Matrix2x2 transform;
	unsigned unk060;
	unsigned unk064;
	unsigned unk068;
	unsigned unk06c;
	unsigned unk070;
	unsigned unk074;
	unsigned unk078;
	unsigned unk07c;
	unsigned unk080;
	unsigned unk084;
	unsigned unk088;
	unsigned unk08c;
	unsigned unk090;
	unsigned unk094;
	unsigned unk098;
	unsigned unk09c;
	unsigned unk0a0;
	unsigned unk0a4;
	unsigned unk0a8;
	unsigned unk0ac;
	unsigned unk0b0;
	unsigned unk0b4;
	unsigned unk0b8;
	unsigned unk0bc;
	unsigned unk0c0;
	unsigned unk0c4;
	unsigned unk0c8;
	unsigned unk0cc;
	unsigned unk0d0;
	unsigned unk0d4;
	unsigned unk0d8;
	unsigned unk0dc;
	unsigned unk0e0;
	unsigned unk0e4;
	unsigned unk0e8;
	unsigned unk0ec;
	unsigned unk0f0;
	unsigned unk0f4;
	unsigned unk0f8;
	unsigned unk0fc;
	unsigned unk100;
	unsigned unk104;
	unsigned unk108;
	unsigned unk10c;
	unsigned unk110;
	unsigned unk114;
	unsigned unk118;
	unsigned unk11c;
	unsigned unk120;
	unsigned unk124;
	unsigned unk128;
	unsigned unk12c;
	unsigned unk130;
	unsigned unk134;
	unsigned unk138;
	unsigned unk13c;
	unsigned unk140;
	unsigned unk144;
	unsigned unk148;
	unsigned unk14c;
	unsigned unk150;
	unsigned unk154;
	unsigned unk158;
	unsigned unk15c;
	unsigned unk160;
	unsigned unk164;
	unsigned unk168;
	unsigned unk16c;
	unsigned unk170;
	unsigned unk174;
	unsigned unk178;
	unsigned unk17c;
	unsigned unk180;
	unsigned unk184;
	unsigned unk188;
	unsigned unk18c;
	unsigned unk190;
	unsigned unk194;
	unsigned unk198;
	unsigned unk19c;
	unsigned unk1a0;
	unsigned unk1a4;
	unsigned unk1a8;
	unsigned unk1ac;
	unsigned unk1b0;
	unsigned unk1b4;
	unsigned unk1b8;
	unsigned unk1bc;
	unsigned unk1c0;
	unsigned unk1c4;
	unsigned unk1c8;
	unsigned unk1cc;
	unsigned unk1d0;
	unsigned unk1d4;
	unsigned unk1d8;
	unsigned unk1dc;
	unsigned unk1e0;
	unsigned unk1e4;
	unsigned unk1e8;
	unsigned unk1ec;
	unsigned unk1f0;
	Vector3 center;
	Matrix2x2 arrowMat;
	unsigned unk210;
	Fix12i targetInvScale;
	Fix12i invScale;
	short angle;
	short unk21e;
	unsigned unk220;
	unsigned unk224;
	unsigned unk228;
	unsigned unk22c;
	unsigned unk230;
	unsigned unk234;
	unsigned unk238;
	unsigned unk23c;
	unsigned unk240;
	unsigned unk244;
	unsigned unk248;
	unsigned unk24c;
	uint8_t unk250;
	uint8_t arrowType;
	uint8_t unk252;
	uint8_t unk253;
	uint8_t unk254; // some counter
	uint8_t unk255;
	uint8_t unk256;
	uint8_t unk257;
};

struct LaunchStar;

//allocating constructor: 020e6c0c, vtable: 0210a83c
struct Player : public Actor
{
	static constexpr uint16_t staticActorID = 0xbf;

	enum Characters
	{
		CH_MARIO,
		CH_LUIGI,
		CH_WARIO,
		CH_YOSHI
	};
	
	struct State
	{
		bool(Player::* init)();
		bool(Player::* main)();
		bool(Player::* cleanup)();
	}
	static ST_LEDGE_GRAB,
	       ST_CEILING_GRATE,
	       ST_YOSHI_POWER, // tongue, spitting, throwing egg, breathing fire
	       ST_SWALLOW,
	       ST_HURT,
	       ST_HURT_WATER,
	       ST_ELECTROCUTE,
	       ST_BURN_FIRE,
	       ST_BURN_LAVA,
	       ST_DEAD_HIT,
	       ST_DEAD_PIT,
	       ST_WALK,
	       ST_WAIT,
	       ST_GRABBED,
	       ST_TURN_AROUND,
	       ST_JUMP,
	       ST_FALL,
	       ST_THROWN,
	       ST_SIDE_FLIP,
	       ST_SLIDE_KICK_RECOVER,
	       ST_FLY,
	       ST_NO_CONTROL, // includes caps
	       ST_OWL,
	       ST_WIND_CARRY,
	       ST_BALLOON,
	       ST_TELEPORT,
	       ST_CANNON,
	       ST_SQUISH,
	       ST_SHELL,
	       ST_STOMACH_SLIDE,
	       ST_BUTT_SLIDE,
	       ST_DIZZY_STARS,
	       ST_HOLD_LIGHT,
	       ST_BONK,
	       ST_HOLD_HEAVY,
	       ST_WALL_SLIDE,
	       ST_WALL_JUMP,
	       ST_SLOPE_JUMP,
	       ST_STUCK_IN_GROUND,
	       ST_LAND,
	       ST_ON_WALL,
	       ST_SPIN,
	       ST_TALK,
	       ST_CRAZED_CRATE,
	       ST_LEVEL_ENTER,
	       ST_CROUCH,
	       ST_CRAWL,
	       ST_BACK_FLIP,
	       ST_LONG_JUMP,
	       ST_PUNCH_KICK,
	       ST_GROUND_POUND,
	       ST_DIVE,
	       ST_THROW,
	       ST_BOWSER_SPIN,
	       ST_SWEEP,
	       ST_SLIDE_KICK,
	       ST_FIRST_PERSON,
	       ST_SWIM,
	       ST_WATER_JUMP,
	       ST_METAL_WATER_GROUND,
	       ST_METAL_WATER_WATER,
	       ST_CLIMB,
	       ST_HEADSTAND,
	       ST_POLE_JUMP,
	       ST_HEADSTAND_JUMP;


	enum TalkStates
	{
		TK_NOT = -1,
		TK_START = 0,
		TK_TALKING = 1, //+0x6e3 == anything but 3, 5, or 7
		TK_UNK2 = 2, //+0x6e3 == 3
		TK_UNK3 = 3  //+0x6e3 == 5 or 7
	};
	
	enum Flags2
	{
		
		
		F2_CAMERA_ZOOM_IN = 1 << 2,
		F2_TELEPORT = 1 << 3,
		
		
		
		F2_RESET_POSITION = 1 << 7,
		
		
		F2_EXIT_LEVEL_IF_DEAD = 1 << 10,
		F2_NO_CONTROL = 1 << 11,
		F2_START_FLOWER_POWER = 1 << 12
	};
	
	unsigned unk0d4;
	unsigned unk0d8;
	ModelAnim2* bodyModels[5]; //the fifth one is the Metal Wario model
	ModelAnim balloonModel;
	Model* headModels[4]; //Yoshi's is a ModelAnim
	Model* headNoCapModels[4]; //Yoshi's is the mouth-is-full model
	ModelAnim wings;
	unsigned unk1d8;
	TextureSequence texSeq1dc;
	TextureSequence texSeq1f0;
	TextureSequence texSeq204;
	TextureSequence texSeq218;
	MaterialChanger matChg22c;
	MaterialChanger matChg240;
	TextureSequence texSeq254;
	TextureSequence texSeq268;
	char* unk27c[4];
	char* unk28c[4];
	char* unk29c[4];
	ShadowModel shadow;
	CylinderClsnWithPos cylClsn;
	CylinderClsnWithPos cylClsn2;
	Actor* shellPtr;
	Actor* actorInHands;
	unsigned unk35c;
	Actor* actorInMouth;
	unsigned unk364;
	ActorBase* speaker;
	unsigned unk36c;
	State* currState;
	State* prevState;
	State* nextState;
	unsigned unk37c;
	WithMeshClsn wmClsn;
	Vector3 unk53c;
	Vector3 unk540; //mirrors the player's position?
	unsigned unk554;
	unsigned unk558;
	unsigned unk55c;
	unsigned unk560;
	unsigned unk564;
	unsigned unk568;
	unsigned unk56c;
	unsigned unk570;
	unsigned unk574;
	char* unk578;
	char* unk57c;
	unsigned unk580;
	unsigned unk584;
	Actor** eggPtrArr;
	unsigned unk58c;
	unsigned unk590;
	unsigned unk594;
	unsigned unk598;
	unsigned unk59c;
	unsigned unk5a0;
	unsigned unk5a4;
	unsigned unk5a8;
	unsigned unk5ac;
	unsigned unk5b0;
	unsigned unk5b4;
	unsigned unk5b8;
	Matrix4x3 unkMat5bc;
	Matrix4x3 unkMat5ec;
	unsigned unk61c;
	unsigned unk620;
	unsigned playLongUniqueID;
	unsigned unk628;
	unsigned unk62c;
	unsigned unk630;
	unsigned unk634;
	unsigned unk638;
	unsigned animID;
	unsigned unk640;
	Fix12i floorY;
	unsigned unk648;
	unsigned unk64c;
	unsigned unk650;
	unsigned unk654;
	unsigned floorTracID;
	unsigned floorCamBehavID;
	unsigned floorViewID;
	unsigned floorBehavID;
	unsigned unk668;
	unsigned floorTexID;
	unsigned floorWindID;
	unsigned unk674;
	unsigned unk678;
	unsigned unk67c;
	unsigned unk680;
	Fix12i jumpPeakHeight; // 0x684
	union { unsigned msgID; Fix12i yPosOnPole; /* zero at the bottom of the pole */ };
	unsigned unk68c;
	unsigned unk690;
	unsigned unk694;
	unsigned unk698;
	short spinningAngularVelY; // used for at least turning on poles and spinning Bowser
	uint16_t unk69e;
	uint16_t visibilityCounter; // the player is visible when this is even (except when the player is electrocuted the second bit is checked instead)
	uint16_t unk6a2;
	unsigned stateTimer; // sleep, run charge, etc.
	unsigned unk6a8;
	uint16_t unk6ac;
	uint16_t featherCapTimeRemaining; // 0x6AE
	unsigned unk6b0;
	unsigned unk6b4;
	unsigned unk6b8;
	uint16_t unk6bc;
	uint16_t powerupTimer;
	unsigned unk6c0;
	unsigned unk6c4;
	unsigned unk6c8;
	uint16_t unk6cc;
	uint16_t flags2;
	uint16_t unk6d0;
	short inputAngle;
	unsigned unk6d4;
	uint8_t playerID; //always 0 in single player mode
	uint8_t realChar; // probably
	uint8_t unk6da;
	uint8_t renderedChar;
	uint8_t prevHatChar; // 0x6DC
	uint8_t currHatChar; // 0x6DD
	bool isInAir;
	uint8_t unk6df;
	uint8_t unk6e0;
	uint8_t currJumpNumber; // 0x6E1: 0 - first, 1 - second, 2 - triple jump, more?
	uint8_t unk6e2;
	uint8_t stateState; // 0x6E3: the current state of the current state. How meta.
	uint8_t unk6e4;
	uint8_t canFlutterJump;
	uint8_t unk6e6;
	uint8_t unk6e7;
	uint8_t unk6e8;
	uint8_t currClsnState; // 0x06E9: 0 - not colliding, 1 - standing on ground, 2 - colliding with wall in water, 3 - colliding with wall on land 
	uint16_t unk6ea;
	unsigned unk6ec;
	unsigned unk6f0;
	uint8_t unk6f4;
	bool opacity;
	uint8_t unk6f6;
	uint8_t unk6f7;
	bool isFireYoshi;
	bool isMetalWario;
	bool hasMetalModel;
	bool isVanishLuigi;
	unsigned unk6fc;
	uint16_t unk700;
	uint8_t unk702;
	bool isMega;
	uint8_t unk704;
	uint8_t unk705;
	bool isUnderwater;
	uint8_t unk707;
	uint8_t unk708;
	uint8_t unk709;
	uint16_t unk70a;
	unsigned unk70c;
	uint16_t unk710;
	uint8_t isInAirIsh; // 0x712
	bool isTangible;
	uint8_t unk714;
	uint8_t unk715;
	uint8_t isIntangibleToMesh;
	uint8_t unk717;
	unsigned unk718;
	unsigned unk71c;
	unsigned unk720;
	unsigned unk724;
	unsigned unk728;
	unsigned unk72c;
	unsigned unk730;
	unsigned unk734;
	unsigned unk738;
	uint16_t toonStateAndFlag; //8 possible states, 0x8000 is the invincible-and-can't-collect-caps flag
	uint16_t unk73e;
	Fix12i toonIntensity;
	Vector3 unk744;
	unsigned unk750;
	unsigned unk754;
	unsigned unk758;
	uint16_t unk75c;
	short spineAngleOffsY; // is added to bodyModels[GetBodyModelID(param1 & 0xff, false)]->data.bones[8].rot.y
	short spineAngleOffsZ; // is added to bodyModels[GetBodyModelID(param1 & 0xff, false)]->data.bones[8].rot.z
	uint8_t lsState0Timer; // 0x762
	uint8_t launchState;   // 0x763
	LaunchStar* lsPtr;     // 0x764
	
	static SharedFilePtr* ANIM_PTRS[0x308];

	Player();

	virtual int  InitResources() override;
	virtual int  CleanupResources() override;
	virtual int  Behavior() override;
	virtual int  Render() override;
	virtual void Virtual30() override;
	virtual ~Player() override;
	virtual unsigned OnYoshiTryEat() override;

	//implemented in LaunchStar.cpp
	bool LS_Init();
	bool LS_Behavior();
	bool LS_Cleanup();
	
	void IncMegaKillCount();
	void SetNewHatCharacter(unsigned character, unsigned arg1, bool makeSfx);
	void SetRealCharacter(unsigned character);
	void TurnOffToonShading(unsigned character);
	
	bool Unk_020bea94();
	unsigned GetBodyModelID(unsigned character, bool checkMetalStateInsteadOfWhetherUsingModel) const;
	void SetAnim(unsigned animID, int flags, Fix12i animSpeed, unsigned startFrame);
	void UpdateAnim();
	bool ShowMessage(ActorBase& speaker, unsigned msgIndex, const Vector3& lookAt, unsigned arg3, unsigned arg4);
	bool StartTalk(ActorBase& speaker, bool noButtonNeeded); //true iff the talk actually started.
	int GetTalkState();
	bool IsOnShell(); //if not on shell, reset shell ptr
	void Burn();
	void Shock(unsigned damage);
	void RegisterEggCoinCount(unsigned numCoins, bool includeSilverStar, bool includeBlueCoin);
	//speed is multiplied by constant at 0x020ff128+charID*2 and divided by 50 (? could be 25, could be 100).
	void Hurt(const Vector3& source, unsigned damage, Fix12i speed, unsigned arg4, unsigned presetHurt, unsigned spawnOuchParticles);
	void Heal(int health);
	void Bounce(Fix12i bounceInitVel);
	bool ChangeState(Player::State& state);
	int CallKuppaScriptInstruction(char* instruction, short minFrame, short maxFrame);

	// TKWSC specific
	bool NoAnimChange_Init();
	bool NoAnimChange_Main();
	bool NoAnimChange_Cleanup();

	void ChangeArea(int newAreaID)
	{
		areaID = newAreaID;
		::ChangeArea(newAreaID);
	}

	bool IsWarping() const
	{
		return currState == &ST_NO_CONTROL && stateState == 6;
	}
};

static_assert(sizeof(Player) == 0x768, "sizeof(Player) is incorrect!");

namespace Event
{
	void ClearBit(unsigned bit);
	void SetBit(unsigned bit);
	int  GetBit(unsigned bit);
}



enum class ttcClock : char
{
	SLOW = 0,
	FAST = 1,
	RANDOM = 2,
	STOP = 3
};

//used for keeping track of dead objects across level parts (e.g. THI big and small mountains)
struct ActorDeathTable
{
	std::byte deadObjs[64]; //technically 512 booleans
};


struct Number : public Actor
{
	static constexpr uint16_t staticActorID = 0x14a;

	Model model;
	TextureSequence textureSequence;
	unsigned unkActorUniqueID;
	Vector3  spawnPos;
	Fix12i   unk148;
	uint16_t unk14c;
	uint8_t  numTimesBounced;
	uint8_t  unk14f;
};
static_assert(sizeof(Number) == 0x150, "sizeof(Number) is incorrect!");

struct PowerStar : public Enemy
{
	static constexpr uint16_t staticActorID = 0xb2;

	CylinderClsnWithPos cylClsn;
	WithMeshClsn wmClsn;
	ModelAnim modelAnim1;
	ModelAnim modelAnim2;
	ShadowModel shadowModel;
	unsigned unk3fc;
	unsigned unk400;
	unsigned unk404;
	unsigned unk408;
	unsigned unk40c;
	unsigned unk410;
	unsigned unk414;
	unsigned unk418;
	unsigned unk41c;
	unsigned unk420;
	unsigned unk424;
	unsigned unk428;
	unsigned unk42c;
	unsigned unk430;
	unsigned unk434;
	unsigned unk438;
	unsigned unk43c;
	unsigned unk440;
	unsigned unk444;
	unsigned unk448;
	unsigned unk44c;
	unsigned unk450;
	unsigned unk454;
	unsigned unk458;
	unsigned unk45c;
	Vector3 unkVec460;
	Vector3 unkVec46c;
	unsigned unk478;
	unsigned unk47c;
	unsigned unk480;
	unsigned unk484;
	unsigned unk488;
	unsigned unk48c;
	unsigned unk490;
	uint16_t unk494;
	short spawnFrameCounter;
	unsigned unk498;
	unsigned unk49c;
	unsigned unk4a0;
	unsigned unk4a4;
	unsigned unk4a8;
	unsigned unk4ac;
	unsigned unk4b0;
	unsigned unk4b4;
	unsigned unk4b8;
	unsigned unk4bc;
	unsigned unk4c0;
};
static_assert(sizeof(PowerStar) == 0x4c4, "sizeof(PowerStar) is incorrect!");

struct StarMarker : public Actor
{
	static constexpr uint16_t staticActorID = 0xb4;

	CylinderClsnWithPos cylClsn;
	Model model;
	ShadowModel shadowModel;
	unsigned unk18c;
	unsigned unk190;
	unsigned unk194;
	unsigned unk198;
	unsigned unk19c;
	unsigned unk1a0;
	unsigned unk1a4;
	unsigned unk1a8;
	unsigned unk1ac;
	unsigned unk1b0;
	unsigned unk1b4;
	unsigned unk1b8;
	unsigned unk1bc;
	unsigned unk1c0;
	unsigned unk1c4;
	unsigned unk1c8;
	unsigned unk1cc;
	unsigned unk1d0;
	unsigned unk1d4;
	uint8_t  unk1d8;
	char     starID;
	uint8_t  unk1da;
	uint8_t  unk1db;
};
static_assert(sizeof(StarMarker) == 0x1dc, "sizeof(StarMarker) is incorrect!");

struct ArchiveInfo
{
	char* archive;
	char* heap;
	uint16_t firstFileID;
	uint16_t firstNotFileID; //1 + lastFileID
	char* name;
	char* fileName;
};

struct Archive
{
	char magic[4];
	Archive* next;
	Archive* prev; //if this is first, it points to the ROM.
	unsigned unk0c;
	unsigned unk10;
	unsigned unk14;
	unsigned unk18;
	unsigned unk1c;
	unsigned unk20;
	unsigned unk24;
	unsigned unk28;
	unsigned unk2c;
	unsigned unk30;
	unsigned unk34;
	unsigned unk38;
	unsigned unk3c; //a function
	unsigned unk40; //a function
	unsigned unk44; //a function
	unsigned unk48;
	unsigned unk4c;
	char* header;
	char* FAT;
	char* fileBlock;
	unsigned unk5c;
	char data[];
};

struct ROM_Info
{
	char magic[4]; //"rom\0"
	Archive* firstArchive;
};

using EnemyDeathFunc = bool(Enemy::*)(WithMeshClsn& wmClsn);

//File ID 0x8zzz is file from archive 0 with id zzz
//020189f0: overlay 0 file ID to file ID and store
//020189f8: 00 10 b0 e1: movs r0, r1 (checks if file address is nullptr)
//02018a00: 0d 01 00 1b: blne 0x02018e3c
extern "C"
{
	extern char** DL_PTR_ARR_PTR;
	
	extern char LEVEL_PART_TABLE[NUM_LEVELS];
	extern char SUBLEVEL_LEVEL_TABLE[NUM_LEVELS];
	
	extern int ACTOR_BANK_OVL_MAP[7][7];
	extern int LEVEL_OVL_MAP[NUM_LEVELS];
	
	extern MsgGenTextFunc MSG_GEN_TEXT_FUNCS[3];
	
	extern char ACTOR_BANK_SETTINGS[7];
	
	extern Vector3 CAM_SPACE_CAM_POS_ASR_3; //constant <0.0, 64.0, -112.0>
	
	extern ArchiveInfo ARCHIVE_INFOS[13];
	
	extern int NEXT_UNIQUE_ACTOR_ID;
	
	extern Matrix4x3 VIEW_MATRIX_ASR_3;
	extern Matrix4x3 INV_VIEW_MATRIX_ASR_3;
	extern Vector3_16* ROT_AT_SPAWN;
	extern Vector3* POS_AT_SPAWN;
	extern Actor::ListNode* FIRST_ACTOR_LIST_NODE;
	
	extern bool IMMUNE_TO_DAMAGE;
	
	extern ttcClock TTC_CLOCK_SETTING;
	extern char LEVEL_ID;
	extern char NEXT_LEVEL_ID;
	extern char AREA_ID;
	extern char STAR_ID;
	extern uint8_t MAP_TILE_ARR_SIZE;
	extern char NUM_LIVES;
	extern Area* AREAS;
	extern Camera* CAMERA;
	extern Fix12i WATER_HEIGHT;
	extern int EVENT_FIELD;
	extern short NUM_COINS[2];
	extern Player* PLAYER_ARR[4];
	extern Input INPUT_ARR[4];
	extern uint16_t HEALTH_ARR[4];
	
	extern Actor* SILVER_STARS[12];	
	extern ActorDeathTable ACTOR_DEATH_TABLE_ARR[3]; //maximum three parts per level.
	
	extern ActorBase* ROOT_ACTOR_BASE;
	
	extern uint16_t* DEATH_BY_GIANT_SPAWN_TABLE;
	
	extern ActorBase::ProcessingListNode* FIRST_BEHAVIOR_LIST_NODE;
	extern ActorBase::ProcessingListNode* FIRST_RENDER_LIST_NODE;
	extern EnemyDeathFunc ENEMY_DEATH_FUNCS[8];

	extern uint8_t GAME_PAUSED; // 0 = not paused, 1 = paused, 2 = unpausing
	extern uint8_t CURRENT_GAMEMODE;
	extern unsigned AMBIENT_SOUND_EFFECTS_DISABLED;

	short GetAngleToCamera(unsigned playerID = 0);
	
	char SublevelToLevel(char levelID);
	int NumStars();
	int IsStarCollected(int actID, int starID);
	
	int DeathTable_GetBit(char bit);
	
	char StarCollectedInCurrLevel(int starID);
	
	void UnloadBlueCoinModel();
	void LoadBlueCoinModel();
	void UnloadSilverStarAndNumber();
	void LoadSilverStarAndNumber();
	void LinkSilverStarAndStarMarker(Actor* starMarker, Actor* silverStar);
	
	short ReadUnalignedShort(const char* from);
	int ReadUnalignedInt(const char* from);
}

//Obj to Model Scale: Divide integer units by 8. (So 1.000 (Q20.12) becomes 1000 / 8 = 125.)

//0202df70 is the start of the load obj bank overlays function (0202df84 is where the cleanup branch is)
//0202e034 and 0202e06c are the ends of the load obj bank overlays function.
//Overlay 0x3c ( 60) gets loaded in a Bowser fight level. It is loaded at 0x02111900 and takes up 0x92e0 bytes of space.
//Overlay 0x62 ( 98) is loaded if the boolean at 0x0209f2d8 is false. It contains the small wooden box, the cannon, the water bomb, and arrow signs.
//Overlay 0x66 (102) is loaded, period. It contains the ? block, the bob-omb, and the koopa shell.

//Super mushroom tag vtable: 02108cf4

/*void Vec3_InterpCubic(Vector3* vF, Vector3* v0, Vector3* v1, Vector3* v2, Vector3* v3, int t) NAKED; //0208f670, 70f60822
bool BezPathIter_Advance(BezierPathIter* it) NAKED; //0208f840, 40f80822

void Vec3_Interp(Vector3* vF, Vector3* v1, Vector3* v2, int t) NAKED; //02090dd0, d00d0922
short Vec3_VertAngle(Vector3* v1, Vector3* v2) NAKED; //0203b770, 70b70322
short Vec3_HorzAngle(Vector3* v1, Vector3* v2) NAKED; //0203b7ac, acb70322
void Vec3_LslInPl(Vector3* vF, int amount) NAKED; //0203d0a0, a0d00322
int  Vec3_HorzDist(Vector3* v1, Vector3* v2) NAKED; //0203cf40, 40cf0322
int  Vec3_Dist(Vector3* v1, Vector3* v2) NAKED; //0203cfdc, dccf0322
void Vec3_MulInPl(Vector3* v, int scalar) NAKED; //0203d224, 24d20322
void Vec3_Sub(Vector3* vF, Vector3* v1, Vector3* v2) NAKED; //0203d2fc, fcd20322
void Vec3_Add(Vector3* vF, Vector3* v1, Vector3* v2) NAKED; //0203d340, 40d30322

void Vec3_MulMat3x3(Vector3* v, Matrix3x3* m, Vector3* vF) NAKED; //020525a0, a0250522
void Mat3x3_Mul(Matrix3x3* m2, Matrix3x3* m1, Matrix3x3* mF) NAKED; //02052624, 25260522
void Mat4x3_Identity(Matrix4x3* mF) NAKED; //020527c0, c0270522
void Vec3_MulMat4x3(Vector3* v, Matrix4x3* m, Vector3* vF) NAKED; //02052858, 58280522
void Mat4x3_Mul(Matrix4x3* m2, Matrix4x3* m1, Matrix4x3* mF) NAKED; //02052914, 14290522

void Model_Update(Model* mdl) NAKED; //0201686c, 6c680122
void Model_UpdateBones(char** mdlFilePtr, char* animFile, int currFrame) NAKED; //02045394, 94530422
void Anim_Change(Model* mdl, char* newAnimFile, Animation::Flags flags, int animSpeed, int startFrame) NAKED; //02016748, 48670122

bool Player_ChangeState(Player* player, Player::State* state) NAKED; //020e30a0, a0300e22
void Player_ChangeAnim(Player* player, int animID, Animation::Flags flags, int animSpeed, int startFrame) NAKED;//020bef2c, 2cef0b22
void Sound_Play0(int soundID, Vector3* camSpacePos) NAKED; //0201264c, 4c260122
void Sound_Play3(int soundID, Vector3* camSpacePos) NAKED; //02012664, 64260122
void Sound_Play(int arg0, int soundID, Vector3* camSpacePos); //02012590, 90250122*/
#endif // SM64DS_2_H_INCLUDED