#ifndef ADD_OR_REMOVE_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE
#define ADD_OR_REMOVE_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE

#include "SceneGraphOperationBase.h"

#include <osgDB/ReadFile>

class AddTempModelByID_Operation : public SceneGraphOperationBase
{
public:
    explicit AddTempModelByID_Operation(const ID &id) : m_id(id){}

    virtual ~AddTempModelByID_Operation(void){}

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        osg::ref_ptr<osg::Node> pNode = osgDB::readNodeFile(m_id);

        osg::Group *pGroup = getTempElementRootNode(pOperator)->asGroup();

        if(!pNode.valid() || pGroup == NULL)
        {
            return false;
        }

        pGroup->addChild(pNode.get());

        return true;
    }

protected:
    const ID m_id;
};

class AddTempModelByNode_Operation : public SceneGraphOperationBase
{
public:
    explicit AddTempModelByNode_Operation(osg::Node *pNode) : m_pNode(pNode){}

    virtual ~AddTempModelByNode_Operation(void){}

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        osg::Group *pGroup = getTempElementRootNode(pOperator)->asGroup();

        if(!m_pNode.valid() || pGroup == NULL)
        {
            return false;
        }

        pGroup->addChild(m_pNode.get());

        return true;
    }

protected:
    osg::ref_ptr<osg::Node> m_pNode;
};

class AddChildToParent_Operation : public SceneGraphOperationBase
{
public:
    explicit AddChildToParent_Operation(void) {}

    virtual ~AddChildToParent_Operation(void) {}

    void setAddPair(osg::Group *pParent, osg::Node *pChild)
    {
        m_vecAddList.push_back(std::make_pair(pParent, pChild));
    }

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        if(m_vecAddList.empty())
        {
            return true;
        }

        for(std::vector<std::pair<osg::Group *, osg::Node *> >::iterator itor = m_vecAddList.begin(); itor != m_vecAddList.end(); ++itor)
        {
            if(itor->first != NULL && itor->second != NULL)
            {
                itor->first->addChild(itor->second);
            }
        }

        return true;
    }

protected:
    std::vector<std::pair<osg::Group *, osg::Node *> > m_vecAddList;
};

class RemoveTempModelByID_Operation : public SceneGraphOperationBase
{
public:
    explicit RemoveTempModelByID_Operation(const ID &id) : m_id(id){}

    virtual ~RemoveTempModelByID_Operation(void){}

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        osg::Group *pGroup = getTempElementRootNode(pOperator)->asGroup();

        if(pGroup == NULL)
        {
            return false;
        }

        for(unsigned int i = 0u; i < pGroup->getNumChildren(); i++)
        {
            if(pGroup->getChild(i)->getID() == m_id)
            {
                pGroup->removeChild(i);
                return true;
            }
        }

        return false;
    }

protected:
    const ID m_id;
};

class RemoveTempModelByNode_Operation : public SceneGraphOperationBase
{
public:
    explicit RemoveTempModelByNode_Operation(osg::Node *pNode) : m_pNode(pNode){}

    virtual ~RemoveTempModelByNode_Operation(void){}

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        osg::Group *pGroup = getTempElementRootNode(pOperator)->asGroup();

        if(m_pNode == NULL || pGroup == NULL)
        {
            return false;
        }

        pGroup->removeChild(m_pNode);

        return true;
    }

protected:
    osg::Node *m_pNode;
};

class RemoveChildFromParent_Operation : public SceneGraphOperationBase
{
public:
    explicit RemoveChildFromParent_Operation(void) {}

    virtual ~RemoveChildFromParent_Operation(void) {}

public:
    void addRemovePair(osg::Group *pParent, osg::Node *pChild)
    {
        m_vecRemoveList.push_back(std::make_pair(pParent, pChild));
    }

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        if(m_vecRemoveList.empty())
        {
            return true;
        }

        for(std::vector<std::pair<osg::Group *, osg::Node *> >::iterator itor = m_vecRemoveList.begin(); itor != m_vecRemoveList.end(); ++itor)
        {
            if(itor->first != NULL && itor->second != NULL)
            {
                unsigned int nPos = itor->first->getChildIndex(itor->second);
                if(nPos == itor->first->getNumChildren())
                {
                    continue;
                }
                itor->first->removeChildren(nPos, 1);
            }
        }

        return true;
    }

protected:
    std::vector<std::pair<osg::Group *, osg::Node *> > m_vecRemoveList;
};

class RemoveChildrenFormParent_Operation : public SceneGraphOperationBase
{
public:
    explicit RemoveChildrenFormParent_Operation(void) {}

    virtual ~RemoveChildrenFormParent_Operation(void) {}

public:
    void setRemoveObj(osg::Group *pParent, unsigned int nPos, unsigned int nNum)
    {
        RemoveObj obj;
        obj.m_pGroup = pParent;
        obj.m_nPos = nPos;
        obj.m_nNum = nNum;
        m_vecRemoveList.push_back(obj);
    }

protected:
    virtual bool doAction(SceneGraphOperator *pOperator)
    {
        if(m_vecRemoveList.empty())
        {
            return true;
        }

        for(std::vector<RemoveObj>::iterator itor = m_vecRemoveList.begin(); itor != m_vecRemoveList.end(); ++itor)
        {
            if(itor->m_pGroup != NULL)
            {
                itor->m_pGroup->removeChildren(itor->m_nPos, itor->m_nNum);
            }
        }

        return true;
    }

protected:
    struct RemoveObj
    {
        osg::Group *m_pGroup;
        unsigned int m_nPos;
        unsigned int m_nNum;
    };
    std::vector<RemoveObj> m_vecRemoveList;
};
//
//class ReplaceChildren_Operation : public SceneGraphOperationBase
//{
//public:
//    explicit ReplaceChildren_Operation(void) {}
//
//    virtual ~ReplaceChildren_Operation(void) {}
//
//public:
//    void addReplacePair(osg::Node *pChild, osg::Node *pNewChild)
//    {
//        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_MtxReplace);
//        m_vecReplaceList.push_back(std::make_pair(pChild, pNewChild));
//    }
//
//protected:
//    virtual bool doAction(SceneGraphOperator *pOperator)
//    {
//        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_MtxReplace);
//        if(m_vecReplaceList.empty())
//        {
//            return true;
//        }
//
//        for(std::vector<std::pair<osg::ref_ptr<osg::Node>, osg::ref_ptr<osg::Node> > >::iterator itor = m_vecReplaceList.begin(); itor != m_vecReplaceList.end(); ++itor)
//        {
//            if(itor->first != NULL && itor->second != NULL)
//            {
//                const unsigned nParentCount = itor->first->getNumParents();
//                for(unsigned i = 0u; i < nParentCount; i++)
//                {
//                    osg::Group *pParent = itor->first->getParent(i);
//                    pParent->replaceChild(itor->first, itor->second);
//                }
//            }
//        }
//
//        m_vecReplaceList.clear();
//
//        return true;
//    }
//
//protected:
//    OpenThreads::Mutex m_MtxReplace;
//    std::vector<std::pair<osg::ref_ptr<osg::Node>, osg::ref_ptr<osg::Node> > > m_vecReplaceList;
//};


#endif
