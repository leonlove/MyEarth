#include "VCubeChanged_Operation.h"
#include "SceneGraphOperator.h"
#include "DEUProxyNode.h"
#include <osg/PagedLOD>
#include <osgUtil/FindNodeVisitor.h>
#include <VirtualTileManager/IVirtualTileManager.h>
#include "FileReadInterceptor.h"
#include <IDProvider/Definer.h>

void VCubeChanged_Operation::addCube(const ID &idVCube, const vcm::IVirtualCube *pVCube)
{
    m_mapChangedCubes[idVCube] = pVCube;
}


bool VCubeChanged_Operation::doAction(SceneGraphOperator *pOperator)
{
    if(m_mapChangedCubes.empty())   return true;

    osg::Node *pCultureRootNode = getCultureRootNode(pOperator);

    std::set<ID>    setChangedIDs;
    for(std::map<ID, OpenSP::sp<const vcm::IVirtualCube> >::const_iterator itorCube = m_mapChangedCubes.begin();
        itorCube != m_mapChangedCubes.end(); ++itorCube)
    {
        setChangedIDs.insert(itorCube->first);
    }

    osg::ref_ptr<osgUtil::FindNodesByIDListVisitor>  pFinder = new osgUtil::FindNodesByIDListVisitor(setChangedIDs);
    pCultureRootNode->accept(*pFinder);

    const osgUtil::FindNodesByIDListVisitor::NodeList &listVCubeNodes = pFinder->getTargetNodes();
    if(listVCubeNodes.empty())
    {
        return true;
    }

    for(osgUtil::FindNodesByIDListVisitor::NodeList::const_iterator itorNode = listVCubeNodes.begin();
        itorNode != listVCubeNodes.end(); ++itorNode)
    {
        osg::Node *pNode = *itorNode;
        if(!pNode)  continue;

        const ID &id = pNode->getID();

        std::map<ID, OpenSP::sp<const vcm::IVirtualCube> >::iterator itor = m_mapChangedCubes.find(id);
        if(itor == m_mapChangedCubes.end())
        {
            continue;
        }

        const vcm::IVirtualCube *pVCube = itor->second.get();
        if(!pVCube)
        {
            const ID &idNode = pNode->getID();
            if(idNode.CubeID.m_nLevel > 0u)
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

        doMerge(pNode, pVCube);
    }

    return true;
}


void VCubeChanged_Operation::doMerge(osg::Node *pVCubeNode, const vcm::IVirtualCube *pVCube)
{
    osg::Group *pVCubeGroup = pVCubeNode->asGroup();
    if(!pVCubeGroup)    return;

    const ID &idVCube = pVCubeNode->getID();
    const unsigned nPagedLODCount = ((idVCube.CubeID.m_nLevel == 0u) ? 4u : 8u);

    IDList objListInFile;
    VCubeFragmentGroup *pCubeFragmentGroup = dynamic_cast<VCubeFragmentGroup *>(pVCubeGroup->getChild(nPagedLODCount));
    for(unsigned y = 0u; y < m_nFragmentCount; y++)
    {
        for(unsigned x = 0u; x < m_nFragmentCount; x++)
        {
            for(unsigned z = 0u; z < m_nFragmentCount; z++)
            {
                objListInFile.clear();
                if(pVCube)
                {
                    pVCube->getFragmentObjects(y, x, z, objListInFile);
                }

                VCubeFragmentNode *pCubeFragmentNode = pCubeFragmentGroup->getFragmentNode(m_nFragmentCount, y, x, z);
                IDList objListInNode;
                pCubeFragmentNode->getInstances(objListInNode);

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
                        pCubeFragmentNode->removeChildObject(*itor);
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
                        pCubeFragmentNode->addChildObject(*itor);
                    }
                }

                osg::Group *pParent = pCubeFragmentNode->getParent(0u);
                if(pCubeFragmentNode->getChildObjectCount() > 0u)
                {
                    pParent->setNodeMask(~0u);
                }
                else
                {
                    pParent->setNodeMask(0u);
                }
            }
        }
    }

    bool bChildValid[8] = {false, false, false, false, false, false, false, false};
    if(pVCube)
    {
        pVCube->getChildCubeState(bChildValid);
    }
    for(unsigned int i = 0u; i < nPagedLODCount; i++)
    {
        osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pVCubeGroup->getChild(i));
        pPagedLOD->setNodeMask(bChildValid[i] ? 0xFFFFFFFF : 0x00000000);
    }
}

