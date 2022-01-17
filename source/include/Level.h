#ifndef SM64DS_LEVEL_INCLUDED
#define SM64DS_LEVEL_INCLUDED

#include "SM64DS_Common.h"


//Order for spawning: 2 3 4 b 1 6 7 8 9 c e a
//In other words:
/*
	Path Nodes
	Paths
	Views
	Minimap Tile File
	Level Entrances
	Teleport Source
	Teleport Dest
	Fog
	Doors
	Minimap Scale
	(Unknown)
	Level Exits
*/
//Not putting these in order in the file can result in drastic consequences
//such as 8-directional cameras suddenly getting angle 0x751c.

namespace LevelFile
{
	struct MapTile;
	struct View;
	struct Entrance;
	struct Path;
}

extern "C"
{
	extern LevelFile::MapTile (*MAP_TILE_ARR_PTR)[];
	extern LevelFile::View (*VIEW_ARR_PTR)[];
	extern LevelFile::Entrance (*ENTRANCE_ARR_PTR)[];
	extern LevelFile::Path (*PATH_ARR_PTR)[];

	extern uint8_t LAST_ENTRANCE_ID;
	extern uint8_t NUM_ENTRANCES;
	extern uint8_t NUM_VIEWS;
	extern unsigned NUM_PATHS;
}

struct PathPtr
{
	const LevelFile::Path* ptr;

	// originally contains this second member, but none of the documented
	// functions use it so it's commented out for better performance

	// unsigned unk04 = 0;
	
	PathPtr() : ptr(nullptr) {}
	PathPtr(const LevelFile::Path* path) : ptr(path) {}
	PathPtr(const LevelFile::Path& path) : ptr(&path) {}
	explicit PathPtr(unsigned pathID) { FromID(pathID); }
	
	void FromID(unsigned pathID);
	void GetNode(Vector3& vF, unsigned index) const;
	unsigned NumNodes() const;

	[[gnu::always_inline]]
	auto GetNode(const unsigned& index) const
	{
		return Vector3::Proxy([this, &index]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			GetNode(res, index);
		});
	}

	inline const LevelFile::Path& operator* () const { return *ptr; }
	inline const LevelFile::Path* operator->() const { return ptr; }

	inline explicit operator bool() const { return ptr != nullptr; }

	inline bool operator==(const PathPtr& other) const { return this->ptr == other.ptr; }

	inline operator const LevelFile::Path*      &()       { return ptr; }
	inline operator const LevelFile::Path* const&() const { return ptr; }
};

namespace LevelFile
{
	struct PathNode
	{
		Vector3_16 pos;
	};
	
	struct Path
	{
		uint16_t firstNodeID;
		uint8_t numNodes;
		uint8_t unk3; // always 0xff?
		uint8_t unk4; // inheritance?
		uint8_t unk5; // 0xff = loop, 0x7f = no loop?
	};
	
	struct View
	{
		enum Modes : uint8_t
		{
			OUTSIDE_CYLINDER,
			INSIDE_CYLINDER,
			NORMAL_CAMERA,
			POINT_FOR_MULTIFOCUS_CAMERA,
			ROTATION_ONLY_CAMERA,
			SPIRALING_STAIRS, // ?
			PATH_FOLLOWING_CAMERA,
			PAUSE_CAMERA_CENTER_POINT
		};

		uint8_t mode;
		uint8_t param2; // ff: normal
		Vector3_16 pos;
		Vector3_16 rot;
		
		static LevelFile::View& Get(unsigned viewID);
	};

	struct Entrance
	{
		uint16_t unk00; // just zero?
		Vector3_16 pos;
		Vector3_16 rot;
		uint16_t unk0e; // ???
	};
	
	struct MapTile
	{
		short ov0FileID;
	};
}

#endif