#ifndef REPLACE_CHILDREN_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE
#define REPLACE_CHILDREN_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE

#include "SceneGraphOperationBase.h"

class ReplaceChildren_Operation : public SceneGraphOperationBase
{
public:
    explicit ReplaceChildren_Operation(void) {}

    virtual ~ReplaceChildren_Operation(void) {}

public:
    void addReplacePair(osg::Node *pChild, osg::Node *pNewChild);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    OpenThreads::Mutex m_mtxReplace;
    std::vector<std::pair<osg::ref_ptr<osg::Node>, osg::ref_ptr<osg::Node> > > m_vecReplaceList;
};

#endif