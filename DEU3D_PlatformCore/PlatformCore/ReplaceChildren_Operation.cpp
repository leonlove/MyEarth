#include "ReplaceChildren_Operation.h"
#include <osg/Group>


void ReplaceChildren_Operation::addReplacePair(osg::Node *pChild, osg::Node *pNewChild)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxReplace);
    m_vecReplaceList.push_back(std::make_pair(pChild, pNewChild));
}


bool ReplaceChildren_Operation::doAction(SceneGraphOperator *pOperator)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mtxReplace);
    if(m_vecReplaceList.empty())
    {
        return true;
    }

    for(std::vector<std::pair<osg::ref_ptr<osg::Node>, osg::ref_ptr<osg::Node> > >::iterator itor = m_vecReplaceList.begin(); itor != m_vecReplaceList.end(); ++itor)
    {
        if(itor->first != NULL && itor->second != NULL)
        {
            const unsigned nParentCount = itor->first->getNumParents();
            for(unsigned i = 0u; i < nParentCount; i++)
            {
                osg::Group *pParent = itor->first->getParent(i);
                pParent->replaceChild(itor->first, itor->second);
            }
        }
    }

    m_vecReplaceList.clear();

    return true;
}
