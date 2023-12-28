#include "actor_tree.h"

static_assert(alignof(ActorTreeNode) == alignof(Actor));
static constinit std::byte* newExtensionAddr;

std::byte* AllocateOnGameHeap(size_t size);

asm(R"(
_Z18AllocateOnGameHeapj:
	push  {r4, r5, r14}
	b     _ZN9ActorBasenwEj + 4
)");

// at the beginning of ActorBase::operator new
void* nsub_02043444(size_t size)
{
	std::byte* allocAddr = AllocateOnGameHeap(size + sizeof(ActorTreeNode));
	newExtensionAddr = allocAddr + size;
	
	return allocAddr;
}

asm(R"(
nsub_020114e0 = _Z18ConstructExtensionR5Actor
nsub_0201162c = _Z18ConstructExtensionR5Actor
)");

Actor& ConstructExtension(Actor& actor)
{
	new (newExtensionAddr) ActorTreeNode(actor);

	return actor;
}

asm(R"(
repl_020112cc:
repl_02011318:
repl_02011378:
	mov  r4, r0
	b    _Z17DestructExtensionRK5Actor
)");

static ActorTreeNode& GetTreeNode(const Actor& actor)
{
	if (!Memory::gameHeapPtr) Crash();
	const std::size_t offset = Memory::gameHeapPtr->Sizeof(&actor) - sizeof(ActorTreeNode);

	return const_cast<ActorTreeNode&>(
		*reinterpret_cast<const ActorTreeNode*>(
			reinterpret_cast<const std::byte*>(&actor) + offset
		)
	);
}

void DestructExtension(const Actor& actor)
{
	GetTreeNode(actor).~ActorTreeNode();
}
