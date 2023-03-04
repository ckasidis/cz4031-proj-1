#include "node.h"

/* constructor */
Node::Node(int nodeSize, bool isLeaf) : isLeaf(isLeaf),
                                        maxKeyNum(nodeSize),
                                        maxPointerNum(nodeSize + 1),
                                        keys(new int[this->maxKeyNum]{0}),
                                        childrenNodes(new Address[this->maxPointerNum]{{nullptr, 0}})
{
}

/* public methods */
bool Node::isFull()
{
    if (this->currentKeyNum == this->maxKeyNum)
    {
        return true;
    }

    return false;
}

void Node::linkOtherLeaf(Address anotherLeafNode)
{
    if (!this->isLeaf)
    {
        throw 1;
    }

    if (this->childrenNodes[this->maxPointerNum - 1].blockAddress != nullptr)
    {
        this->childrenNodes[this->maxPointerNum - 1] = anotherLeafNode;
        return;
    }

    this->childrenNodes[this->maxPointerNum - 1] = anotherLeafNode;
    this->currentPointerNum++;
}

// insert
void Node::insertInitialInNonLeafNode(int key, Address leftPointer, Address rightPointer)
{
    if (this->isLeaf || this->currentKeyNum != 0)
    {
        std::cout << "cannot insertNonLeafNodeInitialPairs with node with size != 0 or NonLeaf" << std::endl;
        throw 1;
    }

    this->insertKeyInKeyArray(key, 0);
    this->insertChildInPointerArray(leftPointer, 0);
    this->insertChildInPointerArray(rightPointer, 1);
}

void Node::insertInitialInLeafNode(int key, Address recordPointer, Address neighbourNode)
{
    if (!this->isLeaf || this->currentKeyNum != 0)
    {
        std::cout << "cannot insertLeafNodeInitialPairs with node with size != 0 or NonLeaf" << std::endl;
        throw 1;
    }

    this->insertKeyInKeyArray(key, 0);
    this->insertChildInPointerArray(recordPointer, 0);
    this->insertChildInPointerArray(neighbourNode, this->maxPointerNum - 1);
}

void Node::insertSubsequentPair(int key, Address nodeOrRecordPointer)
{
    if (this->isFull())
    {
        std::cout << "Key array is full" << std::endl;
        throw 1;
    }
    int insertionIndex = this->binarySearchInsertIndex(key);
    if (insertionIndex == -1)
    {
        std::cout << "duplicate key " << key << " detected and not added into the B+ Tree" << std::endl;
        return;
    }

    this->insertKeyInKeyArray(key, insertionIndex);

    if (this->isLeaf)
    {
        this->insertChildInPointerArray(nodeOrRecordPointer, insertionIndex);
    }
    else
    {
        this->insertChildInPointerArray(nodeOrRecordPointer, insertionIndex + 1);
    }
}

void Node::insertKeyInKeyArray(int key, int index)
{
    if (this->isFull())
    {
        std::cout << "Node Key array is filled! Need to split!" << std::endl;
        throw 1;
    };

    if (index == -1)
    {
        std::cout << "duplicate key " << key << " detected and not added into the B+ Tree" << std::endl;
        return;
    }

    int i;
    for (int i = this->currentKeyNum; i > index; i--)
    {
        (this->keys)[i] = (this->keys)[i - 1];
    }

    (this->keys)[index] = key;

    this->currentKeyNum++;
};

void Node::insertChildInPointerArray(Address child, int index)
{
    if (this->currentPointerNum == this->maxPointerNum)
    {
        std::cout << "Node pointer array is filled! Need to split!" << std::endl;
        throw 1;
    };

    if (this->isLeaf)
    {
        int i;

        int offset = 0;
        if (this->childrenNodes[this->maxPointerNum - 1].blockAddress != nullptr)
        {
            offset = 1;
        }

        for (i = this->currentPointerNum - offset; i > index; i--)
        {
            this->childrenNodes[i] = this->childrenNodes[i - 1];
        }

        this->childrenNodes[index] = child;
    }
    else
    {
        int i;
        for (i = currentPointerNum; i > index; i--)
        {
            this->childrenNodes[i] = this->childrenNodes[i - 1];
        }

        this->childrenNodes[index] = child;
    }

    this->currentPointerNum++;
};

// binary search
// returns -1 if key not found in the node
// only within the node
int Node::binarySearch(int key)
{
    int l = 0;
    int r = this->currentKeyNum - 1;
    int m;

    while (l <= r)
    {

        m = (r + l) / 2;
        if (this->keys[m] > key)
        {
            r = m - 1;
        }
        if (this->keys[m] < key)
        {
            l = m + 1;
        }
        if (this->keys[m] == key)
        {
            return m;
        }
    }

    return -1;
}

// returns -1 if duplicate key is found in the node
// returns the index to insert the value at
int Node::binarySearchInsertIndex(int key)
{
    int l = 0;
    int r = this->currentKeyNum - 1;
    int m;

    while (l <= r)
    {

        m = (r + l) / 2;
        if (this->keys[m] > key)
        {
            r = m - 1;
        }
        if (this->keys[m] < key)
        {
            l = m + 1;
        }
        if (this->keys[m] == key)
        {
            return -1;
        }
    }

    return l;
}

// remove
Address Node::remove(int index)
{
    if (this->currentKeyNum == 0)
    {
        throw 1;
    }
    Address answer = this->childrenNodes[index];

    for (int i = index; i < this->currentKeyNum; i++)
    {
        this->keys[i] = this->keys[i + 1];
        this->childrenNodes[i] = this->childrenNodes[i + 1];
    }

    this->keys[currentKeyNum] = 0;
    this->childrenNodes[currentKeyNum] = {nullptr, 0};
    this->currentKeyNum--;
    this->currentPointerNum--; // the pointer to the vector has to be removed

    return answer;
}

int Node::removeFirstKeyFromNode()
{
    int removedKey = this->keys[0];
    if (this->isLeaf)
    {
        std::cout << "cannot remove first key-pointer pair from a leaf node" << std::endl;
        throw 1;
    }

    for (int i = 0; i < this->currentKeyNum; i++)
    {
        this->keys[i] = this->keys[i + 1];
    }

    for (int i = 0; i < this->currentKeyNum + 1; i++)
    {
        this->childrenNodes[i] = this->childrenNodes[i + 1];
    }
    this->currentKeyNum--;
    this->currentPointerNum--;
    return removedKey;
};

// print
void Node::printNode()
{
    std::cout << "**********************" << std::endl;
    std::cout << "Node's Disk address: " << this->addressInDisk.getAddressNode() << std::endl;
    std::cout << "currentKeyNum :" << this->currentKeyNum << std::endl;
    std::cout << "currentPointerNum :" << this->currentPointerNum << std::endl;
    std::string nodeType = this->isLeaf ? "LEAF" : "InternalNode";
    std::cout << "node type :" << nodeType << std::endl;
    std::cout << "max Key Size: " << this->maxKeyNum << std::endl;
    std::cout << "keyArray: [";

    for (int i = 0; i < this->maxKeyNum; i++)
    {
        std::cout << this->keys[i] << ",";
    };
    std::cout << "]" << std::endl;
    std::cout << "PointerArray: [";
    for (int i = 0; i < this->maxPointerNum; i++)
    {
        std::cout << this->childrenNodes[i].getAddressNode() << ",";
    };
    std::cout << "]" << std::endl;
    std::cout << "**********************" << std::endl;
}

/* getters */
int Node::getCurrentKeyNum()
{
    return this->currentKeyNum;
};

/* destructor */
Node::~Node(){};