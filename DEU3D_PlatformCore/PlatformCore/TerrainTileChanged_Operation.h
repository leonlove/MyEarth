#ifndef TERRAIN_TILE_CHANGED_OPERATION_H_B82942DB_22F2_4055_9EA6_295FEE631667_INCLUDE
#define TERRAIN_TILE_CHANGED_OPERATION_H_B82942DB_22F2_4055_9EA6_295FEE631667_INCLUDE

#include "SceneGraphOperationBase.h"

class TerrainTileChanged_Operation : public SceneGraphOperationBase
{
    virtual bool doAction(SceneGraphOperator *pOperator) = 0;
};


#endif
