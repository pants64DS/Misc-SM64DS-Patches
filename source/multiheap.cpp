#include "Memory.h"
#include <new>
#include <array>

#define ITCM_ARENA_START     0x01ffdf40
#define MAIN_RAM_START       0x02000000
#define MAIN_RAM_CODE_START  0x02004000
#define LEVEL_OVERLAY_START  0x0214eaa0
#define INSERTED_CODE_START  0x02156aa0
#define DTCM_START           0x023c0000
#define DTCM_END             0x023c4000
#define ARM7_ARENA_START     0x023d80e0
#define FLASHCARD_CODE_START 0x023fc000

#define STR(x) #x
#define REGION(name, start, end) extern char name[(end) - (start)]; \
asm(#name " = " STR(start));

REGION(UNUSED_START_OF_RAM,      MAIN_RAM_START,      MAIN_RAM_CODE_START)
REGION(RAM_BEFORE_INSERTED_CODE, LEVEL_OVERLAY_START, INSERTED_CODE_START)
REGION(UNUSED_END_OF_RAM,        DTCM_END,            FLASHCARD_CODE_START)

struct MemoryRange
{
	char* start;
	char* end;

	template<unsigned size>
	constexpr MemoryRange(char(&arr)[size]):
		start(arr),
		end(arr + size)
	{}

	constexpr bool Contains(const void* ptr) const
	{
		static constexpr std::less<const void*> less = {};

		return !less(ptr, start) && less(ptr, end);
	}

	constexpr ExpandingHeap& GetHeap() const
	{
		return *std::launder(reinterpret_cast<ExpandingHeap*>(start));
	}

	void ConstuctHeap() const
	{
		char* const heapStart = start + sizeof(ExpandingHeap);
		const unsigned heapSize = end - heapStart;

		auto* allocator = Heap::CreateExpandingHeapAllocator(heapStart, heapSize, 3);
		if (!allocator) Crash();

		new (start) ExpandingHeap(heapStart, heapSize, nullptr, allocator);
	}
};

struct ExtraHeapIterator
{
	const MemoryRange* memRange;

	constexpr bool operator==(const ExtraHeapIterator& other) const = default;

	constexpr ExpandingHeap& operator*() const
	{
		return memRange->GetHeap();
	}

	constexpr ExtraHeapIterator& operator++()
	{
		++memRange;

		return *this;
	}
};

struct ExtraHeaps
{
	std::array<MemoryRange, 3> memRanges
	{
		UNUSED_START_OF_RAM,
		RAM_BEFORE_INSERTED_CODE,
		UNUSED_END_OF_RAM
	};

	void Init()
	{
		InitFileSystem();

		OverlayInfo ovInfo;
		unsigned maxLevelOverlaySize = 0;

		for (unsigned i = 0; i < 52; ++i)
		{
			unsigned ovID = i + 103;

			if (!LoadOverlayInfo(ovInfo, false, ovID))
				Crash();

			const unsigned overlaySize = ovInfo.loadSize + ovInfo.bssSize;

			if (maxLevelOverlaySize < overlaySize)
				maxLevelOverlaySize = overlaySize;
		}

		memRanges[1].start += maxLevelOverlaySize;

		const bool momExists = LoadOverlayInfo(ovInfo, false, 155);

		if (momExists)
		{
			const unsigned momOverlaySize = ovInfo.loadSize + ovInfo.bssSize;

			memRanges[2].start += momOverlaySize;
		}

		for (const MemoryRange& memRange : memRanges)
			memRange.ConstuctHeap();
	}

	constexpr ExtraHeapIterator begin()
	{
		return {memRanges.begin()};
	}

	constexpr ExtraHeapIterator end()
	{
		return {memRanges.end()};
	}
}
constinit extraHeaps;

class MultiHeap : public ExpandingHeap
{
	ExpandingHeap& GetHeap(const void* ptr)
	{
		for (const MemoryRange& extraHeapMemRange : extraHeaps.memRanges)
		{
			if (extraHeapMemRange.Contains(ptr))
				return extraHeapMemRange.GetHeap();
		}

		return *this;
	}

public:
	MultiHeap(void* start, unsigned size, Heap* root, ExpandingHeapAllocator* allocator);

	virtual void VDestroy() override
	{
		ExpandingHeap::VDestroy();

		for (ExpandingHeap& extraHeap : extraHeaps)
			extraHeap.Destroy();
	}

	virtual void* VAllocate(unsigned size, int align) override
	{
		for (ExpandingHeap& extraHeap : extraHeaps)
		{
			void* res = extraHeap.ExpandingHeap::VAllocate(size, align);

			if (res) return res;
		}

		return ExpandingHeap::VAllocate(size, align);
	}

	virtual bool VDeallocate(void* ptr) override
	{
		return GetHeap(ptr).ExpandingHeap::VDeallocate(ptr);
	}

	virtual void VDeallocateAll() override
	{
		ExpandingHeap::VDeallocateAll();

		for (ExpandingHeap& extraHeap : extraHeaps)
			extraHeap.ExpandingHeap::VDeallocateAll();
	}

	virtual unsigned VReallocate(void* ptr, unsigned newSize) override
	{
		return GetHeap(ptr).ExpandingHeap::VReallocate(ptr, newSize);
	}

	virtual unsigned VSizeof(const void* ptr) override
	{
		return GetHeap(ptr).ExpandingHeap::VSizeof(ptr);
	}

	virtual unsigned VMaxAllocationUnitSize() override
	{
		unsigned maxSize = ExpandingHeap::VMaxAllocationUnitSize();

		for (ExpandingHeap& extraHeap : extraHeaps)
		{
			const unsigned newSize = extraHeap.ExpandingHeap::VMaxAllocationUnitSize();

			if (maxSize < newSize)
				maxSize = newSize;
		}

		return maxSize;
	}

	virtual unsigned VMaxAllocatableSize() override
	{
		unsigned maxSize = ExpandingHeap::VMaxAllocatableSize();

		for (ExpandingHeap& extraHeap : extraHeaps)
		{
			const unsigned newSize = extraHeap.ExpandingHeap::VMaxAllocatableSize();

			if (maxSize < newSize)
				maxSize = newSize;
		}

		return maxSize;
	}

	virtual unsigned VMemoryLeft() override
	{
		unsigned res = ExpandingHeap::VMemoryLeft();

		for (ExpandingHeap& extraHeap : extraHeaps)
			res += extraHeap.ExpandingHeap::VMemoryLeft();

		return res;
	}
};

[[gnu::flatten]]
MultiHeap::MultiHeap(void* start, unsigned size, Heap* root, ExpandingHeapAllocator* allocator):
	ExpandingHeap(start, size, root, allocator)
{
	extraHeaps.Init();
}

static_assert(sizeof(MultiHeap) == sizeof(ExpandingHeap));

asm("nsub_0201a0d8 = 0x0201a0dc");
asm("repl_0203c948 = _ZN9MultiHeapC1EPvjP4HeapP22ExpandingHeapAllocator");
