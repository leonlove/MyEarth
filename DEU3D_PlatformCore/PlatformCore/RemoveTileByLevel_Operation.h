#ifndef REMOVE_TILE_BY_LEVEL_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE
#define REMOVE_TILE_BY_LEVEL_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE

#include "SceneGraphOperationBase.h"
#include <osg/NodeVisitor>

class RemoveTileByLevel_Operation : public SceneGraphOperationBase
{
public:
    explicit RemoveTileByLevel_Operation(unsigned int nLevel) : m_nLevel(nLevel){}
    virtual ~RemoveTileByLevel_Operation(void){}

public:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    unsigned int m_nLevel;
};

#endif