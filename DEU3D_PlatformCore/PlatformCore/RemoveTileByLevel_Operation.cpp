#include "RemoveTileByLevel_Operation.h"
#include <osg/Group>
#include <IDProvider/Definer.h>

class FineTileByLevelVisitor: public osg::NodeVisitor
{
public:
    FineTileByLevelVisitor(unsigned int nLevel):
      osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
          m_nLevel(nLevel)
      {
      }

      void apply(osg::Group &node)
      {
          const ID &id = node.getID();
          if(!id.isValid())
          {
              traverse(node);
              return;
          }

          if (id.TileID.m_nType == TERRAIN_TILE && id.TileID.m_nLevel == m_nLevel)
          {
              //node.getParent(0)->removeChild(&node);
              m_vecTagertNode.push_back(&node);
              return;
          }

          traverse(node);
      }

      const unsigned int m_nLevel;
      std::vector<osg::Node *> m_vecTagertNode;
};


bool RemoveTileByLevel_Operation::doAction(SceneGraphOperator *pOperator)
{
    FineTileByLevelVisitor ftlv(m_nLevel);
    getTerrainRootNode(pOperator)->accept(ftlv);

    unsigned int nSize = ftlv.m_vecTagertNode.size();
    for(unsigned int i = 0; i < nSize; i++)
    {
        osg::Group *pGroup = ftlv.m_vecTagertNode[i]->getParent(0);
        if(pGroup->getNumChildren() == 2)
        {
            pGroup->osg::Group::removeChildren(1u, 1u);
        }
    }

    return true;
}
