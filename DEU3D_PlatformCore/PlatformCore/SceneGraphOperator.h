#ifndef SCENE_GRAPH_OPERATOR_H_7436AE62_4AC9_468D_8566_01194870447D_INCLUDE
#define SCENE_GRAPH_OPERATOR_H_7436AE62_4AC9_468D_8566_01194870447D_INCLUDE

#include <osg/Referenced>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <deque>
#include <OpenThreads/Mutex>

#include "SceneGraphOperationBase.h"

class SceneGraphOperator : public osg::NodeCallback
{
    friend  class SceneGraphOperationBase;
public:
    explicit SceneGraphOperator(void);
    virtual ~SceneGraphOperator(void);

public:
    bool    initialize(osg::Node *pSceneRootNode);
    void    pushOperation(SceneGraphOperationBase *pOperation);

protected:
    SceneGraphOperationBase *takeOperation(void);
    void    feedbackOperation(SceneGraphOperationBase *pOperation);

protected:
    virtual void operator()(osg::Node *, osg::NodeVisitor *pNodeVisitor);

protected:
    osg::Node      *m_pTerrainRootNode;
    osg::Node      *m_pCultureRootNode;
    osg::Node      *m_pTempElementRootNode;

    std::deque<osg::ref_ptr<SceneGraphOperationBase> >  m_queueOperations;
    OpenThreads::Mutex                                  m_mtxOperations;

protected:

};

#endif

