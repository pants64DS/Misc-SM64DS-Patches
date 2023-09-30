#ifndef SM64DS_MODEL_INCLUDED
#define SM64DS_MODEL_INCLUDED

#include "SM64DS_Common.h"

/*
	It is guaranteed that all structs with a vtable have all virtual functions defined. 
	To find the missing functions, start from the dtor and look for push instructions.
	Static functions can be easily found by testing r0 for an object's instance. If it's not, you have a static function.
	'done' denotes that all of the object's members have been found. However, they may not all have a trivial name.
*/


struct Model;
struct ModelAnim;
struct MaterialChanger;
struct TextureSequence;
struct ShadowModel;
extern "C"
{
	//Remember to have these loaded before spawning an object with them
	//They are as disorganized here as they are in memory.
	extern SharedFilePtr LUIGI_CAP_MODEL_PTR;
	extern SharedFilePtr RED_NUMBER_MODEL_PTR;
	extern SharedFilePtr POWER_FLOWER_OPEN_MODEL_PTR;
	extern SharedFilePtr COIN_YELLOW_POLY32_MODEL_PTR;
	extern SharedFilePtr WARIO_CAP_MODEL_PTR;
	extern SharedFilePtr COIN_BLUE_POLY32_MODEL_PTR;
	extern SharedFilePtr POWER_FLOWER_CLOSED_MODEL_PTR;
	extern SharedFilePtr ONE_UP_MUSHROOM_MODEL_PTR;
	extern SharedFilePtr BOB_OMB_MODEL_PTR;
	extern SharedFilePtr NUMBER_TEXSEQ_PTR;
	extern SharedFilePtr SNUFIT_BULLET_MODEL_PTR;
	extern SharedFilePtr COIN_RED_POLY32_MODEL_PTR;
	extern SharedFilePtr COIN_BLUE_POLY4_MODEL_PTR;
	extern SharedFilePtr SILVER_NUMBER_TEXSEQ_PTR;
	extern SharedFilePtr WATER_RING_MODEL_PTR;
	extern SharedFilePtr SHELL_GREEN_MODEL_PTR;
	extern SharedFilePtr SHELL_RED_MODEL_PTR;
	extern SharedFilePtr SILVER_NUMBER_MODEL_PTR;
	extern SharedFilePtr SUPER_MUSHROOM_MODEL_PTR;
	extern SharedFilePtr BUBBLE_MODEL_PTR;
	extern SharedFilePtr MARIO_CAP_MODEL_PTR;
	extern SharedFilePtr COIN_YELLOW_POLY4_MODEL_PTR;
	extern SharedFilePtr COIN_RED_POLY4_MODEL_PTR;
	extern SharedFilePtr FEATHER_MODEL_PTR;
	
	//Graphics ports
	//Do NOT read from the ports!
	extern volatile unsigned GXPORT_MATRIX_MODE;
	extern volatile unsigned GXPORT_MTX_LOAD_4x4;
	extern volatile unsigned GXPORT_MTX_LOAD_4x3;
	extern volatile unsigned GXPORT_LIGHT_VECTOR;
	extern volatile unsigned GXPORT_LIGHT_COLOR;
	
	extern uint16_t CHANGE_CAP_TOON_COLORS[0x20];
}

namespace Vram
{
	void StartTexWrite();
	void LoadTex(uint8_t* texelArr, unsigned texVramOffset, unsigned texelArrSize);
	void EndTexWrite();
	void StartPalWrite();
	void LoadPal(uint16_t* palColArr, unsigned palVramOffset, unsigned palleteSize);
	void EndPalWrite();
};

namespace GXFIFO
{
	inline void LoadMatrix4x3(const Matrix4x3* matrix)
	{
		GXPORT_MTX_LOAD_4x3 = matrix->c0.x.val;		GXPORT_MTX_LOAD_4x3 = matrix->c0.y.val;		GXPORT_MTX_LOAD_4x3 = matrix->c0.z.val;
		GXPORT_MTX_LOAD_4x3 = matrix->c1.x.val;		GXPORT_MTX_LOAD_4x3 = matrix->c1.y.val;		GXPORT_MTX_LOAD_4x3 = matrix->c1.z.val;
		GXPORT_MTX_LOAD_4x3 = matrix->c2.x.val;		GXPORT_MTX_LOAD_4x3 = matrix->c2.y.val;		GXPORT_MTX_LOAD_4x3 = matrix->c2.z.val;
		GXPORT_MTX_LOAD_4x3 = matrix->c3.x.val;		GXPORT_MTX_LOAD_4x3 = matrix->c3.y.val;		GXPORT_MTX_LOAD_4x3 = matrix->c3.z.val;
	}

	// Do NOT set the light vector to <1, 0, 0>, <0, 1, 0>, or <0, 0, 1>. Instead, do <0x0.ff8, 0, 0>, for example.
	inline void SetLightVector(int lightID, Fix12i x, Fix12i y, Fix12i z)
	{
		GXPORT_LIGHT_VECTOR = (((z.val >> 3 & 0x1ff) | (z.val >> 22 & 0x200)) << 10 |
								(y.val >> 3 & 0x1ff) | (y.val >> 22 & 0x200)) << 10 |
								(x.val >> 3 & 0x1ff) | (x.val >> 22 & 0x200) | lightID << 30;
	}

	inline void SetLightVector(int lightID, const Vector3& v)
	{
		SetLightVector(lightID, v.x, v.y, v.z);
	}

	inline void SetLightColor(int lightID, uint8_t r, uint8_t g, uint8_t b) //0x00 to 0xff
	{
		GXPORT_LIGHT_COLOR = (unsigned)b >> 3 << 10 |
							 (unsigned)g >> 3 <<  5 |
							 (unsigned)r >> 3 | lightID << 30;
	}
}

struct Command30
{
	unsigned diffuse : 15;
	unsigned ambient : 15;
	bool setsVertexColor : 1 = false;

	constexpr operator unsigned() const
	{
		return diffuse | setsVertexColor << 15 | ambient << 16;
	}
};

struct Command31
{
	unsigned specular : 15;
	unsigned emission : 15;
	bool usesShininessTable : 1 = false;

	constexpr operator unsigned() const
	{
		return specular | usesShininessTable << 15 | emission << 16;
	}
};

struct BMD_File
{
	struct Bone
	{
		enum Flags
		{
			BILLBOARD = 1 << 0
		};

		int boneID;
		char* name;
		short offsetToParent; // in bones, not in bytes
		short hasChildren;
		int offsetToNextSibling; // in bones, not in bytes
		Vector3 scale;
		Vector3_16 rotation;
		uint16_t unk22; // probably padding
		Vector3 translation;
		unsigned numDisplayListMaterialPairs;
		void* materialIDLis;
		void* diplayListIDList;
		unsigned flags;
	};

	static_assert(sizeof(Bone) == 0x40);

	struct Texture
	{
		char* name;
		char* data; // dangling after Model::LoadFile
		unsigned size;
		uint16_t width;
		uint16_t height;
		unsigned cmd2aPart1;
	};

	static_assert(sizeof(Texture) == 0x14);

	struct Palette
	{
		char* name;
		char* data; // dangling after Model::LoadFile
		unsigned size;
		unsigned vramOffset;
	};

	static_assert(sizeof(Palette) == 0x10);

	struct Material
	{
		char* name;
		int textureID; // -1 when no texture
		int paletteID; // -1 when no palette
		Vector2 scale;
		short rotation;
		Vector2 translation;
		unsigned cmd2aPart2;
		unsigned cmd29;
		unsigned cmd30;
		unsigned cmd31;
	};

	static_assert(sizeof(Material) == 0x30);

	unsigned scaleShift;
	unsigned numBones;
	Bone* bones;
	unsigned numDisplayLists;
	void* displayLists;
	unsigned numTextures;
	Texture* textures;
	unsigned numPalettes;
	Palette* palettes;
	unsigned numMaterials;
	Material* materials;
	void* TransformMap;
	unsigned unk30;
	unsigned unk34;
	unsigned ramSize;

	// These three functions are called in SharedFilePtr::LoadBMD
	// if numRefs == 1 after loading the file

	void AdjustPointers(); // before this is called, the pointers are offsets within the file
	void LoadTexturesToVRAM(); // also loads texture palettes
	void ShrinkAllocation(); // unloads textures and palettes from RAM by shrinking the allocation size to ramSize
};

static_assert(sizeof(BMD_File) == 0x3c);

struct MaterialProperties
{
	short materialID;
	short unk02; //probably just padding
	const char* materialName;
	
	char unk08; bool difRedAdv;     uint16_t difRedOffset;
	char unk0c; bool difGreenAdv;   uint16_t difGreenOffset;
	char unk10; bool difBlueAdv;    uint16_t difBlueOffset;
	char unk14; bool ambRedAdv;     uint16_t ambRedOffset;
	char unk18; bool ambGreenAdv;   uint16_t ambGreenOffset;
	char unk1c; bool ambBlueAdv;    uint16_t ambBlueOffset;
	char unk20; bool specRedAdv;    uint16_t specRedOffset;
	char unk24; bool specGreenAdv;  uint16_t specGreenOffset;
	char unk28; bool specBlueAdv;   uint16_t specBlueOffset;
	char unk2c; bool emitRedAdv;    uint16_t emitRedOffset;
	char unk30; bool emitGreenAdv;  uint16_t emitGreenOffset;
	char unk34; bool emitBlueAdv;   uint16_t emiBlueOffset;
	char unk38; bool alphaAdv;      uint16_t alphaOffset;
};

struct MaterialDef
{
	uint16_t numFrames;
	uint16_t unk02;
	char* values;
	int matPropCount;
	MaterialProperties* matProp;
};

struct TexSRTAnim
{
	uint16_t materialID;
	uint16_t unk02; //probably just padding
	const char* materialName;
	uint16_t numScaleXs;
	uint16_t scaleXOffset;
	uint16_t numScaleYs;
	uint16_t scaleYOffset;
	uint16_t numRots;
	uint16_t rotOffset;
	uint16_t numTransXs;
	uint16_t transXOffset;
	uint16_t numTransYs;
	uint16_t transYOffset;
	
};

struct TexSRTDef
{
	unsigned numFrames;
	Fix12i* scales;
	short* rots;
	Fix12i* transs;
	int texAnimCount;
	TexSRTAnim* texAnims;
};

struct Bone
{
	unsigned unk00;
	unsigned unk04;
	unsigned unk08;
	Vector3 scale;
	uint16_t unk18;
	Vector3_16 rot;
	Vector3 pos;
	unsigned unk2c;
	unsigned unk30;
};



struct Animation	//internal: FrameCtrl; done
{

	static constexpr int FLAG_MASK = 0xC0000000;

    enum Flags : int
    {
        LOOP = 0x00000000,
        NO_LOOP = 0x40000000
    };
	
	//vtable
	Fix12i numFramesAndFlags;
	Fix12i currFrame;
	Fix12i speed;
	
	virtual ~Animation();
	void Advance();
	bool Finished();

	Animation(const Animation&) = delete;
	Animation(Animation&&) = delete;

	Flags GetFlags();
	void SetFlags(Flags flags);
	unsigned GetFrameCount();
	void SetAnimation(uint16_t frames, Flags flags, Fix12i speed = 1._f, uint16_t startFrame = 0);
	void Copy(const Animation& anim);
	bool Func_02015A98(int arg0); //Does something like simulating an advance? Like checking if the next frame expires the animation...

	static char* LoadFile(SharedFilePtr& filePtr);
};



struct Material
{
	unsigned unk00;
	unsigned unk04;
	Fix12i texScaleX;
	Fix12i texScaleY;
	short texRot; //then alignment
	Fix12i texTransX;
	Fix12i texTransY;
	unsigned teximageParam; //gx command 0x2a
	unsigned paletteInfo;
	unsigned polygonAttr; //gx command 0x29
	unsigned difAmb; //gx command 0x30
	unsigned speEmi; //gx command 0x31

	constexpr void EnableScrolling()
	{
		teximageParam &= ~0xC0000000;
		teximageParam |=  0x40000000;
	}

	constexpr void SetAlpha(unsigned alpha) // from 0 to 31
	{
		polygonAttr &= ~(0b11111 << 16);
		polygonAttr |= (alpha & 0b11111) << 16;
	}

	constexpr void SetPolygonID(unsigned polygonID) // from 0 to 63
	{
		polygonAttr &= ~(0x3f << 24);
		polygonAttr |= (polygonID & 0x3f) << 24;
	}

	constexpr void ShowBackSide () { polygonAttr |=  (1 << 6); }
	constexpr void HideBackSide () { polygonAttr &= ~(1 << 6); }
	constexpr void ShowFrontSide() { polygonAttr |=  (1 << 7); }
	constexpr void HideFrontSide() { polygonAttr &= ~(1 << 7); }
	constexpr void EnableFog    () { polygonAttr |=  (1 << 15); }
	constexpr void DisableFog   () { polygonAttr &= ~(1 << 15); }
};

struct ModelComponents
{
	BMD_File* modelFile;
	Material* materials;
	Bone* bones;
	Matrix4x3* transforms;
	char* unk10;
	
	void UpdateBones(char* animFile, int frame);
	void UpdateVertsUsingBones();
	void Render(Matrix4x3* mat = nullptr, Vector3* scale = nullptr);
};


struct MaterialChanger : Animation		//internal: AnmMaterial; done
{
	MaterialDef* material;

	MaterialChanger();
	virtual ~MaterialChanger();
    static void Prepare(BMD_File& modelFile, MaterialDef& matDef);
	void SetMaterial(MaterialDef& matDef, Flags flags, Fix12i speed, unsigned startFrame);
	void Update(ModelComponents& modelData);

	[[deprecated]]
	static void Prepare(char* modelFile, MaterialDef& matDef)
	{
		Prepare(*reinterpret_cast<BMD_File*>(modelFile), matDef);
	}
};


struct TextureTransformer : Animation	//internal: AnmTexSRT; done
{
	TexSRTDef* texSRT;

	TextureTransformer();
	virtual ~TextureTransformer();
    static void Prepare(BMD_File& modelFile, TexSRTDef& texDef);
	void SetTexSRT(TexSRTDef& texDef, Flags flags, Fix12i speed, unsigned startFrame);
	void Update(ModelComponents& modelData);

	[[deprecated]]
	static void Prepare(char* modelFile, TexSRTDef& texDef)
	{
		Prepare(*reinterpret_cast<BMD_File*>(modelFile), texDef);
	}
};


struct TextureSequence : Animation		//internal: AnmTexPat; done
{
	char* texSequenceFile;

	TextureSequence();
	virtual ~TextureSequence();
    static void Prepare(BMD_File& modelFile, char* texSeqFile);
	void SetFile(char* texSeqFile, Flags flags, Fix12i speed, unsigned startFrame);
	void Update(ModelComponents& modelData);	

	static char* LoadFile(SharedFilePtr& filePtr);

	static void Prepare(char* modelFile, char* texSeqFile)
	{
		Prepare(*reinterpret_cast<BMD_File*>(modelFile), texSeqFile);
	}
};



struct ModelBase	//internal: Model; done
{
	//vtable
	unsigned unk04;			//Pointer that is freed in all dtors

	ModelBase();
	virtual ~ModelBase();

	ModelBase(const ModelBase&) = delete;
	ModelBase(ModelBase&&) = delete;

	bool SetFile(BMD_File& file, bool enableFog = false, int polygonID = -1);
	virtual bool DoSetFile(BMD_File& file, bool enableFog = false, int polygonID = -1) = 0;

	[[deprecated]]
	bool SetFile(char* file, bool enableFog = false, int polygonID = -1)
	{
		return SetFile(*reinterpret_cast<BMD_File*>(file), enableFog, polygonID);
	}
};




struct Model : public ModelBase		//internal: SimpleModel
{
	ModelComponents data;
	Matrix4x3 mat4x3;
	Matrix4x3* unkMatPtr;
	
	Model();
	virtual ~Model();
	virtual bool DoSetFile(BMD_File& file, bool enableFog = false, int polygonID = -1) override;
	virtual void UpdateVerts();
	virtual void Virtual10(Matrix4x3& arg0);
	virtual void Render(const Vector3* scale = nullptr);

	void Render(const Vector3& scale) { Render(&scale); }
	void Render(Fix12i scale) { Render({scale, scale, scale}); }

	[[deprecated]]
	static char* LoadFile(SharedFilePtr& filePtr)
	{
		return reinterpret_cast<char*>(&filePtr.LoadBMD());
	}
};

struct ModelAnim : public Model, Animation	//internal: ModelAnm
{
	//vtable
	//Animation data
	char* file;

	ModelAnim();
	virtual ~ModelAnim();
	virtual void UpdateVerts() override;
	virtual void Virtual10(Matrix4x3& arg0) override;
	virtual void Render(const Vector3* scale = nullptr) override; // Calls UpdateVerts and then Model::Render
	virtual void Virtual18(unsigned arg0, const Vector3* scale);  // Calls Virtual10 and then Model::Render
	
	void SetAnim(char* animFile, int flags, Fix12i speed = 1._f, unsigned startFrame = 0);

	void Copy(const ModelAnim& anim, char* newFile);					//if newFile != nullptr, it gets copied instead of anim->file
};

struct ModelAnim2 : public ModelAnim	//internal: ModelAnm2
{
	unsigned unk64;
	Animation otherAnim;
	
	void Copy(const ModelAnim2& anim, char* newFile, unsigned newUnk64);	//copies anim to *this, otherAnim is set to anim's Animation base class

	ModelAnim2();
	virtual ~ModelAnim2();

	//2 funcs missing before
	void Func_020162C4(unsigned newUnk64, Flags animFlags, Fix12i speed, uint16_t startFrame);				//Always calls on otherAnim
};

struct ShadowModel : public ModelBase	//internal: ShadowModel; done
{
	ModelComponents* modelDataPtr;
	Matrix4x3* matPtr;
	Vector3 scale;
	uint8_t opacity;
	uint8_t unk1d;
	uint8_t unk1e;
	uint8_t unk1f;
	ShadowModel* prev;
	ShadowModel* next;
	
	ShadowModel();
	virtual ~ShadowModel();
	bool InitCylinder();
	bool InitCuboid();

	// May only have 2 params, but then it wouldn't match ModelBase's declaration
	virtual bool DoSetFile(BMD_File& file, bool enableFog = false, int polygonID = -1) override;

	// The opacity is from 0 to 30
	void InitModel(Matrix4x3* transform, Fix12i scaleX, Fix12i scaleY, Fix12i scaleZ, unsigned opacity);

	static void RenderAll();
	static void Func_02015E14();

};


struct CommonModel : public ModelBase	//internal: CommonModel; done
{
	ModelComponents* data;
	Matrix4x3 mat4x3;

	CommonModel();
	virtual ~CommonModel();
	virtual bool DoSetFile(BMD_File& file, bool enableFog = false, int polygonID = -1) override;

	void Func_0201609C(unsigned arg0);
	void Func_020160AC(unsigned arg0, unsigned arg1);
	void Render(const Vector3* scale = nullptr);

	void Render(const Vector3& scale) { Render(&scale); }
	void Render(Fix12i scale) { Render({scale, scale, scale}); }
};



struct BlendModelAnim : public ModelAnim	//internal: BlendAnmModel
{
	unsigned unk64;
	unsigned unk68;
	unsigned unk6C;

	//0x0208E94C vtable, 0x020166D4 ctor
	BlendModelAnim();
	virtual ~BlendModelAnim();

	virtual bool DoSetFile(BMD_File& file, bool enableFog = false, int polygonID = -1) override;
	virtual void UpdateVerts() override;
	virtual void Virtual10(Matrix4x3& arg0) override;
	virtual void Render(const Vector3* scale = nullptr) override;
	virtual void Virtual18(unsigned arg0, const Vector3* scale) override;		//Calls Virtual10 and then Model::Render

	//2 funcs missing

};


//vtable at 0x0208EAFC
struct Fader		//internal name: dFader
{

};


//vtable at 0x0208EACC
struct FaderBrightness : public Fader		//internal name: dFdBrightness
{

};


//vtable at 0x0208EA40
struct FaderColor : public FaderBrightness	//internal name: dFdColor
{

};


//vtable at 0x0208EA9C
struct FaderWipe : public FaderColor		//internal name: dFdWipe
{

};

#endif