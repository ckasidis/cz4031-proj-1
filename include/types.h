#ifndef TYPES_H
#define TYPES_H

#include "node.h"

struct Record
{
    // 12 Bytes
    char tconst[12];
    // 4 Bytes
    float averageRating;
    // 4 Bytes
    unsigned int numVotes;
};

#endif // TYPES_H
