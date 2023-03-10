#include "include/types.h"
#include "include/storage.h"
#include "include/bptree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
#include <functional>
#include <algorithm>
#include <math.h>
#include <inttypes.h>

int BLOCKSIZE = 200;
int DISK_CAPACITY = 500000000;
struct tempRecord {
    char tconst[12]; 
    float averageRating; 
    unsigned int numVotes;
    bool operator<(const tempRecord &rec) const {
        return (numVotes < rec.numVotes);
    }
};

int main() {
    int NodeSize = floor((BLOCKSIZE * 8 - 80 - 127 - 64) / 96);

    Storage disk(DISK_CAPACITY, BLOCKSIZE);
    std::ifstream file("./data/data.tsv");
    std::vector<tempRecord> recordList;

    std::cout << "Loading records into storage..." << std::endl;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            tempRecord newRec;
            std::stringstream datastream(line);
            std::string data;
            std::getline(datastream, data, '\t');
            if (data == "tconst") {
                continue;
            }
            strcpy(newRec.tconst, data.c_str());
            datastream >> newRec.averageRating >> newRec.numVotes;
            recordList.push_back(newRec);
        }
    }

    std::sort(recordList.begin(), recordList.end());

    std::vector<Address> addressList;
    std::vector<unsigned int> keyList;

    int recordCounter = 0;
    for (int i = 0; i < recordList.size(); i++) {
        Record newRec;
        std::copy(std::begin(recordList[i].tconst), std::end(recordList[i].tconst), std::begin(newRec.tconst));
        newRec.averageRating = recordList[i].averageRating;
        newRec.numVotes = recordList[i].numVotes;
        Address recAddress = disk.save(&newRec, sizeof(Record));

        if (i == 0 || recordList[i].numVotes != recordList[i - 1].numVotes)
        {
            addressList.push_back(recAddress);
            keyList.push_back(newRec.numVotes);
        }
        recordCounter++;
    }
    
    std::cout << "Creating B+ Tree..." << std::endl;
    BPTree *tree = new BPTree(NodeSize);
    for (int i = 0; i < addressList.size(); i++) {
        tree->insert(tree->getRootNode(), keyList[i], addressList[i], disk);
    }
    tree->linkLeaf();
    std::cout << std::endl;

    //////////////////
    // Experiment 1 //
    //////////////////
    std::cout << "##### Experiment 1 #####" << std::endl;
    std::cout << "Number of records: " << recordCounter << std::endl;
    std::cout << "Size of a record: " << sizeof(Record) << " Bytes" << std::endl;
    std::cout << "Number of records stored in a curBlockPtr: " << disk.getBlockSize()/sizeof(Record) << std::endl;
    std::cout << "Number of blocks for storing the data: " << disk.getNumBlocksForRecords() << std::endl;
    std::cout << std::endl << std::endl;

    //////////////////
    // Experiment 2 //
    //////////////////
    std::cout << "##### Experiment 2 #####" << std::endl;
    tree->printDetails();
    std::cout << std::endl << std::endl;

    //////////////////
    // Experiment 3 //
    //////////////////
    std::cout << "##### Experiment 3 #####" << std::endl;
    std::cout << "Retrieving records with numVotes = 500..." << std::endl;
    std::cout << std::endl;

    int *result;
    result = tree->search(500, 500, disk);
    
    std::cout << "Number of index nodes accessed: " << *(result + 0) << std::endl;
    std::cout << "Number of data blocks accesses: " << *(result + 1) << std::endl;
    std::cout << std::endl << std::endl;

    disk.resetBlocksAccessed();

    auto startTime = std::chrono::high_resolution_clock::now();
    for (unsigned int i=0; i < (BLOCKSIZE * disk.getNumBlocksForRecords()); i+= sizeof(Record)) {
        Record* diskIterator = (Record *)disk.load({disk.getStoragePtr(), i }, sizeof(Record));
        if (diskIterator->numVotes == 500) {
            break;
        }
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto runningTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << std::endl
              << "Running time of brute-force linear scan : " << runningTime.count() << "microseconds" << std::endl;

    std::cout << "Number of blocks accessed: " << disk.getNumBlocksAccessed()/10 << std::endl;

    //////////////////
    // Experiment 4 //
    //////////////////
    std::cout << "##### Experiment 4 #####" << std::endl;
    std::cout << "Retrieving records with numVotes from 30K to 40K..." << std::endl;
    std::cout << std::endl;

    int *result4;
    result4 = tree->search(30000, 40000, disk);

    std::cout << "Number of index blocks accessed: " << *(result4 + 0) << std::endl;
    std::cout << "Number of data blocks accessed: " << *(result4 + 1) << std::endl;
    std::cout << std::endl << std::endl;

    disk.resetBlocksAccessed();

    auto startTime1 = std::chrono::high_resolution_clock::now();
    for (unsigned int i=0; i < (BLOCKSIZE * disk.getNumBlocksForRecords()); i+= sizeof(Record)) {
        Record* diskIterator = (Record *)disk.load({disk.getStoragePtr(), i }, sizeof(Record));
        if (diskIterator->numVotes == 40000) {
            break;
        }
    }
    auto endTime1 = std::chrono::high_resolution_clock::now();
    auto runningTime1 = std::chrono::duration_cast<std::chrono::microseconds>(endTime1 - startTime1);
    std::cout << std::endl
              << "Running time of brute-force linear scan : " << runningTime1.count() << "microseconds" << std::endl;
    std::cout << "Number of blocks accessed: " << disk.getNumBlocksAccessed()/10 << std::endl;

    //////////////////
    // Experiment 5 //
    //////////////////
    std::cout << "##### Experiment 5 #####" << std::endl;
    std::cout << "Deleting records with numVotes = 1000..." << std::endl;
    std::cout << std::endl;

    disk.resetBlocksAccessed();
    auto startTime2 = std::chrono::high_resolution_clock::now();
    for (unsigned int i=0; i < (BLOCKSIZE * disk.getNumBlocksForRecords()); i+= sizeof(Record)) {
        Record* diskIterator = (Record *)disk.load({disk.getStoragePtr(), i }, sizeof(Record));
        if (diskIterator->numVotes == 1000) {
            break;
        }
    }

    auto endTime2 = std::chrono::high_resolution_clock::now();
    auto runningTime2 = std::chrono::duration_cast<std::chrono::microseconds>(endTime2 - startTime2);
    std::cout << std::endl
              << "Running time of brute-force linear scan : " << runningTime2.count() << "microseconds" << std::endl;

    std::cout << "Number of blocks accessed: " << disk.getNumBlocksAccessed()/10 << std::endl;

    int numNodesDeleted = 0;
    int numNodesUpdated = 0;
    int height = 0;
    tree->remove(1000, numNodesDeleted, numNodesUpdated, height, disk);

    std::cout << "Number of levels of the updated B+ tree: " << height << std::endl << std::endl;
    tree->printDetails();

    std::cout << "##### End #####" << std::endl;
    return 0;
}
