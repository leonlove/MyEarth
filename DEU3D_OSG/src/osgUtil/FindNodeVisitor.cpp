#include <osgUtil/FindNodeVisitor.h>
#include <IDProvider/Definer.h>

namespace osgUtil
{

FindNodeByIDVisitor::FindNodeByIDVisitor(const ID &id) :
    osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
    m_pTargetNode(NULL),
    m_id(id)
{
}


FindNodeByIDVisitor::~FindNodeByIDVisitor(void)
{
}


void FindNodeByIDVisitor::apply(osg::Node &node)
{
    const ID &id = node.getID();

    if(id == m_id)
    {
        m_pTargetNode = &node;
        return;
    }

    if(id.ObjectID.m_nType == PARAM_POINT_ID
        || id.ObjectID.m_nType == PARAM_LINE_ID
        || id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        // 优化一点性能，已经遍历到了单个物体了，绝对不需要再继续往下traverse了
        return;
    }

    traverse(node);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

FindNodesByIDListVisitor::FindNodesByIDListVisitor(const std::set<ID> &listIDs) :
    osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    m_listIDs = listIDs;
}


FindNodesByIDListVisitor::~FindNodesByIDListVisitor(void)
{
}


void FindNodesByIDListVisitor::apply(osg::Node &node)
{
    const ID &id = node.getID();
    std::set<ID>::const_iterator itorFind = m_listIDs.find(id);
    if(itorFind != m_listIDs.end())
    {
        m_listTargetNodes.push_back(&node);
        m_listIDs.erase(itorFind);
    }

    if(id.ObjectID.m_nType == PARAM_POINT_ID
        || id.ObjectID.m_nType == PARAM_LINE_ID
        || id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        // 优化一点性能，已经遍历到了单个物体了，绝对不需要再继续往下traverse了
        return;
    }

    traverse(node);
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

FindNodesByIDTypeVisitor::FindNodesByIDTypeVisitor(unsigned nType) :
    osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
    m_nIDType(nType)
{
}


FindNodesByIDTypeVisitor::~FindNodesByIDTypeVisitor(void)
{
}


void FindNodesByIDTypeVisitor::apply(osg::Node &node)
{
    const ID &id = node.getID();
    if(id.ObjectID.m_nType == m_nIDType)
    {
        m_listTargetNodes.push_back(&node);
    }
    else if(id.ObjectID.m_nType == PARAM_POINT_ID
        || id.ObjectID.m_nType == PARAM_LINE_ID
        || id.ObjectID.m_nType == PARAM_FACE_ID)
    {
        // 优化一点性能，已经遍历到了单个物体了，绝对不需要再继续往下traverse了
        return;
    }
    traverse(node);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
FindNodeByNameVisitor::FindNodeByNameVisitor(const std::string &strName) :
    osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
    m_pTargetNode(NULL),
    m_strName(strName)
{
}


FindNodeByNameVisitor::~FindNodeByNameVisitor(void)
{
}


void FindNodeByNameVisitor::apply(osg::Node &node)
{
    if(node.getName() == m_strName)
    {
        m_pTargetNode = &node;
        return;
    }
    traverse(node);
}

}