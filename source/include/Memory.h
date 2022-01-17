#ifndef SM64DS_MEMORY_INCLUDED
#define SM64DS_MEMORY_INCLUDED


//WARNING: Only use these objects if you REALLY know what you're doing. Heap management in SM64DS is VERY unstable when using functions outside of their respective safe scopes.
//WARNING: You should NEVER touch HeapAllocator functions when hacking. Those are used internally to manage the memory and represent the low level management of the heap. 
//WANRING: It is also strictly prohibited to write to HeapAllocator members. Read access is safe. 
//HINT: HeapAllocator objects are only (and REALLY only) defined for documentation purposes on the heap's internals and to register the symbols to aid debugging.
//HINT: All functions dealing indirectly with the heap (Solid-/ExpandingHeap) are safe to use, but be careful with the correct addresses if you try to create a new heap.
//HINT: align is sometimes a signed integer. If align is positive, the heap is traversed forwards; if negative, the heap is traversed backwards. The abs of align determines the real memory alignment.
//HINT: The name of Heap's virtual function 0x1C (Rescue) is just made up. It always translates to a direct return without any other information. The name has been derived from the fact that it only appears to be called before and by Crash. Its purpose may have been to rescue certain heap data before the game fully crashes.
//HINT: Address ranges: Frontend: 0x0203C1B4 - 0x0203CC24; Backend: 0x0204DCFC - 0x0204EDA0


/*
	INFORMATION
	How the game uses heaps:
	SM64DS creates a root heap of type ExpandingHeap at a specific address given by Heap::nextRootHeapStart (only available at the beginning since it's overwritten
	after the call to InitializeRootHeap)[this is not 100% correct, see 'How to resize the heaps']. Its size is given by Heap::rootHeapEnd - the start allocation address. 
	When the root heap is set up, it's set as the default heap for default allocation functions (new/delete/malloc/free). Heap::rootHeapPtr is set to point to it at this stage.
	The root heap stores every other heap and anything else such as temporary model data, file data, etc. There is also a root heap iterator which has the root heap as
	its first element. It suggests the game could theoretically have multiple root heaps all linked through this iterator, but it's not wise to do so (especially since 
	the game was compiled with only one root heap in mind. Heap::rootParamOffset with anything else than 0 would point to some other data which may inflict hard-to-debug crashs).
	The second heap created is the game heap which is responsible for all actor allocations. It is allocated as the first element on the root heap with a hardcoded size (0x3B000).
	The third heap created is the sound heap which stores all sound related files currently active. It additionally created two nested heaps inside of it (which in turn are
	pretty small). It is allocated as the second element on the root heap with a size of 0x100000. 
	The game never creates a real SolidHeap but only utilizes its allocator. The sound heap allocator starts offset by 0x10 bytes after the allocated pointer.
*/

/*
	INFORMATION
	Heap types:
	ExpandingHeap:
	A heap which does not expand like the name suggests but rather allocates each element as a block. These blocks are linked to each other via a double linked list. 
	There are two linked lists: One with all free blocks and one with all allocated blocks. It is very easy to traverse and pretty efficient since the allocation order/direction
	doesn't really matter, but fragmentation may occur if a large enough chunk doesn't exist anymore (only many small free memory blocks).
	Data belonging to a node is found right after the respective MemoryNode's members.
	SolidHeap:
	A heap which allocates linearly. That means, each element is allocated in the big free block. Since there is no way to deallocate specific data (because of the one-free-block-limitation)
	it implements a special mechanism: Allocation states. An allocation state may be created at any time and stores the current free block state. If it wishes to deallocate
	certain data, it simply restores the state. That makes the heap way faster because no linked list adjustment must be made, but a bit more static and less flexible.
*/

/*
	INSTRUCTION
	How to resize the heaps:
	1) Root heap:
	The root heap's parameters are governed by hardcoded parameters and predefined globals:
	a) Loading address: Located at 0x02058DE0 (indirect loading parameter for a function). It is NOT the direct loading address. Instead, it allocates some information at this address.
	The real loading address is calculated via x = align(32) loadingAddress + 0x14 + n * 0xC; where n is hardcoded as 4. x is aligned at a 32-byte boundary, so padding may be added.
	Account this formula when calculating the loading address.
	b) Size: Simply Heap::rootHeapEnd - loadingAddress. Don't increase Heap::rootHeapEnd; remember that the DTCM starts there. Stack corruption is the last thing you want.
	Set it just before the call to InitializeRootHeap (you need the special globals to be loaded, so this is a safe spot).
	2) Game heap:
	The game heap's parameters are governed by hardcoded parameters and dynamic allocation:
	a) Loading address: The first root heap allocation which is typically offset to Heap::rootHeapPtr + 0x60.
	b) Size: Hardcoded. You need an assembly patch for this. Look at function 0x0201A054: Two lines before the call to InitializeGameHeap you see mov r0, 0x3B000. 
	Change the constant to your desired size. WARNING: It must fit into mov-instruction encoding format.
	3) Sound heap:
	The sound heap's parameter are governed by hardcoded parameters and dynamic allocation:
	a) Loading address: The second root heap allocation which is typically offset to Heap::rootHeapPtr + 0x60 + 0x18 + gameHeapSize + 0x10.
	b) Size: Hardcoded. You need an assembly patch for this. Look at function 0x020133BC: You find at the beginning two mov's with the constant 0x100000. Change both of them to
	the same desired size value. WARNING: It must fit into mov-instruction encoding format.
*/

/*
#define SM64DS_SAFE_REALLOC				//If enabled, checks for defaultHeapPtr = null in realloc
*/

struct HeapAllocator;
struct ExpandingHeap;
struct SolidHeap;


struct NestedHeapIterator
{
	HeapAllocator* first;										//Pointer to the first nested HeapAllocator
	HeapAllocator* last;										//Pointer to the last nested HeapAllocator
	uint16_t nestedHeaps;										//Number of nested heaps belonging to the iterator
	uint16_t nestedIterOffset;									//Offset for all nested heaps from the start of HeapAllocator to prev (most of the time 4)

	NestedHeapIterator(unsigned offset);
	HeapAllocator* Previous(HeapAllocator* allocator);			//Returns the previous heap of allocator (iterator object should be the parent). If allocator is 0, the last nested heap is returned instead.
	HeapAllocator* Next(HeapAllocator* allocator);				//Returns the next heap of allocator (iterator object should be the parent). If allocator is 0, the first nested heap is returned instead.
	HeapAllocator* RecursiveFindNested(void* ptr);				//Recursive function that returns either the last nested allocator containing ptr or a nullptr if the allocator owning the iterator is already the last nested allocator.
	void AddFirst(HeapAllocator* allocator);					//Adds allocator to the iterator as the first element.
	void AddAt(HeapAllocator* ref, HeapAllocator* allocator);	//Adds allocator to the iterator (which contains ref) right before ref
	void AddLast(HeapAllocator* allocator);						//Adds allocator to the iterator as the last element.
	void Init(HeapAllocator* allocator);						//Sets allocator as the first element.
	void Remove(HeapAllocator* allocator);						//Removes allocator from the iterator.

	static NestedHeapIterator* FindNested(void* ptr);			//Returns the last nested allocator's iterator containing ptr, or the root iterator if there is no nested iterator yet. Calls FindNestedHeap on the root iterator.

};


struct HeapAllocator
{
	const char magic[4];						//HPXE for ExpandingHeapAllocator, HMRF for SolidHeapAllocator
	HeapAllocator* prev;						//In case two heap allocators reside on the same root heap, they're linked with prev and next
	HeapAllocator* next;
	NestedHeapIterator nestedHeapIt;			//Iterator to traverse nested heaps
	void* heapStart;
	void* heapEnd;
	unsigned flags;								//1: Fill with 0's if set; 2: Unused but also set if 1 is set

	void Remove();								//Removes the heap. Be careful: All pointers should be deallocated before removal.
	void Destroy();								//Calls Remove()

	HeapAllocator(unsigned name, void* heapStart, void* heapEnd, unsigned flags);

};


struct MemoryNode
{
	struct Target
	{
		void* start;
		void* end;

		Target(MemoryNode* node);				//Creates a Target from the node's parameters
	};

	char magic[2];								//DU (allocated) or RF (free)
	uint16_t flags;								//0x00FF: Node ID, 
												//0x7F00: Relative allocation offset (in case the node starts later than the reserved memory, this serves as a hint to Target in order to avoid memory leaks)
												//0x8000: Allocation direction (0=forwards, 1=backwards)
	unsigned size;								//Size of data block
	MemoryNode* prev;
	MemoryNode* next;
};


struct ExpandingHeapAllocator : public HeapAllocator
{
	typedef void (*DeallocationFunction)(void*, ExpandingHeapAllocator*, unsigned);

	MemoryNode* firstFreeBlock;
	MemoryNode* lastFreeBlock;
	MemoryNode* firstAllocatedBlock;
	MemoryNode* lastAllocatedBlock;
	uint16_t nodeID;												//Nodes can be assigned a number which is copied from here. Allocation nodes for nested heaps from root are assigned 2, anything else 3 (the game heap doesn't assign those).
	uint16_t allocationMode;										//0: First fit (first one); 1: Best fit (smallest one)

	void* Allocate(unsigned size, int align);						//Allocates memory by calling Allocate<Direction> and returns a ptr to it. Size needn't be a multiple by four since it gets aligned before the call.
	void* AllocateForwards(unsigned size, unsigned align);			//Tries to find an address to fit the allocation aligned at align bytes. If enough space is available, it returns a pointer to the newly allocated memory. If not, it returns a nullptr. Allocation traversial is performed forwards.
	void* AllocateBackwards(unsigned size, unsigned align);			//Tries to find an address to fit the allocation aligned at align bytes. If enough space is available, it returns a pointer to the newly allocated memory. If not, it returns a nullptr. Allocation traversial is performed backwards.
	bool Deallocate(void* ptr);										//Deallocates, returns 1 if successful.
	unsigned Reallocate(void* ptr, unsigned newSize);				//Reallocates and returns the final node size. If newSize is less than the previous size, a part of ptr is freed; if newSize is greater than the previous size and no free block directly after the old block exists where it could fit in, it returns 0;
	unsigned MemoryLeft();											//Returns the total number of free bytes.
	unsigned MaxAllocatableSize(int align);							//Returns the size of the biggest contiguous free memory block aligned with least alignment padding (if multiple have the same size)
	unsigned SetNodeID(unsigned id);								//Sets nodeID to the given id and returns the previous value.
	unsigned GetNodeID();											//Returns the current nodeID.
	void DeallocateAll(DeallocationFunction* func, unsigned tmp);	//Deallocates all nodes.

	static MemoryNode* LinkNode(MemoryNode* nodePair, MemoryNode* newNode, MemoryNode* prevNode);			//Link node on heap (corrects previous and next) and return a pointer to the new node
	static MemoryNode* UnlinkNode(MemoryNode* nodePair, MemoryNode* target);								//Unlink node from heap (corrects previous and next) and return a pointer to the node before the unlinked one
	static MemoryNode* CreateNode(MemoryNode::Target* target, uint16_t nodeType);							//Sets up a new node at target.start, zeroes it and stores the block size in it. Returns a pointer to the newly created node.
	static void* AllocateNode(MemoryNode* freeNodePair, MemoryNode* freeNode, void* address, unsigned size, unsigned direction);	//Allocates size bytes at address (and zeroes it) inside freeNode, fixes node links and returns a ptr to the data of the new node after setup
	static bool FreeNode(MemoryNode* freeNodePair, MemoryNode::Target* target);								//Frees the node and merges free blocks. Returns 1 if deallocation has been successful, 0 otherwise.
	static unsigned SizeofInternal(void* ptr);																//Returns the allocated size of a generic pointer to an allocated memory block
	static void InvokeDeallocate(void* ptr, ExpandingHeapAllocator* allocator, unsigned tmp);				//Translates to allocator->Deallocate(ptr). tmp is used in order to swap the registers since this function acts as a thunk for Deallocate.

	ExpandingHeapAllocator(void* heapEnd, unsigned flags);

};


struct AllocationState
{
	unsigned id;
	void* blockBegin;
	void* blockEnd;
	AllocationState* nextState;
};


struct SolidHeapAllocator : public HeapAllocator 
{
	void* freeBlockBegin;
	void* freeBlockEnd;
	AllocationState* state;

	void* Allocate(unsigned size, int align);
	unsigned Reallocate(void* ptr, unsigned newSize);
	unsigned MemoryLeft(int align);
	unsigned TryResizeToFit();								//Tries to resize the heap and returns the new size. Fails if freeBlockEnd != heapEnd and returns 0 in this case.

	bool SaveState(unsigned id);							//Saves the current allocation state to the state list. Returns 1 if successful, 0 if there was not enough memory to allocate the state.
	bool LoadState(unsigned id);							//Tries to revert the heap to the state given by id and returns 1 if successful. If the state could not be found, it returns 0.

	void Reset(unsigned params);							//Resets ('deallocates') the heap. If params & 1 == 1, ResetStart is called; If params & 2 == 1, ResetEnd is called. Consequently, both functions are called if params == 3.
	void ResetStart();										//Resets freeBlockBegin to heapStart and sets state to null (all states are unlinked).
	void ResetEnd();										//Resets freeBlockEnd and blockEnd of each state to heapEnd.

	static void* AllocateForwards(void* freeBlockPair, unsigned size, unsigned align);
	static void* AllocateBackwards(void* freeBlockPair, unsigned size, unsigned align);

	SolidHeapAllocator(void* heapEnd, unsigned flags);

};


//Abstract heap base class
struct Heap								//internal name: mHeap::Heap_t
{
	void* heapStart;					//Pointer to the heap's start
	unsigned heapSize;					//Heap area containing allocator and allocated data
	Heap* parentHeap;					//Pointer to the heap's parent. May be null if it's the root heap.
	unsigned flags;						//Only flag is 0x4000 (which is set in ctor) which allows crashing the game on a fatal heap exception if set.

	static SolidHeap* CreateSolidHeap(unsigned size, Heap* root, int align);								//Allocates a SolidHeap on root with size size (except heap object members) and alignment/allocation direction align. Returns a pointer to the newly created heap.
	static ExpandingHeap* CreateExpandingHeap(unsigned size, Heap* root, int align);						//Allocates an ExpandingHeap on root with size size (except heap object members) and alignment/allocation direction align. Returns a pointer to the newly created heap.
	static SolidHeapAllocator* CreateSolidHeapAllocator(void* address, unsigned size, unsigned flags);
	static ExpandingHeapAllocator* CreateExpandingHeapAllocator(void* address, unsigned size, unsigned flags);
	static ExpandingHeap* CreateRootHeap(void* address, unsigned size);										//Creates the root heap (and therefore doesn't allocate it on another heap which doesn't exist at this point)
	static void SetupRootHeap();																			//Sets up the root heap's bounds, creates it and registers it to its respective global pointers.
	static void InitializeRootHeap();																		//Sets rootParamOffset to 0 and calls SetupRootHeap.
	static void InitializeGameHeap(unsigned size, Heap* root);												//Forwards the arguments to CreateExpandingHeap and sets gameHeapPtr to the new game heap pointer.
	static void* SetupSolidHeapAsDefault(unsigned size, Heap* root, int align);								//Creates a SolidHeap, saves the current default heap to tmpHeapPtr and sets the newly created heap as default; Returns a pointer to the new heap or null if CreateSolidHeap failed to allocate space for it.
	static void* InitializeSolidHeapAsDefault(unsigned size, Heap* root, int align);						//Calls SetupSolidHeapAsDefault
	static void RestoreFromTemporary();																		//Sets the temporary heap to the default heap and resets tmpHeapPtr to 0.

	Heap(void* start, unsigned size, Heap* root);
	virtual ~Heap();

	virtual void VDestroy() = 0;											//Destroys the heap and removes the allocator
	virtual void* VAllocate(unsigned size, int align) = 0;					//Allocates size bytes with an alignment/allocation direction of align
	virtual bool VDeallocate(void* ptr) = 0;								//Deallocates ptr from the heap.
	virtual void VDeallocateAll() = 0;										//Deallocates everything from the heap.
	virtual bool VIntact() = 0;												//Returns 1 if the heap represents a valid object. Not very reliable to check if the heap is broken...
	virtual void VRescue() = 0;												//Does nothing.
	virtual unsigned VReallocate(void* ptr, unsigned newSize) = 0;			//Reallocates the memory given by ptr with the size of newSize. Returns 0 in case the reallocation failed.
	virtual unsigned VSizeof(void* ptr) = 0;								//Returns the size of an allocated block
	virtual unsigned VMaxAllocationUnitSize() = 0;							//Returns the maximum size that is allocatable at once
	virtual unsigned VMaxAllocatableSize() = 0;								//Returns the size of the largest contiguous free memory block
	virtual unsigned VMemoryLeft() = 0;										//Returns the number of unallocated bytes
	virtual unsigned VSetNodeID(unsigned id) = 0;							//Sets the assignable node ID for subsequent nodes. Returns the previous node ID.
	virtual unsigned VGetNodeID() = 0;										//Returns the current node ID.
	virtual unsigned VResizeToFit() = 0;									//Resizes the heap to fit (e.g. deallocates all unused memory). Returns the new size or 0 if it failed.

	void Destroy();															//Calls VDestroy and Deallocate
	void _Destroy();														//Calls Destroy, always called outside from Heap (thunk to Destroy)
	void* Allocate(unsigned size, int align);								//Calls VAllocate
	void* Allocate(unsigned size);											//Calls Allocate with align = 4
	void* _Allocate(unsigned size, int align);								//Calls Allocate (thunk to Allocate)
	bool Deallocate(void* ptr);												//Calls VDeallocate
	bool _Deallocate(void* ptr);											//Calls Deallocate (thunk to Deallocate)
	bool Intact();															//Calls VIntact and sets heapDamaged
	void Rescue();															//Calls VRescue
	unsigned Reallocate(void* ptr, unsigned newSize);						//Calls VReallocate
	unsigned Sizeof(void* ptr);												//Calls VSizeof
	unsigned _Sizeof(void* ptr);											//Calls Sizeof (thunk to Sizeof)
	unsigned MaxAllocationUnitSize();										//Calls VMaxAllocationUnitSize
	unsigned SetNodeID(unsigned id);										//Calls VSetNodeID
	unsigned ResizeToFit();													//Calls VResizeToFit

	Heap* SetDefault();														//Sets the heap to the current default one and returns the previous default heap

};


//Double linked list heap
struct ExpandingHeap : public Heap		//internal name: mHeap::ExpHeap_t
{
	ExpandingHeapAllocator* allocator;

	ExpandingHeap(void* start, unsigned size, Heap* root, ExpandingHeapAllocator* allocator);
	virtual ~ExpandingHeap();

	virtual void VDestroy() override;
	virtual void* VAllocate(unsigned size, int align) override;
	virtual bool VDeallocate(void* ptr) override;
	virtual void VDeallocateAll() override;
	virtual bool VIntact() override;
	virtual void VRescue() override;
	virtual unsigned VReallocate(void* ptr, unsigned newSize) override;
	virtual unsigned VSizeof(void* ptr) override;
	virtual unsigned VMaxAllocationUnitSize() override;						//Calls ExpandingHeapAllocator::MaxAllocatableSize()
	virtual unsigned VMaxAllocatableSize() override;
	virtual unsigned VMemoryLeft() override;
	virtual unsigned VSetNodeID(unsigned id) override;
	virtual unsigned VGetNodeID() override;
	virtual unsigned VResizeToFit() override;								//Always fails (forbidden operation)

};


//Linear heap
struct SolidHeap : public Heap			//internal name: mHeap::SolidHeap_t
{
	SolidHeapAllocator* allocator;

	SolidHeap(void* start, unsigned size, Heap* root, SolidHeapAllocator* allocator);
	virtual ~SolidHeap();

	virtual void VDestroy() override;
	virtual void* VAllocate(unsigned size, int align) override;
	virtual bool VDeallocate(void* ptr) override;							//Crashes
	virtual void VDeallocateAll() override;
	virtual bool VIntact() override;
	virtual void VRescue() override;
	virtual unsigned VReallocate(void* ptr, unsigned newSize) override;		//Only allowed if ptr was the last allocation
	virtual unsigned VSizeof(void* ptr) override;							//Crashes and returns -1
	virtual unsigned VMaxAllocationUnitSize() override;						//Calls SolidHeapAllocator::MemoryLeft()
	virtual unsigned VMaxAllocatableSize() override;						//Calls SolidHeapAllocator::MemoryLeft()
	virtual unsigned VMemoryLeft() override;
	virtual unsigned VSetNodeID(unsigned id) override;						//Simply returns 0 (since SolidHeap has no need for ID's)
	virtual unsigned VGetNodeID() override;									//Simply returns 0 (since SolidHeap has no need for ID's)
	virtual unsigned VResizeToFit() override;

};

static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ == 4, "Compile with the -faligned-new=4 flag");

/*
	This is the namespace of memory functions you might use directly which are always safe. Since there are multiple layers between the allocation call and the heap's
	VAllocate consisting of many thunked functions, the code and call chain is heavily documented here and in Heap in case one tries to optimize the game.
*/
namespace Memory
{

	extern bool heapDamaged;					//True if any heap returns Intact() with 0
	extern ExpandingHeap* rootHeapPtr;			//Pointer to the root heap
	extern Heap* defaultHeapPtr;				//Pointer to the heap used in default allocation functions
	extern unsigned rootParamOffset;			//Offset from param section (0x027FFDA0+)
	extern Heap* tmpHeapPtr;					//Temporary Heap pointer
	extern ExpandingHeap* gameHeapPtr;			//Pointer to the game heap
	extern bool isRootHeapIterInitialized;		//True if the root heap iterator has been initialized
	extern NestedHeapIterator rootHeapIterator;	//Iterator containing the root heap
	extern void* nextRootHeapStart;				//Pointer to the next heap start (mirrored from 0x027x)
	extern void* rootHeapEnd;					//Pointer to the root heap end (mirrored from 0x027x)

	//Calls to Heap::Allocate(size, align)
	void* Allocate(unsigned size, int align, Heap* heap);				//Basic Allocate, if heap = 0 the default heap is used
	
	//Calls to Heap::Allocate(size)
	void* malloc(unsigned size);										//Base for operator_new functions, allocates on root heap (standard malloc)

	//Calls to Memory::Allocate(size, align, heap)
	//void* ActorBase::operator_new(unsigned size)						//Defined in ActorBase, calls with align = -4 and heap = gameHeap
	//void* operator_new(unsigned size, unsigned align)					//May be defined with C++17, but most likely you want to use
	void* Allocate(unsigned size, int align);							//Allocates on root heap (heap = 0)
	void* Allocate(unsigned size);										//Allocates on root heap (heap = 0) and aligns with align = 4

	//Calls to malloc(size)
	//operator_new calls are interchangeable
	//void* operator_new(unsigned size);								//Standard operator_new, size is always a constant expression (caller-deduced)
	void* operator_new2(unsigned size);									//Behaves exactly like operator_new, but size is always runtime-dependent (caller-deduced)

	//Calls to Heap::Deallocate(ptr)
	bool Deallocate(void* ptr, Heap* heap);								//Basic Deallocate, if heap == 0 the default heap is used

	//Calls to Heap::_Deallocate(ptr)
	void free(void* ptr);												//Frees memory (standard free)

	//Calls to free(ptr)
	//operator_delete calls are interchangeable
	//void operator_delete(void* ptr);									//Standard operator_delete
	void operator_delete2(void* ptr);									//Internal operator_delete (most often in virtual dtors)

	//Calls to Memory::Deallocate(ptr, heap)
	void Deallocate(void* ptr);											//Calls Deallocate on root heap (heap = 0)

	//Custom functions simplifying reallocation
	inline unsigned Reallocate(void* ptr, unsigned size, Heap* heap)
	{
		if (!heap) heap = Memory::defaultHeapPtr;

#ifdef SM64DS_SAFE_REALLOC
		if (!Memory::defaultHeapPtr)
			return 0;
#endif

		return heap->Reallocate(ptr, size);

	}

	inline void* realloc(void* ptr, unsigned size)
	{
#ifdef SM64DS_SAFE_REALLOC
		if (!Memory::defaultHeapPtr) return nullptr;
#endif

		if (!Memory::defaultHeapPtr->Reallocate(ptr, size))
			return nullptr;
		else
			return ptr;
	}

}

#endif	// SM64DS_MEMORY_INCLUDED