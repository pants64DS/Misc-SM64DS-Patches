#include "actor_extension.h"
#include <array>
#include <type_traits>

static_assert(alignof(ActorExtension) <= alignof(Actor));
static_assert(std::is_trivially_destructible_v<ActorExtension>);

constexpr size_t AlignOffset(size_t offset)
{
	constexpr size_t alignment = alignof(ActorExtension);
	return (offset + alignment - 1) & ~(alignment - 1);
}

class ExtensionOffsetTable
{
	static constexpr uint16_t length = 0x310;
	std::array<uint16_t, length> arr;

public:

	inline size_t Insert(uint16_t actorID, size_t offset)
	{
		offset = AlignOffset(offset);
		
		if (actorID < length && offset < 0x10000) [[likely]]
			return arr[actorID] = offset;

		else [[unlikely]]
			return 0;
	}

	inline size_t Get(uint16_t actorID)
	{
		if (actorID < length) [[likely]]
			return arr[actorID];

		else [[unlikely]]
			return 0;
	}
}
extern offsetTable;

uint16_t spawningActor = 0;

char* AllocateOnGameHeap(size_t size);

// at the beginning of ActorBase::operator new
void* nsub_02043444(size_t size)
{
	if (spawningActor > 0)
	{
		const size_t offset = offsetTable.Insert(spawningActor, size);

		if (offset > 0)
		{
			char* const allocAddr = AllocateOnGameHeap(offset + sizeof(ActorExtension));

			new (allocAddr + offset) ActorExtension();

			return allocAddr;
		}
	}

	return AllocateOnGameHeap(size);
}

ActorExtension* GetActorExtension(const Actor& actor)
{
	size_t offset = offsetTable.Get(actor.actorID);
	if (offset == 0) return nullptr;

	const char* ptr = reinterpret_cast<const char*>(&actor) + offset;
	return const_cast<ActorExtension*>(reinterpret_cast<const ActorExtension*>(ptr));
}
