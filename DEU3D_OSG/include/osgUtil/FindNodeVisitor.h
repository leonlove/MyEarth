#ifndef FIND_NODE_VISITOR_H_282DFEAF_FD93_46F6_82D4_1405CE1D0D83_INCLUDE
#define FIND_NODE_VISITOR_H_282DFEAF_FD93_46F6_82D4_1405CE1D0D83_INCLUDE

#include <osgUtil/Export>

#include <osg/NodeVisitor>

namespace osgUtil
{

class OSGUTIL_EXPORT FindNodeByIDVisitor : public osg::NodeVisitor
{
public:
    explicit FindNodeByIDVisitor(const ID &id);
    virtual ~FindNodeByIDVisitor(void);

public:
    inline osg::Node *getTargetNode()    {    return m_pTargetNode.get();    }

protected:
    virtual void      apply(osg::Node &node);

protected:
    ID  m_id;
    osg::ref_ptr<osg::Node>  m_pTargetNode;
};


class OSGUTIL_EXPORT FindNodesByIDListVisitor : public osg::NodeVisitor
{
public:
    explicit FindNodesByIDListVisitor(const std::set<ID> &listIDs);
    virtual ~FindNodesByIDListVisitor(void);

public:
    typedef std::vector<osg::ref_ptr<osg::Node> >   NodeList;
    inline const NodeList &getTargetNodes(void) const    {    return m_listTargetNodes;    }

protected:
    virtual void      apply(osg::Node &node);

protected:
    std::set<ID>        m_listIDs;
    NodeList            m_listTargetNodes;
};


class OSGUTIL_EXPORT FindNodesByIDTypeVisitor : public osg::NodeVisitor
{
public:
    explicit FindNodesByIDTypeVisitor(unsigned nType);
    virtual ~FindNodesByIDTypeVisitor(void);

public:
    typedef std::vector<osg::ref_ptr<osg::Node> >   NodeList;
    inline const NodeList &getTargetNodes(void) const    {    return m_listTargetNodes;    }

protected:
    virtual void      apply(osg::Node &node);

protected:
    const unsigned      m_nIDType;
    NodeList            m_listTargetNodes;
};


class OSGUTIL_EXPORT FindNodeByNameVisitor : public osg::NodeVisitor
{
public:
    explicit FindNodeByNameVisitor(const std::string &strName);
    virtual ~FindNodeByNameVisitor(void);

public:
    inline osg::Node *getTargetNode()    {    return m_pTargetNode;    }

protected:
    virtual void      apply(osg::Node &node);

protected:
    std::string m_strName;
    osg::Node  *m_pTargetNode;
};


}

#endif
