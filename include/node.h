#ifndef NODE_H
#define NODE_H

#include <iostream>

class Node;
struct Address;

struct Address
{
    // 8 Bytes
    void *blockAddress;
    // 4 Bytes
    // unsigned short int offset;
    unsigned int offset;

public:
    Node *getAddressNode()
    {
        return (Node *)((char *)blockAddress + offset);
    };
};

class Node
{
    friend class BPTree;

private:
    /* private attributes */
    int currentKeyNum{0};
    int currentPointerNum{0};
    int maxKeyNum{0};
    int maxPointerNum{0};
    int *keys;
    bool isLeaf;
    Address *childrenNodes{nullptr};
    Address addressInDisk{nullptr};

public:
    /* constructors */
    Node(int nodeSize, bool isLeaf);
    /* public methods */
    bool isFull();
    void linkOtherLeaf(Address child);
    // insert
    void insertInitialInNonLeafNode(int key, Address leftPointer, Address rightPointer);
    void insertInitialInLeafNode(int key, Address recordPointer, Address neighbourNode);
    void insertSubsequentPair(int key, Address NodeOrRecordPointer);
    void insertKeyInKeyArray(int key, int index);
    void insertChildInPointerArray(Address child, int index);
    // binary search
    int binarySearchInsertIndex(int key);
    int binarySearch(int remove);
    // remove
    Address remove(int remove);
    int removeFirstKeyFromNode();
    // print
    void printNode();
    /* getters */
    int getCurrentKeyNum();
    /* destructor */
    ~Node();
};

#endif // NODE_H
