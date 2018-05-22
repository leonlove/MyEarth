#ifndef TERRAIN_MODIFICATION_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE
#define TERRAIN_MODIFICATION_OPERATION_H_500E181C_F9D3_4A64_91E4_F44DF4724629_INCLUDE

#include "SceneGraphOperationBase.h"
#include "TerrainModificationManager.h"
#include <IDProvider/Definer.h>

class TerrainModification_Operation : public SceneGraphOperationBase
{
public:
    explicit TerrainModification_Operation(osg::Node *pTerrainNode, TerrainModificationManager *pTerrainModificationManager, TerrainModification *pModification, bool bApply) :
    m_pTerrainModificationManager(pTerrainModificationManager),
        m_pTerrainNode(pTerrainNode),
        m_bApply(bApply)
    {
        m_pTerrainModification = pModification;
    }

    virtual ~TerrainModification_Operation(void) {}

protected:
    class TerrainTileFinder : public osg::NodeVisitor
    {
    public:
        TerrainTileFinder(TerrainModification_Operation *pOperation) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            m_pOperation(pOperation)
        {
        }

        void apply(osg::Group &node)
        {
            osgTerrain::TerrainTile *pTile = dynamic_cast<osgTerrain::TerrainTile *>(&node);
            if(!pTile)
            {
                osg::NodeVisitor::apply(node);
                return;
            }

            const std::string &strType = m_pOperation->m_pTerrainModification->getType();
            const bool bModifyTexture = (strType.compare(TMT_DOM_MODIFICATION) == 0 || strType.compare(TMT_COLOR_MODIFICATION) == 0);

            bool bModified = false;
            if(m_pOperation->m_bApply)
            {
                bModified = m_pOperation->m_pTerrainModification->modifyTerrainTile(pTile);
            }
            else
            {
                if(m_pOperation->m_pTerrainModification->shouldBeModified(pTile))
                {
                    bModified = m_pOperation->m_pTerrainModificationManager->modifyTerrainTile(pTile, bModifyTexture);
                }
            }

            if(bModified)
            {
                const osgTerrain::TerrainTile::DirtyMask  eDirtyMask =
                    (bModifyTexture ? osgTerrain::TerrainTile::IMAGERY_DIRTY : osgTerrain::TerrainTile::ELEVATION_DIRTY);
                pTile->init(eDirtyMask, false);
            }
        }
    protected:
        TerrainModification_Operation  *m_pOperation;
    };

    virtual bool doAction(SceneGraphOperator *pOperator)
    {
#if _DEBUG
        printf("doAction Begin\n");
#endif

        //printf("TerrainModificationManager size is %d\n", m_pTerrainModificationManager->getModificationCount());
        if(!m_pTerrainNode.valid() || !m_pTerrainModificationManager.valid() || !m_pTerrainModification.valid())
        {
            return false;
        }

        TerrainTileFinder ttf(this);

        m_pTerrainNode->accept(ttf);

#if _DEBUG
        printf("doAction End\n");
#endif
        return true;
    }

protected:
    osg::ref_ptr<osg::Node>                 m_pTerrainNode;
    OpenSP::sp<TerrainModificationManager>  m_pTerrainModificationManager;
    OpenSP::sp<TerrainModification>         m_pTerrainModification;
    bool                                    m_bApply;
};

#endif