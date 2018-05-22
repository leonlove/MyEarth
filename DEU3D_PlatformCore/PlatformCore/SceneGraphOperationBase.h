#ifndef SCENE_GRAPH_OPERATION_BASE_H_20717016_A202_42A7_9227_383ABD8E7921_INCLUDE
#define SCENE_GRAPH_OPERATION_BASE_H_20717016_A202_42A7_9227_383ABD8E7921_INCLUDE

#include <OpenSP/sp.h>
#include <osg/Node>

class SceneGraphOperator;

class SceneGraphOperationBase : public OpenSP::Ref
{
public:
    explicit SceneGraphOperationBase(void){}
    virtual ~SceneGraphOperationBase(void) = 0 {}

protected:
    osg::Node *getTerrainRootNode(SceneGraphOperator *pOperator);
    osg::Node *getTempElementRootNode(SceneGraphOperator *pOperator);
    osg::Node *getCultureRootNode(SceneGraphOperator *pOperator);

public:
    virtual bool doAction(SceneGraphOperator *) = 0;
};



#endif
