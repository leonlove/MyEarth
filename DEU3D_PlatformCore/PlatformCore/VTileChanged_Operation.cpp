#include "VTileChanged_Operation.h"
#include "SceneGraphOperator.h"
#include "DEUProxyNode.h"
#include <osg/PagedLOD>
#include <osgUtil/FindNodeVisitor.h>
#include <VirtualTileManager/IVirtualTileManager.h>
#include "FileReadInterceptor.h"
#include <IDProvider/Definer.h>

void VTileChanged_Operation::addTile(const ID &idVTile, const vtm::IVirtualTile *pVTile)
{
    m_mapChangedTiles[idVTile] = pVTile;
}


bool VTileChanged_Operation::doAction(SceneGraphOperator *pOperator)
{
    if(m_mapChangedTiles.empty())   return true;

    osg::Node *pCultureRootNode = getCultureRootNode(pOperator);

    std::set<ID>    setChangedIDs;
    for(std::map<ID, OpenSP::sp<const vtm::IVirtualTile> >::const_iterator itorTile = m_mapChangedTiles.begin();
        itorTile != m_mapChangedTiles.end(); ++itorTile)
    {
        setChangedIDs.insert(itorTile->first);
    }

    osg::ref_ptr<osgUtil::FindNodesByIDListVisitor>  pFinder = new osgUtil::FindNodesByIDListVisitor(setChangedIDs);
    pCultureRootNode->accept(*pFinder);

    const osgUtil::FindNodesByIDListVisitor::NodeList &listVTileNodes = pFinder->getTargetNodes();
    if(listVTileNodes.empty())
    {
        return true;
    }

    for(osgUtil::FindNodesByIDListVisitor::NodeList::const_iterator itorNode = listVTileNodes.begin();
        itorNode != listVTileNodes.end(); ++itorNode)
    {
        osg::Node *pNode = *itorNode;
        if(!pNode)  continue;

        const ID &id = pNode->getID();

        std::map<ID, OpenSP::sp<const vtm::IVirtualTile> >::iterator itor = m_mapChangedTiles.find(id);
        if(itor == m_mapChangedTiles.end())
        {
            continue;
        }

        const vtm::IVirtualTile *pVTile = itor->second.get();
        if(!pVTile)
        {
            const ID &idNode = pNode->getID();
            if(idNode.TileID.m_nLevel > 0u)
            {
                osg::ref_ptr<osg::Group> pParent = pNode->getParent(0u);
                const unsigned nPos = pParent->getChildIndex(pNode);
                if(nPos < pParent->getNumChildren())
                {
                    pParent->osg::Group::removeChildren(nPos, 1u);
                }
                pParent->setNodeMask(0x00000000);
                continue;
            }
        }

        doMerge(pNode, pVTile);
    }

    return true;
}


void VTileChanged_Operation::doMerge(osg::Node *pVTileNode, const vtm::IVirtualTile *pVTile)
{
    osg::Group *pVTileGroup = pVTileNode->asGroup();
    const ID &idVTile = pVTileNode->getID();
    const unsigned nPagedLODCount = ((idVTile.TileID.m_nLevel == 0u) ? 2u : 4u);

    IDList objListInFile;
    DEUProxyGroup *pProxyGroup = dynamic_cast<DEUProxyGroup *>(pVTileGroup->getChild(nPagedLODCount));
    for(unsigned y = 0u; y < m_nFragmentCount; y++)
    {
        for(unsigned x = 0u; x < m_nFragmentCount; x++)
        {
            objListInFile.clear();
            if(pVTile)
            {
                pVTile->getFragmentObjects(y, x, objListInFile);
            }

            DEUProxyNode *pProxyNode = pProxyGroup->getProxyNode(m_nFragmentCount, y, x);
            IDList objListInNode;
            pProxyNode->getInstances(objListInNode);

            std::set<ID>    setObjListInFile(objListInFile.begin(), objListInFile.end());
            std::set<ID>    setObjListInNode(objListInNode.begin(), objListInNode.end());
            if(setObjListInFile == setObjListInNode)
            {
                continue;
            }

            //先删除多余的
            std::set<ID>::iterator itor;
            for(itor = setObjListInNode.begin(); itor != setObjListInNode.end(); )
            {
                if(setObjListInFile.end() == setObjListInFile.find(*itor))
                {
                    pProxyNode->removeChildObject(*itor);
                    itor = setObjListInNode.erase(itor);
                    continue;
                }
                ++itor;
            }

            //添加新增的
            for(itor = setObjListInFile.begin(); itor != setObjListInFile.end(); ++itor)
            {
                if(setObjListInNode.end() == setObjListInNode.find(*itor))
                {
                    pProxyNode->addChildObject(*itor);
                }
            }

            osg::Group *pParent = pProxyNode->getParent(0u);
            if(pProxyNode->getChildObjectCount() > 0u)
            {
                pParent->setNodeMask(~0u);
            }
            else
            {
                pParent->setNodeMask(0u);
            }
        }
    }

    bool bChildValid[4] = {false, false, false, false};
    if(pVTile)
    {
        pVTile->getChildTileState(bChildValid);
    }
    for(unsigned int i = 0u; i < nPagedLODCount; i++)
    {
        osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pVTileGroup->getChild(i));
        pPagedLOD->setNodeMask(bChildValid[i] ? 0xFFFFFFFF : 0x00000000);
    }
}

