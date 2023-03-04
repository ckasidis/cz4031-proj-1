#ifndef BPTREE_H
#define BPTREE_H

#include "types.h"
#include "storage.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <queue>
#include <vector>
#include <stack>
#include <cmath>
#include <cstring>
#include <algorithm>

class BPTree
{
    friend class Node;

private:
    /* private attributes */
    int nodeSize;
    Node *rootNode{nullptr};

public:
    static Storage *storage;
    /* constructor */
    BPTree(int nodeSize);
    /* public methods */
    Address queryNumVotesKey(int key, int &nodesUpdated);
    int findMinimumKey(Node *node);
    void linkLeaf();
    void DFS(Address currentNode, std::vector<Address> &recordList, int &nodeCount);
    int findHeight(Node *rootNode);
    // insert
    Address *insert(Node *parentNode, int key, Address recordAddress, Storage &disk);
    // remove
    void remove(int key, int &nodesDeleted, int &nodesUpdated, int &height, Storage &disk);
    void updateParent(std::stack<Node *> stack, int key, int &nodesUpdated);
    // search
    int *search(int lowKey, int highKey, Storage &disk);
    // print
    void display();
    void printDetails();
    /* getters */
    int getNodeSize() const;
    Node *getRootNode() const;
    /* destructor */
    ~BPTree();
};

#endif // BPTREE_H