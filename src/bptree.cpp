#include "bptree.h"

/* constructor */
BPTree::BPTree(int nodeSize) : nodeSize(nodeSize), rootNode(nullptr) {}

/* public methods */
Address BPTree::queryNumVotesKey(int key, int &nodesUpdated)
{
    Node *cursor = rootNode;
    nodesUpdated = 1;
    int blockCount = 1;

    std::cout << "Index root node accessed: " << std::endl;
    rootNode->printNode();

    if (rootNode == nullptr)
    {
        std::cout << "BPTree does not exist. instantiate rootNode" << std::endl;
        return {nullptr, 0};
    }

    while (!cursor->isLeaf)
    {

        nodesUpdated += 1;
        int insertionIndex = cursor->binarySearchInsertIndex(key);

        if (insertionIndex == -1)
        {
            insertionIndex = cursor->binarySearch(key);
            cursor = cursor->childrenNodes[insertionIndex + 1].getAddressNode();
        }
        else
        {
            cursor = cursor->childrenNodes[insertionIndex].getAddressNode();
        }
        if (blockCount < 5)
        {

            blockCount++;
        }
    }

    int index;
    for (int i = 0; i < cursor->currentKeyNum; i++)
    {
        if (cursor->keys[i] >= key)
        {
            index = i;
            break;
        }
    }
    return cursor->childrenNodes[index];
}

int BPTree::findMinimumKey(Node *node)
{
    if (node->isLeaf)
    {
        return node->keys[0];
    }
    return findMinimumKey(node->childrenNodes[0].getAddressNode());
}

void BPTree::linkLeaf()
{
    std::vector<Address> leafNodes;
    int nodeCount = 1;

    DFS(rootNode->addressInDisk, leafNodes, nodeCount);

    for (int i = 0, j = 1; j < leafNodes.size(); i++, j++)
    {
        leafNodes.at(i).getAddressNode()->linkOtherLeaf(leafNodes.at(j));
    }
    std::cout << "Finished linking leaf nodes" << std::endl;
}

void BPTree::DFS(Address currentNode, std::vector<Address> &recordList, int &nodeCount)
{
    int addressZeroCounter = 0;
    std::queue<Address> childrenNodesToSearch;
    if (currentNode.getAddressNode()->isLeaf)
    {
        nodeCount++;
        if (currentNode.getAddressNode())

            recordList.push_back(currentNode);

        return;
    }

    nodeCount += currentNode.getAddressNode()->currentPointerNum;

    for (int i = 0; i < currentNode.getAddressNode()->currentPointerNum; i++)
    {
        childrenNodesToSearch.push(currentNode.getAddressNode()->childrenNodes[i]);
    }
    while (!childrenNodesToSearch.empty())
    {
        Address childrenNodeToTraverse = childrenNodesToSearch.front();
        childrenNodesToSearch.pop();
        DFS(childrenNodeToTraverse, recordList, nodeCount);
    }
}

int BPTree::findHeight(Node *rootNode)
{
    int height = 1;
    Node *cursor = rootNode;

    while (!cursor->isLeaf)
    {
        int insertionIndex = cursor->binarySearchInsertIndex(0);
        cursor = cursor->childrenNodes[insertionIndex].getAddressNode();
        height++;
    }

    std::cout << "height of B+ Tree is: " << height << std::endl;

    return height;
}

// insert
Address *BPTree::insert(Node *parentNode, int key, Address address, Storage &disk)
{
    if (rootNode == nullptr)
    {
        rootNode = new Node(nodeSize, true);
        rootNode->addressInDisk = disk.allocate(disk.getBlockSize(), false);
        rootNode->keys[0] = key;

        rootNode->childrenNodes[0] = address;
        rootNode->currentPointerNum++;
        rootNode->currentKeyNum++;

        disk.save(rootNode, disk.getBlockSize());

        return nullptr;
    }
    else
    {
        int insertionIndex = parentNode->binarySearchInsertIndex(key);

        if (insertionIndex == -1)
        {
            std::cout << "Duplicate key: " << key << std::endl;
            return nullptr;
        }

        if (parentNode->isLeaf)
        {
            if (parentNode->isFull())
            {
                Address leftNodeAddress = disk.allocate(disk.getBlockSize(), false);
                Address rightNodeAddress = disk.allocate(disk.getBlockSize(), false);

                Node *leftNode = new Node(nodeSize, true);
                leftNode->addressInDisk = leftNodeAddress;
                Node *rightNode = new Node(nodeSize, true);
                rightNode->addressInDisk = rightNodeAddress;

                int minimumKeySizeRight = (int)floor((nodeSize / 2));
                int minimumKeySizeLeft = nodeSize - minimumKeySizeRight;

                int virtualKeyArray[nodeSize + 1];
                std::memset(virtualKeyArray, 0, sizeof(virtualKeyArray));
                Address virtualPointerArray[nodeSize + 1];
                std::memset(virtualPointerArray, 0, sizeof(virtualPointerArray));

                for (int i = 0; i < nodeSize; i++)
                {
                    virtualKeyArray[i] = parentNode->keys[i];
                    virtualPointerArray[i] = parentNode->childrenNodes[i];
                }

                for (int i = nodeSize; i > insertionIndex; i--)
                {
                    virtualPointerArray[i] = virtualPointerArray[i - 1];
                    virtualKeyArray[i] = virtualKeyArray[i - 1];
                }

                virtualKeyArray[insertionIndex] = key;
                virtualPointerArray[insertionIndex] = address;

                int virtualKeyCounter = 0;
                int virtualPtrCounter = 0;
                int newNodesKeyCounter = 0;

                while (newNodesKeyCounter < minimumKeySizeLeft)
                {
                    leftNode->keys[newNodesKeyCounter] = virtualKeyArray[virtualKeyCounter];
                    (leftNode->childrenNodes)[newNodesKeyCounter] = virtualPointerArray[virtualPtrCounter];

                    newNodesKeyCounter++;
                    virtualKeyCounter++;
                    virtualPtrCounter++;
                    leftNode->currentKeyNum++;
                    leftNode->currentPointerNum++;
                }

                newNodesKeyCounter = 0;
                while (newNodesKeyCounter < minimumKeySizeRight + 1)
                {
                    rightNode->keys[newNodesKeyCounter] = virtualKeyArray[virtualKeyCounter];
                    (rightNode->childrenNodes)[newNodesKeyCounter] = virtualPointerArray[virtualPtrCounter];

                    newNodesKeyCounter++;
                    virtualPtrCounter++;
                    virtualKeyCounter++;
                    rightNode->currentKeyNum++;
                    rightNode->currentPointerNum++;
                }

                Address testLeftAddress = disk.save(leftNode, disk.getBlockSize(), leftNodeAddress);
                Address testRightAddress = disk.save(rightNode, disk.getBlockSize(), rightNodeAddress);

                if (parentNode == rootNode)
                {
                    Node *newParentNode = new Node(nodeSize, false);
                    newParentNode->insertInitialInNonLeafNode(rightNode->keys[0], leftNodeAddress, rightNodeAddress);

                    Address newParentAddress = disk.allocate(disk.getBlockSize(), false);

                    disk.save(newParentNode, disk.getBlockSize(), newParentAddress);

                    rootNode = newParentAddress.getAddressNode();
                    rootNode->addressInDisk = newParentAddress;
                    return nullptr;
                }

                return new Address[2]{leftNodeAddress, rightNodeAddress};
            }
            else
            {
                parentNode->insertSubsequentPair(key, address);
                return nullptr;
            }
        }
        else
        {

            Address childNodeAddress = parentNode->childrenNodes[insertionIndex];
            Node *childNode = childNodeAddress.getAddressNode();

            Address *returnedChildSubTrees = insert(childNode, key, address, disk);

            if (returnedChildSubTrees == nullptr)
            {
                return nullptr;
            }

            Node *leftChildSubTree = returnedChildSubTrees[0].getAddressNode();
            Node *rightChildSubTree = returnedChildSubTrees[1].getAddressNode();

            int keyToInsertIntoParent;

            if (rightChildSubTree->isLeaf)
            {
                keyToInsertIntoParent = rightChildSubTree->keys[0];
            }
            else
            {
                keyToInsertIntoParent = rightChildSubTree->removeFirstKeyFromNode();
            }

            if (parentNode->isFull())
            {
                Node *leftNode = new Node(nodeSize, false);
                Node *rightNode = new Node(nodeSize, false);

                Address leftNodeAddress = disk.allocate(disk.getBlockSize(), false);
                Address rightNodeAddress = disk.allocate(disk.getBlockSize(), false);
                leftNode->addressInDisk = leftNodeAddress;
                rightNode->addressInDisk = rightNodeAddress;

                disk.save(leftNode, disk.getBlockSize(), leftNodeAddress);
                disk.save(rightNode, disk.getBlockSize(), rightNodeAddress);

                leftNode = leftNodeAddress.getAddressNode();
                rightNode = rightNodeAddress.getAddressNode();

                int minimumKeySizeRight = (nodeSize / 2);
                int minimumKeySizeLeft = nodeSize - minimumKeySizeRight;

                int virtualKeyArray[nodeSize + 1];
                std::memset(virtualKeyArray, 0, sizeof(virtualKeyArray));
                Address virtualPointerArray[nodeSize + 2];
                std::memset(virtualPointerArray, 0, sizeof(virtualPointerArray));

                int i = 0;
                for (; i < nodeSize; i++)
                {
                    virtualKeyArray[i] = parentNode->keys[i];
                    virtualPointerArray[i] = parentNode->childrenNodes[i];
                }
                virtualPointerArray[i] = parentNode->childrenNodes[i];

                for (int i = nodeSize; i > insertionIndex; i--)
                {
                    virtualKeyArray[i] = virtualKeyArray[i - 1];
                }

                virtualKeyArray[insertionIndex] = keyToInsertIntoParent;
                virtualPointerArray[insertionIndex] = returnedChildSubTrees[0];

                for (int i = nodeSize + 1; i > insertionIndex + 1; i--)
                {
                    virtualPointerArray[i] = virtualPointerArray[i - 1];
                }
                virtualPointerArray[insertionIndex + 1] = returnedChildSubTrees[1];

                int virtualKeyCounter = 0;
                int virtualPtrCounter = 0;
                int newNodesKeyCounter = 0;

                while (newNodesKeyCounter < minimumKeySizeLeft)
                {
                    leftNode->keys[newNodesKeyCounter] = virtualKeyArray[virtualKeyCounter];
                    (leftNode->childrenNodes)[newNodesKeyCounter] = virtualPointerArray[virtualPtrCounter];

                    newNodesKeyCounter++;
                    virtualKeyCounter++;
                    virtualPtrCounter++;
                    leftNode->currentKeyNum++;
                    leftNode->currentPointerNum++;
                }

                (leftNode->childrenNodes)[newNodesKeyCounter] = virtualPointerArray[virtualPtrCounter];

                leftNode->currentPointerNum++;

                newNodesKeyCounter = 0;
                while (newNodesKeyCounter < minimumKeySizeRight + 1)
                {
                    rightNode->keys[newNodesKeyCounter] = virtualKeyArray[virtualKeyCounter];
                    (rightNode->childrenNodes)[newNodesKeyCounter] = virtualPointerArray[virtualPtrCounter];

                    newNodesKeyCounter++;
                    virtualPtrCounter++;
                    virtualKeyCounter++;
                    rightNode->currentKeyNum++;
                    rightNode->currentPointerNum++;
                }

                (rightNode->childrenNodes)[newNodesKeyCounter] = virtualPointerArray[virtualKeyCounter];
                rightNode->currentPointerNum++;

                if (parentNode == rootNode)
                {
                    int parentInsertionKey = rightNode->removeFirstKeyFromNode();

                    Node *newRoot = new Node(nodeSize, false);
                    Address newRootAddress = disk.allocate(disk.getBlockSize(), false);
                    newRoot->addressInDisk = newRootAddress;
                    disk.save(newRoot, disk.getBlockSize(), newRootAddress);

                    newRootAddress.getAddressNode()->insertInitialInNonLeafNode(parentInsertionKey, leftNodeAddress, rightNodeAddress);

                    rootNode = newRootAddress.getAddressNode();
                    rootNode->addressInDisk = newRootAddress;

                    return nullptr;
                }

                return new Address[2]{leftNodeAddress, rightNodeAddress};
            }
            else
            {
                parentNode->childrenNodes[insertionIndex] = returnedChildSubTrees[0];
                parentNode->insertSubsequentPair(keyToInsertIntoParent, returnedChildSubTrees[1]);

                return nullptr;
            }
        }
    }
}

// remove
void BPTree::remove(int key, int &nodesDeleted, int &nodesUpdated, int &height, Storage &disk)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    nodesDeleted = 0;
    nodesUpdated = 0;

    height = findHeight(rootNode);
    Address AddressDeleted;
    int minkeyLeaf, minkeyNonLeaf, minptLeaf, minptNonLeaf;

    minkeyLeaf = (int)floor((nodeSize + 1) / 2);
    minptLeaf = minkeyLeaf + 1;
    minkeyNonLeaf = (int)floor(nodeSize / 2);
    minptNonLeaf = minkeyNonLeaf + 1;

    if (!rootNode)
    {
        std::cout << "The B+ Tree is Empty" << std::endl;
        return;
    }

    if (rootNode->isLeaf)
    {
        int index = rootNode->binarySearch(key);
        if (index == -1)
        {
            std::cout << "Key not found!" << std::endl;
            return;
        }
        else
        {
            AddressDeleted = rootNode->remove(index);
            if (rootNode->currentKeyNum == 0)
            {
                nodesDeleted++;
                disk.deallocate(rootNode->addressInDisk, disk.getBlockSize());
            }
        }
    }

    std::cout << std::endl;

    std::stack<Node *> stack;
    Node *current = rootNode;
    Node *parent = NULL;

    int index = -1;
    stack.push(current);

    while (!current->isLeaf)
    {
        index = -1;
        if (current->currentKeyNum == 1)
        {

            if (key < current->keys[0])
            {
                parent = (current->childrenNodes[0]).getAddressNode();
                index = 0;
                stack.push(NULL);
                stack.push(current->childrenNodes[1].getAddressNode());
            }

            else
            {
                parent = (current->childrenNodes[1]).getAddressNode();
                index = 0;
                stack.push((current->childrenNodes[0]).getAddressNode());
                stack.push(NULL);
            }
        }

        else
        {
            if (!current->isLeaf)
            {
                for (int i = 0; i < current->currentKeyNum; i++)
                {
                    if (key > current->keys[i] && key <= current->keys[i + 1])
                    {
                        parent = (current->childrenNodes[i + 1]).getAddressNode();
                        stack.push((current->childrenNodes[i]).getAddressNode());
                        stack.push((current->childrenNodes[i + 2]).getAddressNode());
                        index = i;
                        break;
                    }
                }
            }
            else
            {
                index = current->binarySearch(key);
                stack.push(current->childrenNodes[index - 1].getAddressNode());
                stack.push(current->childrenNodes[index + 1].getAddressNode());
                parent = current->childrenNodes[index].getAddressNode();
                break;
            }
        }

        if (index == -1 && key >= current->keys[current->currentKeyNum - 1])
        {
            parent = (current->childrenNodes[current->currentPointerNum - 1]).getAddressNode();
            stack.push((current->childrenNodes[current->currentPointerNum - 2]).getAddressNode());
            stack.push(NULL);
        }

        else if (index == -1 && key < current->keys[0])
        {
            parent = (current->childrenNodes[0]).getAddressNode();
            stack.push(NULL);
            stack.push((current->childrenNodes[1]).getAddressNode());
        }

        stack.push(parent);
        current = parent;
    }

    index = current->binarySearch(key);

    if (index == -1)
    {
        std::cout << "Key not found!" << std::endl;
        return;
    }

    AddressDeleted = current->remove(index);

    Node *left = NULL;
    Node *right = NULL;

    current = stack.top();
    stack.pop();

    int minkeys, minpt;

    while (!stack.empty())
    {
        if (current->isLeaf)
        {
            minkeys = minkeyLeaf;
            minpt = minptLeaf;
        }
        else
        {
            minkeys = minkeyNonLeaf;
            minpt = minptNonLeaf;
        }

        if (current->currentKeyNum >= minkeys || current == rootNode)
        {
            if (key < current->keys[0])
            {

                stack.push(current);
                updateParent(stack, key, nodesUpdated);

                break;
            }
            else if (current == rootNode)
            {
                stack.push(current);
                updateParent(stack, key, nodesUpdated);
            }

            break;
        }

        if (!stack.empty())
        {
            right = stack.top();
            stack.pop();
        }
        else
        {
            right = NULL;
            break;
        }

        if (!stack.empty())
        {
            left = stack.top();
            stack.pop();
        }
        else
        {
            left = NULL;
            break;
        }

        if (left)
        {
            if (left->isLeaf)
            {
                minkeys = minkeyLeaf;
                minpt = minptLeaf;
            }
            else
            {
                minkeys = minkeyNonLeaf;
                minpt = minptNonLeaf;
            }

            if (left->currentKeyNum > minkeys && current->currentKeyNum < current->maxKeyNum)
            {
                if (!left->isLeaf)
                {
                    for (int k = current->currentKeyNum - 1; k > 0; k--)
                    {
                        current->keys[k] = current->keys[k - 1];
                    }

                    for (int k = current->currentPointerNum - 1; k > 0; k--)
                    {
                        current->childrenNodes[k] = current->childrenNodes[k - 1];
                    }
                }
                else
                {
                    for (int k = current->currentKeyNum; k > 0; k--)
                    {
                        current->keys[k] = current->keys[k - 1];
                        current->childrenNodes[k] = current->childrenNodes[k - 1];
                    }
                }

                current->keys[0] = left->keys[left->currentKeyNum - 1];
                left->keys[left->currentKeyNum - 1] = 0;
                current->childrenNodes[0] = left->childrenNodes[left->currentPointerNum - 1];
                left->childrenNodes[left->currentPointerNum - 1] = {nullptr, 0};

                left->currentKeyNum--;
                current->currentKeyNum++;

                current->currentPointerNum++;
                left->currentPointerNum--;
            }
            nodesUpdated++;
        }

        if (right)
        {
            if (right->isLeaf)
            {
                minkeys = minkeyLeaf;
                minpt = minptLeaf;
            }
            else
            {
                minkeys = minkeyNonLeaf;
                minpt = minptNonLeaf;
            }
            if (right->currentKeyNum > minkeys)
            {
                current->keys[current->currentKeyNum + 1] = right->keys[0];
                current->childrenNodes[current->currentPointerNum + 1] = right->childrenNodes[0];

                if (!right->isLeaf)
                {
                    for (int k = 0; k < right->currentKeyNum; k++)
                    {
                        right->keys[k] = right->keys[k + 1];
                    }
                    for (int k = 0; k < right->currentPointerNum; k++)
                    {

                        right->childrenNodes[k] = right->childrenNodes[k + 1];
                    }
                }
                else
                {
                    for (int k = 0; k < right->currentKeyNum; k++)
                    {
                        right->keys[k] = right->keys[k + 1];
                        right->childrenNodes[k] = right->childrenNodes[k + 1];
                    }
                }

                right->keys[right->currentKeyNum - 1] = 0;
                right->childrenNodes[right->currentPointerNum - 1] = {nullptr, 0};
                right->currentKeyNum--;
                right->currentPointerNum--;
                current->currentKeyNum++;
                current->currentPointerNum++;

                nodesUpdated++;
                parent = stack.top();
                // stack.pop();
                for (int m = 0; m < parent->currentPointerNum; m++)
                {
                    if (parent->childrenNodes[m].getAddressNode() == right)
                    {
                        parent->keys[m - 1] = findMinimumKey(right);
                        break;
                    }
                }
            }
        }

        if (left)
        {
            if (left->isLeaf)
            {
                minkeys = minkeyLeaf;
                minpt = minptLeaf;
            }
            else
            {
                minkeys = minkeyNonLeaf;
                minpt = minptNonLeaf;
            }

            if (left->currentKeyNum <= minkeys && left->currentKeyNum + current->currentKeyNum <= left->maxKeyNum)
            {
                int i = 0;
                for (int k = left->currentKeyNum; k < left->maxKeyNum; k++)
                {
                    left->keys[k] = current->keys[i];
                    current->keys[i] = 0;
                    i++;
                }
                if (left->isLeaf)
                {
                    i = 0;
                    for (int k = left->currentPointerNum; k < left->maxPointerNum; k++)
                    {
                        left->childrenNodes[k] = current->childrenNodes[i];
                        current->childrenNodes[i] = {nullptr, 0};
                        i++;
                    }
                    left->childrenNodes[left->maxPointerNum - 1] = current->childrenNodes[current->maxPointerNum - 1];
                }
                else
                {
                    i = 0;

                    for (int k = left->currentPointerNum; k < left->maxPointerNum; k++)
                    {
                        left->childrenNodes[k] = current->childrenNodes[i];
                        current->childrenNodes[i] = {nullptr, 0};
                        i++;
                    }
                }

                left->currentKeyNum += current->currentKeyNum;
                left->currentPointerNum += current->currentPointerNum;

                parent = stack.top();
                stack.pop();

                int min = left->keys[0];
                for (int m = 0; m < parent->currentPointerNum; m++)
                {
                    if (parent->childrenNodes[m].getAddressNode() == current)
                    {
                        for (int j = m; j < parent->currentPointerNum; j++)
                        {
                            parent->childrenNodes[j] = parent->childrenNodes[j + 1];
                            break;
                        }
                        for (int j = m; j < parent->currentKeyNum; j++)
                        {
                            parent->keys[j] = parent->keys[j];
                        }
                        if (!parent->isLeaf)
                        {
                            parent->keys[m] = left->keys[left->currentKeyNum];
                        }
                    }
                }

                parent->currentKeyNum--;
                parent->currentPointerNum--;
                nodesDeleted++;
                nodesUpdated++;

                disk.deallocate(current->addressInDisk, disk.getBlockSize());

                current = parent;
            }
        }

        else if (right)
        {
            if (right->isLeaf)
            {
                minkeys = minkeyLeaf;
                minpt = minptLeaf;
            }
            else
            {
                minkeys = minkeyNonLeaf;
                minpt = minptNonLeaf;
            }

            if (right->currentKeyNum <= minkeys && right->currentKeyNum + current->currentKeyNum <= current->maxKeyNum)
            {
                int i = 0;
                for (int k = current->currentKeyNum; k < current->maxKeyNum; k++)
                {
                    current->keys[k] = right->keys[i];
                    right->keys[i] = 0;
                    i++;
                }

                parent = stack.top();
                stack.pop();

                for (int m = 0; m < parent->currentPointerNum; m++)
                {

                    if (parent->childrenNodes[m].getAddressNode() == right)
                    {

                        for (int j = m; j < parent->currentPointerNum; j++)
                        {
                            parent->childrenNodes[j] = parent->childrenNodes[j + 1];
                            parent->keys[j - 1] = parent->keys[j];
                        }

                        break;
                    }

                    if (parent->childrenNodes[m].getAddressNode() == current)
                    {
                        parent->keys[m - 1] = findMinimumKey(current);
                    }
                }

                parent->currentKeyNum--;
                parent->currentPointerNum--;

                current = parent;
                nodesDeleted++;
                nodesUpdated++;

                disk.deallocate(right->addressInDisk, disk.getBlockSize());
            }
        }
    }
    height = findHeight(rootNode);
    std::cout << "Content of Parent Node:" << std::endl;
    rootNode->printNode();
    Node *child = rootNode->childrenNodes[0].getAddressNode();
    std::cout << "Content of Child Node:" << std::endl;
    child->printNode();
    disk.deallocate(AddressDeleted, disk.getBlockSize());

    auto endTime = std::chrono::high_resolution_clock::now();
    auto runningTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << std::endl
              << "Running time of delete: " << runningTime.count() << "ms" << std::endl;

    return;
}

void BPTree::updateParent(std::stack<Node *> stack, int key, int &nodesUpdated)
{

    Node *parent = NULL;
    Node *current = NULL;
    Node *left = NULL;
    Node *right = NULL;
    int index = -1, minimum = 9999;
    current = stack.top();
    stack.pop();
    while (!stack.empty())
    {
        stack.pop();
        stack.pop();
        parent = stack.top();
        stack.pop();
        index = -1;

        minimum = findMinimumKey(current);

        for (int m = 0; m < parent->currentPointerNum; m++)
        {
            if (parent->childrenNodes[m].getAddressNode() == current)
            {
                nodesUpdated++;
                parent->keys[m - 1] = minimum;
            }
        }

        current = parent;
    }
}

// search
int *BPTree::search(int lowKey, int highKey, Storage &disk)
{
    static int result[2];
    result[0] = 0;
    result[1] = 0;
    if (!rootNode)
    {
        std::cout << "The B+ Tree is Empty" << std::endl;
        return result;
    }

    // measure running time
    auto startTime = std::chrono::high_resolution_clock::now();

    Node *current = rootNode;

    bool end = false;
    unsigned int currentKey = lowKey;
    int indexNodesAcccessed = 0;

    Address start = queryNumVotesKey(lowKey, indexNodesAcccessed);

    void *startingBlock = start.blockAddress;
    unsigned short int startingOffset = start.offset;
    Record *record = (Record *)((char *)start.blockAddress + start.offset);

    int blockCount = 1;
    int recordCount = 0;
    float totalRating = 0.0;
    unsigned short int newOffset = start.offset;

    while (!end)
    {
        if (record->numVotes > highKey || blockCount > 5)
        {
            end = true;
            break;
        }
        else if (record->numVotes <= highKey)
        {
            if (record->numVotes >= lowKey)
            {
                std::cout << "********************Record Content " << recordCount + 1 << " at address: " << (void *)((char *)start.blockAddress + (start.offset)) << " ********************" << std::endl;
                std::cout << "Record address in disk: " << (void *)((char *)start.blockAddress + (start.offset)) << std::endl;
                std::cout << "NumVotes for current record: " << record->numVotes << std::endl;
                std::cout << "Average Rating for current record: " << record->averageRating << std::endl;
                std::cout << "tconst for current record: " << record->tconst << std::endl;
                recordCount++;
                totalRating += record->averageRating;
            }

            newOffset += sizeof(Record);
            if (newOffset % disk.getBlockSize() == 0)
            {
                blockCount++;
            }
            record = (Record *)((char *)start.blockAddress + newOffset);
        }
    }

    result[0] = indexNodesAcccessed;
    if (blockCount > 5)
    {
        blockCount = 5;
    }
    result[1] = blockCount;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto runningTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    std::cout << std::endl
              << "--- Summary ---" << std::endl;
    std::cout << "Running time of search: " << runningTime.count() << "ms" << std::endl;
    std::cout << "Number of records accessed: " << recordCount << std::endl;
    std::cout << "Average avgRating of the records: " << float(totalRating / (float)recordCount) << std::endl;

    return result;
}

// print
void BPTree::display()
{
    std::cout << "enter display" << std::endl;
    std::priority_queue<std::vector<Node *>> pq;

    std::vector<Node *> rootLevel = std::vector<Node *>{rootNode};
    pq.push(rootLevel);
    std::cout << "*********display() debug********" << std::endl;
    int level = 0;

    while (!pq.top().empty())
    {
        std::vector<Node *> currentLevel = (pq.top());
        pq.pop();

        std::cout << "level: " << level << std::endl;
        std::cout << "********************" << std::endl;

        std::vector<Node *> nextLevel;

        for (int i = 0; i < currentLevel.size(); i++)
        {

            Node *currentNode = currentLevel.at(i);

            std::cout << "Node " << i << "\n";
            std::cout << "Node address: " << currentNode << " \n";
            std::cout << "currentKeyNum: " << currentNode->currentKeyNum << "\n";
            std::cout << "currentPointerNum: " << currentNode->currentPointerNum << "\n";
            std::string nodeType = currentNode->isLeaf ? "LEAF" : "INTERNAL NODE";
            std::cout << "nodeType: " << nodeType << "\n";

            int j;
            std::cout << "keys: [";

            for (j = 0; j < currentNode->maxKeyNum; j++)
            {

                std::cout << currentNode->keys[j] << ",";

                if (currentNode->childrenNodes[j].getAddressNode() != nullptr && !currentNode->isLeaf)
                {
                    nextLevel.push_back(currentNode->childrenNodes[j].getAddressNode());
                }
            }

            if (currentNode->childrenNodes[j].getAddressNode() != nullptr && !currentNode->isLeaf)
            {
                nextLevel.push_back(currentNode->childrenNodes[j].getAddressNode());
            }

            std::cout << "]" << std::endl;

            std::cout << "Pointers: [";

            for (int i = 0; i < currentNode->maxPointerNum; i++)
            {
                std::cout << currentNode->childrenNodes[i].getAddressNode() << ",";
            }
            std::cout << "]" << std::endl;
            std::cout << std::endl;
        }

        pq.push(nextLevel);
        level++;
    }
    std::cout << "**** <Finished traversal> ****" << std::endl;
};

void BPTree::printDetails()
{
    // experiment 2
    std::vector<Address> leafNodes;
    int nodeCount = 1;
    DFS(rootNode->addressInDisk, leafNodes, nodeCount);
    std::cout << "******BPTREE DETAILS******" << std::endl;
    std::cout << "Parameter n of B+ Tree (number of keys in Node): " << nodeSize << std::endl;
    std::cout << "Total NodeSize: " << nodeCount << " nodes" << std::endl;

    int height = findHeight(rootNode);
    std::cout << "height of B+ Tree is: " << height << std::endl;

    std::cout << "\n\nContent Of Root Node:" << std::endl;
    rootNode->printNode();
    std::cout << "\n\nContent of First Child Of Root Node" << std::endl;
    ((Node **)rootNode->childrenNodes)[0]->printNode();
}

/* getters */
int BPTree::getNodeSize() const
{
    return nodeSize;
}

Node *BPTree::getRootNode() const
{
    return rootNode;
}

/* destructor */
BPTree::~BPTree(){};