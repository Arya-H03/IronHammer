#pragma once
#include <cassert>
#include <cstddef>
#include <new>
#include <stddef.h>
#include <vector>

class TypeErasedBlockAllocator
{
  private:
    std::vector<void*> m_allBlocks;  // ptr to all memory blocks
    std::vector<void*> m_freeBlocks; // ptr to free memory blocks
    size_t m_dataSize;               // Size of the data that needs to be stored
    size_t m_dataPerBlock;

    void AllocateNewBlock()
    {
        void* newBlock = operator new(m_dataPerBlock * m_dataSize, std::align_val_t(64));
        m_allBlocks.push_back(newBlock);
        m_freeBlocks.push_back(newBlock);
    }

  public:
    TypeErasedBlockAllocator(size_t dataSize, size_t dataPerBlock = 64) : m_dataSize(dataSize), m_dataPerBlock(dataPerBlock)
    {
    }

    ~TypeErasedBlockAllocator()
    {
        for (void* block : m_allBlocks) ::operator delete(block, std::align_val_t(64));
    }

    size_t GetDataSize()
    {
        return m_dataSize;
    }

    void* AllocateBlock()
    {
        if (m_freeBlocks.empty()) AllocateNewBlock();

        void* freeBlock = m_freeBlocks.back();
        m_freeBlocks.pop_back();
        return freeBlock;
    }

    void DeallocateBlock(void* ptr)
    {
#ifndef NDEBUG
        bool found = false;
        for (void* block : m_allBlocks)
            if (block == ptr)
            {
                found = true;
                break;
            }
        assert(found && "Pointer not found in this allocator");
#endif
        m_freeBlocks.push_back(ptr);
    }
};
