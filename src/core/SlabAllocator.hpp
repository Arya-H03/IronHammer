#include <bit>
#include <cassert>
#pragma once

template<typename T, size_t SlabSize>
class SlabAllocator
{
	static_assert(SlabSize > 0, "SlotCount must be positive");

private:

	struct Slab
	{
		static constexpr size_t wordCount = (SlabSize + 63) / 64;

		union
		{
			uint64_t single;
			uint64_t multiple[wordCount];
		}bitSet; // 1 -> free bit // 0 -> used bit 

		T blocks[SlabSize];
		size_t freeCount;	

		Slab()
		{
			ResetBitSetToOne();
			MaskInvalidBits();
			freeCount = SlabSize;
		}

		void SetBitToZero(size_t index)
		{
			if constexpr (wordCount == 1)
			{
				bitSet.single &= ~(1ull << index);
			}
			else
			{
				size_t wordIndex = index / 64;
				size_t bitIndex = index % 64;

				bitSet.multiple[wordIndex] &= ~(1ull << bitIndex);
			}
		}

		void SetBitsetToZero()
		{
			if constexpr (wordCount == 1)bitSet.single = 0ull;
			else for (size_t i = 0; i < wordCount; ++i) bitSet.multiple[i] = 0ull;
		}

		void ResetBitToOne(size_t index)
		{
			if constexpr (wordCount == 1)
			{
				bitSet.single |= 1ull << index;
			}
			else
			{
				size_t wordIndex = index / 64;
				size_t bitIndex = index % 64;

				bitSet.multiple[wordIndex] |= 1ull << bitIndex;
			}
		}

		void ResetBitSetToOne()
		{
			if constexpr (wordCount == 1) bitSet.single = ~0ull;
			else for (size_t i = 0; i < wordCount; ++i) bitSet.multiple[i] = ~0ull;
		}

		//For when SlabSize if not a multiple of 64
		void MaskInvalidBits()
		{
			if constexpr (wordCount > 1)
			{
				constexpr size_t validBits = SlabSize % 64;
				if (validBits > 0)
				{
					bitSet.multiple[wordCount - 1] &= ((1ull << validBits) - 1);
				}
			}
		}

		size_t FindFirstFreeBit()
		{
			if constexpr (wordCount == 1)
			{
				if (bitSet.single != 0) return std::countr_zero(bitSet.single);
			}
			else
			{
				for (size_t i = 0; i < wordCount; ++i)
				{
					if (bitSet.multiple[i] != 0)
					{
						return (i * 64) + std::countr_zero(bitSet.multiple[i]);
					}
				}
			}

			assert(false && "No free bit found");
			return SIZE_MAX;
		}
	};

	std::vector<Slab> slabs;
	//Index of the Slab with at least one free block
	size_t currentSlabIndex = SIZE_MAX;

	size_t FindFreeSlab()
	{
		for (size_t i = 0; i < slabs.size(); ++i)
		{
			if (slabs[i].freeCount > 0) return i;
		}

		return SIZE_MAX;
	}

	size_t FindEmptySlab()
	{
		for (size_t i = 0; i < slabs.size(); ++i)
		{
			if (slabs[i].freeCount == SlabSize) return i;
		}

		return SIZE_MAX;
	}

	std::pair<size_t, size_t> LocateBlock(T* ptr)
	{
		for (size_t i = 0; i < slabs.size(); ++i)
		{
			auto base = slabs[i].blocks;

			if (ptr >= base && ptr < base + SlabSize) return { i, size_t(ptr - base) };

		}
		assert(false && "Pointer wans't found");
	}

	size_t LocateSlab(T* arrayPtr)
	{
		for (size_t i = 0; i < slabs.size(); ++i) if (arrayPtr == slabs[i].blocks) return i;

		assert(false && "Pointer wans't found");
	}

public:

	T* Allocate()
	{
		//When no free Slabs Exist
		if (currentSlabIndex = SIZE_MAX || slabs[currentSlabIndex].freeCount == 0)
		{
			currentSlabIndex = FindFreeSlab();
			if (currentSlabIndex == SIZE_MAX)
			{
				slabs.emplace_back();
				currentSlabIndex = slabs.size() - 1;
			}
		}

		auto& slab = slabs[currentSlabIndex];
		size_t freeIndex = slab.FindFirstFreeBit();
		slab.SetBitToZero(freeIndex);
		--slab.freeCount;
		return &slab.blocks[freeIndex];
	}

	T* AllocateArray()
	{
		//When no free Slabs Exist
		if (currentSlabIndex = SIZE_MAX || slabs[currentSlabIndex].freeCount != SlabSize)
		{
			currentSlabIndex = FindEmptySlab();
			if (currentSlabIndex == SIZE_MAX)
			{
				slabs.emplace_back();
				currentSlabIndex = slabs.size() - 1;
			}
		}

		auto& slab = slabs[currentSlabIndex];
		slab.SetBitsetToZero();
		slab.freeCount = 0;
		return slab.blocks;
	}

	void DeAllocate(T* ptr)
	{
		auto [slabIndex, blockIndex] = LocateBlock(ptr);
		slabs[slabIndex].ResetBitToOne(blockIndex);
		++slabs[slabIndex].freeCount;
		currentSlabIndex = slabIndex;
	}

	void DeAllocateArray(T* arrayPtr)
	{
		size_t arrayIndex = LocateSlab(arrayPtr);
		slabs[arrayIndex].ResetBitSetToOne();
		slabs[arrayIndex].freeCount = SlabSize;

		//This sets index to an entire empty slab. There might be cases in the futher that it should be set to the first Free slab.
		currentSlabIndex = arrayIndex;	
	}
};

