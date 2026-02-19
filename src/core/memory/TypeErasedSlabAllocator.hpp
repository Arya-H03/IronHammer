#pragma once
#include <cassert>
#include <cstddef>
#include <new>
#include <stddef.h>
#include <vector>

struct Slot
{
    Slot* nextFreeSlot;       // Points to next free slot
    alignas(64) char data[1]; // Placeholder for alignment
};

class TypeErasedSlabAllocator
{
  private:

    std::vector<void*> m_slabs; // Raw slab blocks
    size_t m_dataSize;          // Size of the data that needs to be stored
    size_t m_slotPerSlab;
    Slot* m_freeList;

    void AllocateNewSlab()
    {
        size_t slabSize = m_slotPerSlab * (sizeof(Slot) - 1 + m_dataSize); // Size of each slab block
        void* slab = ::operator new(slabSize, std::align_val_t(64));       // Get a ptr to a block of memory
        m_slabs.push_back(slab);

        // Init free list
        char* ptr = static_cast<char*>(slab); // Cast to operate on the slab as a block of 1Byte chars
        for (size_t i = 0; i < m_slotPerSlab; ++i)
        {
            Slot* slot = reinterpret_cast<Slot*>(ptr);
            slot->nextFreeSlot =
                (i < m_slotPerSlab - 1) ? reinterpret_cast<Slot*>(ptr + (sizeof(Slot) - 1 + m_dataSize)) : nullptr;
            ptr += sizeof(Slot) - 1 + m_dataSize;
        }
        m_freeList = static_cast<Slot*>(slab);
    }

    void* InternalAllocate()
    {
        if (m_freeList == nullptr) AllocateNewSlab();

        Slot* slot = m_freeList;
        m_freeList = m_freeList->nextFreeSlot;
        return slot->data; // Return ptr to usable memory
    }

    void InternalDeallocate(void* ptr)
    {
        // ptr points inside data[], recover ComponentSlot*
        Slot* slot = reinterpret_cast<Slot*>(reinterpret_cast<char*>(ptr) - offsetof(Slot, data));
        slot->nextFreeSlot = m_freeList;
        m_freeList = slot;
    }

  public:

    TypeErasedSlabAllocator(size_t dataSize, size_t slotsPerSlab = 64)
        : m_dataSize(dataSize), m_slotPerSlab(slotsPerSlab)
    {
    }

    ~TypeErasedSlabAllocator()
    {
        for (void* slab : m_slabs) ::operator delete(slab, std::align_val_t(64));
    }

    template <typename T, typename... Args>
    T* Allocate(Args&&... args)
    {
        assert(sizeof(T) == m_dataSize && "Wrong type was given to allocate");

        void* memory = InternalAllocate();                  // Find memory
        return new (memory) T(std::forward<Args>(args)...); // Forward data to memory
    }

    template <typename T>
    void Deallocate(T* obj)
    {
        assert(sizeof(T) == m_dataSize && "Wrong type was given to deallocate");

        obj->~T();
        InternalDeallocate(obj);
    }
};
