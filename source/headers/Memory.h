#ifndef SM64DS_MEMORY_INCLUDED
#define SM64DS_MEMORY_INCLUDED



struct HPXE
{
	const char magic[4];			//HPXE
	unsigned unk04;
	unsigned unk08;
	HPXE* nestedHPXE;				//Pointer to nested heap HXPE
	HPXE* nestedHPXE2;				//Pointer to nested heap after the first one
	uint16_t unk14;
	uint16_t unk16;
	unsigned heapStart;
	unsigned heapEnd;
	unsigned unk20;					//Align?
	MemoryNode* firstFreeBlock;
	MemoryNode* lastFreeBlock;
	MemoryNode* firstAllocatedBlock;
	MemoryNode* lastAllocatedBlock;
	uint16_t unk34;
	uint16_t unk36;
};



struct MemoryNode
{
	struct TargetInfo				//Allocated on stack
	{
		unsigned start;
		unsigned end;
	};

	char magic[2];					//DU or RF
	uint16_t unk02;
	unsigned size;
	MemoryNode* previous;
	MemoryNode* next;
};






//vtable at 0x02099D90, ctor at 0x0203CAAC
struct Heap								//internal name: mHeap::Heap_t
{
	unsigned memoryBegin;
	unsigned maxAllocatedBytes;			//Heap area containing HPXE and allocated data
	Heap* rootHeap;
	unsigned unk10;

	Heap(unsigned a, unsigned b, unsigned c, unsigned unused);
	virtual ~Heap();

};


//vtable at 0x02099DD8, ctor at 0x0203CA80
struct ExpandingHeap : public Heap		//internal name: mHeap::ExpHeap_t
{
	HPXE* hpxe;

	ExpandingHeap();
	virtual ~ExpandingHeap();

	virtual void* Allocate(unsigned size, unsigned addressMask);												//Allocates size bytes with an alignment of addressMask
	virtual bool Deallocate(void* ptr);																			//Deallocates ptr from heap (0x10)
	virtual unsigned GetSize(void* ptr);																		//Returns the size of an allocated block

	static MemoryNode* linkNode(MemoryNode* hpxeFLNodePair, MemoryNode* newNode, MemoryNode* prevNode);			//Link node on heap (corrects previous and next) and return a pointer to the new node
	static MemoryNode* unlinkNode(MemoryNode* hpxeFLNodePair, MemoryNode* target);								//Unlink node from heap (corrects previous and next) and return a pointer to the node before the unlinked one
	static MemoryNode* createNode(MemoryNode::TargetInfo* targetInfo, char* nodeType);							//Sets up a new node at targetInfo.start, zeroes it and stores the block size in it. Returns a pointer to the newly created node.
	static unsigned AllocatedSize(void* ptr);																	//Returns the allocated size of a generic pointer to an allocated memory block

};

//0x0204E8E0 linkNode
//0x0204E910 unlinkNode
//0x0204E8B0 createNode
//0x0204E084 AllocatedSize

//0x020A0EA0 active heap ptr?
//special struct at +0x18


//vtable at 0x02099D48, ctor at 0x0203CA54
struct SolidHeap : public Heap			//internal name: mHeap::SolidHeap_t
{
	SolidHeap();
	virtual ~SolidHeap();
};


#endif	// SM64DS_MEMORY_INCLUDED