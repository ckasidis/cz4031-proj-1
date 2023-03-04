#include "storage.h"

/* constructor */
Storage::Storage(std::size_t maxStorageSize, std::size_t blockSize) : numAllocatedBlocks(0),
                                                                      numBlocksAccessed(0),
                                                                      numBlocksForRecords(0),
                                                                      storagePtr(operator new(maxStorageSize)),
                                                                      blockSize(blockSize),
                                                                      usedStorageSize(0),
                                                                      actualUsedStorageSize(0),
                                                                      curBlockUsedSize(0),
                                                                      maxStorageSize(maxStorageSize)
{
    std::memset(storagePtr, '\0', maxStorageSize);
}

/* public methods */
bool Storage::allocateBlock(bool record)
{
    if (record)
    {
        ++numBlocksForRecords;
    }

    if (usedStorageSize + blockSize <= maxStorageSize)
    {
        curBlockPtr = static_cast<char *>(storagePtr) + (numAllocatedBlocks * blockSize);
        usedStorageSize += blockSize;
        curBlockUsedSize = 0;
        ++numAllocatedBlocks;
        return true;
    }
    else
    {
        std::cout << "Error: Block size exceeds current available memory\n";
        return false;
    }
}

Address Storage::allocate(std::size_t sizeRequired, bool record)
{
    unsigned short int extraSizeUsed;

    if (actualUsedStorageSize + sizeRequired > maxStorageSize || sizeRequired > blockSize)
    {
        std::cout << "Required size too large!\n";
        std::exit(0);
    }

    void *blockAddress;
    unsigned short int offset;

    if (!record)
    {
        bool allocatedSuccessful = allocateBlock(record);
        if (!allocatedSuccessful)
        {
            throw std::logic_error("Failed to allocate new curBlockPtr!");
        }

        blockAddress = curBlockPtr;
        offset = curBlockUsedSize;
    }

    else
    {
        if (numAllocatedBlocks == 0)
        {
            bool allocatedSuccessful = allocateBlock(record);
            if (!allocatedSuccessful)
            {
                throw std::logic_error("Failed to allocate new curBlockPtr!");
            }
        }

        if (curBlockUsedSize < blockSize)
        {
            blockAddress = curBlockPtr;
            offset = curBlockUsedSize;
        }
        else
        {
            blockAddress = static_cast<void *>(static_cast<char *>(curBlockPtr) + blockSize);
            offset = 0;
        }

        if (curBlockUsedSize + sizeRequired > blockSize)
        {
            unsigned short int extraSizeUsed = curBlockUsedSize + sizeRequired - blockSize;
            bool allocatedSuccessful = allocateBlock(record);
            if (!allocatedSuccessful)
            {
                throw std::logic_error("Failed to allocate new curBlockPtr!");
            }

            curBlockUsedSize += extraSizeUsed;
        }
        else
        {
            curBlockUsedSize += sizeRequired;
        }
    }

    actualUsedStorageSize += sizeRequired;
    Address recordAddress = {blockAddress, offset};
    return recordAddress;
}

bool Storage::deallocate(Address addressToBeDeleted, std::size_t sizeToDelete)
{
    actualUsedStorageSize -= sizeToDelete;

    std::memset(static_cast<char *>(addressToBeDeleted.blockAddress) + addressToBeDeleted.offset, 0, sizeToDelete);

    if (sizeToDelete < blockSize)
    {
        unsigned char testBlock[blockSize];
        if (std::memcmp(testBlock, addressToBeDeleted.blockAddress, blockSize) == 0)
        {
            usedStorageSize -= blockSize;
            --numAllocatedBlocks;
        }
    }
    else
    {
        const unsigned int blocksToMinus = (std::floor(addressToBeDeleted.offset + sizeToDelete) / blockSize) - (addressToBeDeleted.offset == 0 ? 0 : 1);
        numAllocatedBlocks -= blocksToMinus;
        usedStorageSize -= blocksToMinus * blockSize;
    }

    return true;
}

void *Storage::load(Address address, std::size_t size)
{
    void *dataAddress = operator new(size);
    std::memcpy(dataAddress, static_cast<char *>(address.blockAddress) + address.offset, size);

    ++numBlocksAccessed;

    if (size > blockSize - curBlockUsedSize)
    {
        ++numBlocksAccessed;
    }

    return dataAddress;
}

Address Storage::save(void *itemAddress, std::size_t size)
{
    Address diskAddress = allocate(size, true);
    std::memcpy(static_cast<char *>(diskAddress.blockAddress) + diskAddress.offset, itemAddress, size);

    ++numBlocksAccessed;

    return diskAddress;
}

Address Storage::save(void *itemAddress, std::size_t size, Address diskAddress)
{
    std::memcpy(static_cast<char *>(diskAddress.blockAddress) + diskAddress.offset, itemAddress, size);

    ++numBlocksAccessed;

    return diskAddress;
}

int Storage::resetBlocksAccessed()
{
    int tempBlocksAccessed = numBlocksAccessed;
    numBlocksAccessed = 0;
    return tempBlocksAccessed;
}

/* getters */
int Storage::getNumAllocatedBlocks() const
{
    return numAllocatedBlocks;
};

int Storage::getNumAllocatedBlocksWithoutPadding() const
{
    unsigned int blocksUsedForIndex = numAllocatedBlocks - numBlocksForRecords;
    std::size_t actualSpaceUsedForRecordsWithoutPadding = ceil(((numBlocksForRecords * getBlockSize()) - numBlocksForRecords) / getBlockSize());
    std::size_t actualBlockUsedWithoutPadding = actualSpaceUsedForRecordsWithoutPadding + blocksUsedForIndex;
    return numAllocatedBlocks;
};

int Storage::getNumBlocksAccessed() const
{
    return numBlocksAccessed;
}

int Storage::getNumBlocksForRecords() const
{
    return numBlocksForRecords;
}

void *Storage::getStoragePtr()
{
    return storagePtr;
}

std::size_t Storage::getBlockSize() const
{
    return blockSize;
}

std::size_t Storage::getUsedStorageSize() const
{
    return usedStorageSize;
}

std::size_t Storage::getCurBlockUsedSize() const
{
    return curBlockUsedSize;
}

std::size_t Storage::getActualUsedStorageSize() const
{
    return actualUsedStorageSize;
}

std::size_t Storage::getActualUsedStorageSizeWithoutPadding() const
{
    std::size_t blocksUsedForIndex = numAllocatedBlocks - numBlocksForRecords;
    std::size_t actualRecordSizeWithoutPadding = (numBlocksForRecords * getBlockSize() / 20) * 19;
    std::size_t actualTotalSizeWithoutPadding = actualRecordSizeWithoutPadding + blocksUsedForIndex * getBlockSize();
    return actualTotalSizeWithoutPadding;
}

std::size_t Storage::getMaxStorageSize() const
{
    return maxStorageSize;
}

/* destructor */
Storage::~Storage(){};
