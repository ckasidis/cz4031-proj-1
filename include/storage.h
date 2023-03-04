#ifndef STORAGE_H
#define STORAGE_H

#include "types.h"
#include <cstring>
#include <cmath>

class Storage
{
private:
    /* private attributes */
    int numAllocatedBlocks;
    int numBlocksAccessed;
    int numBlocksForRecords;
    void *curBlockPtr;
    void *storagePtr;
    std::size_t blockSize;
    std::size_t usedStorageSize;
    std::size_t actualUsedStorageSize;
    std::size_t curBlockUsedSize;
    std::size_t maxStorageSize;

public:
    /* constructor */
    Storage(std::size_t maxStorageSize, std::size_t blockSize);
    /* public methods */
    bool allocateBlock(bool record);
    Address allocate(std::size_t sizeRequired, bool record);
    bool deallocate(Address address, std::size_t sizeToDelete);
    void *load(Address address, std::size_t size);
    Address save(void *itemAddress, std::size_t size);
    Address save(void *itemAddress, std::size_t size, Address diskAddress);
    int resetBlocksAccessed();
    /* getters */
    int getNumAllocatedBlocks() const;
    int getNumAllocatedBlocksWithoutPadding() const;
    int getNumBlocksAccessed() const;
    int getNumBlocksForRecords() const;
    void *getStoragePtr();
    std::size_t getBlockSize() const;
    std::size_t getUsedStorageSize() const;
    std::size_t getCurBlockUsedSize() const;
    std::size_t getActualUsedStorageSize() const;
    std::size_t getActualUsedStorageSizeWithoutPadding() const;
    std::size_t getMaxStorageSize() const;
    /* destructor */
    ~Storage();
};

#endif // STORAGE_H
