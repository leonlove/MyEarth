#include "VirtualCubeNode.h"

#include <osg/Math>
#include <algorithm>

#include <osg/Material>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Depth>
#include <osgDB/DatabasePager>
#include <osg/CoordinateSystemNode>
#include <osg/LOD>
#include <iostream>

#include "SmartLOD.h"
#include "LODFixer.h"
#include <IDProvider/Definer.h>

#include "Registry.h"

OpenSP::op<cmm::IStateQuerier>      VCubeFragmentNode::ms_pStateQuerier = NULL;

unsigned VCubeFragmentNode::ms_nLoadingCountPerFrame = 0u;

VCubeFragmentNode::VCubeFragmentNode(void) :
    m_nNextRequestChild(0u)
{
    m_vecChildrenInfo.reserve(64u);
}


VCubeFragmentNode::VCubeFragmentNode(const VCubeFragmentNode &proxynode, const osg::CopyOp &copyop)
{

}


VCubeFragmentNode::~VCubeFragmentNode(void)
{
    m_vecChildrenInfo.clear();
}


//添加一个子节点
bool VCubeFragmentNode::addChild(osg::Node *pChild)
{
    const ID &childID = pChild->getID();
    std::vector<TileChildInfo>::iterator itorFind = std::find_if(m_vecChildrenInfo.begin(), m_vecChildrenInfo.end(), ChildFinder(childID));
    if(itorFind == m_vecChildrenInfo.end())
    {
        return false;
    }

    TileChildInfo &info = *itorFind;
    if(info.m_eVisible != cmm::SV_Enable)
    {
        // 隐藏状态下不添加
        return false;
    }

    if(!Group::addChild(pChild))
    {
        return false;
    }

    info.m_pChild       = pChild;
    info.m_pFileRequest = NULL;

    return true;
}


bool VCubeFragmentNode::setVisible(const ID &id, bool bVisible)
{
    std::vector<TileChildInfo>::iterator itorFind = std::find_if(m_vecChildrenInfo.begin(), m_vecChildrenInfo.end(), ChildFinder(id));
    if(itorFind == m_vecChildrenInfo.end())
    {
        return false;
    }

    TileChildInfo &info = *itorFind;
    if(bVisible)
    {
        info.m_eVisible = cmm::SV_Enable;
    }
    else
    {
        info.m_eVisible = cmm::SV_Disable;
        osg::ref_ptr<osg::Node> pChild;
        if(info.m_pChild.lock(pChild))
        {
            pChild->getParent(0)->removeChild(pChild);
        }
    }
    return true;
}


void VCubeFragmentNode::setVisible(const std::set<ID> &listIDs, bool bVisible)
{
    std::vector<TileChildInfo>::iterator itorFind = m_vecChildrenInfo.begin();
    for( ; itorFind != m_vecChildrenInfo.end(); ++itorFind)
    {
        TileChildInfo &info = *itorFind;
        std::set<ID>::iterator itor = listIDs.find(info.m_id);
        if(itor == listIDs.end())
        {
            continue;
        }

        if(bVisible)
        {
            info.m_eVisible = cmm::SV_Enable;
        }
        else
        {
            info.m_eVisible = cmm::SV_Disable;
            osg::ref_ptr<osg::Node> pChild;
            if(info.m_pChild.lock(pChild))
            {
                pChild->getParent(0)->removeChild(pChild);
            }
        }
    }
}


bool VCubeFragmentNode::addChildObject(const ID &id)
{
    m_vecChildrenInfo.resize(m_vecChildrenInfo.size() + 1);
    TileChildInfo &info = m_vecChildrenInfo.back();
    info.m_id = id;
    info.m_eVisible = cmm::SV_Unknown;

    return true;
}


bool VCubeFragmentNode::removeChildObject(const ID &id)
{
    std::vector<TileChildInfo>::iterator itorFind = std::find_if(m_vecChildrenInfo.begin(), m_vecChildrenInfo.end(), ChildFinder(id));
    if(itorFind == m_vecChildrenInfo.end())
    {
        return false;
    }

    //若其对应的Node不为空，从ChildList中删除
    osg::ref_ptr<osg::Node> pNode;
    TileChildInfo &info = *itorFind;
    if(info.m_pChild.lock(pNode))
    {
        osg::Group::removeChild(pNode.get());
    }

    m_vecChildrenInfo.erase(itorFind);

    return true;
}


bool VCubeFragmentNode::getChildByID(const ID &id, osg::ref_ptr<osg::Node> &pChildNode) const
{
    std::vector<TileChildInfo>::const_iterator itorFind
        = std::find_if(m_vecChildrenInfo.begin(), m_vecChildrenInfo.end(), ChildFinder(id));
    if(itorFind != m_vecChildrenInfo.end())
    {
        const TileChildInfo &info = *itorFind;
        return info.m_pChild.lock(pChildNode);
    }
    return false;
}


void VCubeFragmentNode::getObjects(IDList &listIDs) const
{
    std::vector<TileChildInfo>::const_iterator itorChild = m_vecChildrenInfo.begin();
    for( ; itorChild != m_vecChildrenInfo.end(); ++itorChild)
    {
        const TileChildInfo &child = *itorChild;
        listIDs.push_back(child.m_id);
    }
}


void VCubeFragmentNode::traverse(osg::NodeVisitor& nv)
{
    if(m_vecChildrenInfo.empty())
    {
        return;
    }

    if(!nv.getDatabaseRequestHandler())
    {
        Group::traverse(nv);
        return;
    }

    unsigned nIndex = 0u;
    OpenSP::sp<cmm::IStateQuerier>  pStateQuerier;
    ms_pStateQuerier.lock(pStateQuerier);
    for(nIndex = m_nNextRequestChild; nIndex < m_vecChildrenInfo.size(); nIndex++)
    {
        TileChildInfo &childInfo = m_vecChildrenInfo[nIndex];
#if 1
        if(childInfo.m_eVisible == cmm::SV_Enable)
        {
            osg::ref_ptr<osg::Node> pNode;
            if(childInfo.m_pChild.lock(pNode))
            {
                continue;
            }
        }
        else if(childInfo.m_eVisible == cmm::SV_Unknown)
        {
            if(pStateQuerier.valid())
            {
                childInfo.m_eVisible = pStateQuerier->queryObjectState(childInfo.m_id, cmm::STATE_VISIBLE);
            }
            else
            {
                childInfo.m_eVisible = cmm::SV_Disable;
            }

            if(childInfo.m_eVisible != cmm::SV_Enable)
            {
                continue;
            }
        }
        else
        {
            osg::ref_ptr<osg::Node> pNode;
            if(childInfo.m_pChild.lock(pNode))
            {
                osg::Group::removeChild(pNode.get());
            }
            continue;
        }
#else
        osg::ref_ptr<osg::Node> pNode;
        if(childInfo.m_pChild.lock(pNode))
        {
            continue;
        }
        childInfo.m_eVisible = cmm::SV_Enable;
#endif
        nv.getDatabaseRequestHandler()->requestNodeFile(childInfo.m_id, 
            nv.getNodePath(),
            nv.getFrameStamp(),
            childInfo.m_pFileRequest,
            m_pDatabaseOptions.get());

        if(ms_nLoadingCountPerFrame != 0u)
        {
            if(nIndex + 1u >= ms_nLoadingCountPerFrame + m_nNextRequestChild)
            {
                // 一次最多发送出ms_nReadCountPerFrame个加载请求
                nIndex++;
                break;
            }
        }
    }

    m_nNextRequestChild = nIndex;
    if(m_nNextRequestChild >= m_vecChildrenInfo.size())
    {
        m_nNextRequestChild = 0u;
    }

    if (Registry::instance()->getUseShadow())
    {
        for (unsigned int i = 0u; i < _children.size(); i++)
        {
            osg::Node* pChildNode = _children[i];
            if (!pChildNode)
            {
                continue;
            }

            float distance = nv.getDistanceToViewPoint(pChildNode->getBound().center());
            float dEnv = 300.0;

            char *pDistance = ::getenv("DEU_SHADOW_DISTANCE");
            if (pDistance != NULL)
            {
                dEnv = atof(pDistance);
            }

            if (pChildNode->getFlag() == 1 && distance < dEnv)
            {
                pChildNode->setNodeMask(2);
            }
            else
            {
                pChildNode->setNodeMask(1);
            }

            pChildNode->accept(nv);
        }
    }
    else
    {
        Group::traverse(nv);
    }

    return;
}


VCubeFragmentGroup::VCubeFragmentGroup(void) : m_dblRadius(-1.0), m_nFragmentCount(0u)
{
}


VCubeFragmentGroup::~VCubeFragmentGroup(void)
{
}


bool VCubeFragmentGroup::create(const vcm::IVirtualCubeManager *pVCManager, const ID &id, const ObjectCube &cubeObjectIDs, LODFixer *pLODFixer)
{
    const cmm::Pyramid3 *pPyramid = cmm::Pyramid3::instance();

    cmm::math::Point3d ptMinCoord, ptMaxCoord;
    pPyramid->getCubePos(id.CubeID.m_nLevel, id.CubeID.m_nRow, id.CubeID.m_nCol, id.CubeID.m_nHeight, ptMinCoord, ptMaxCoord);
    cmm::math::Point3d ptCenterCoord((ptMinCoord + ptMaxCoord) * 0.5);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    {
        double dblSeaLevel;

        dblSeaLevel = pEllipsoidModel->computeLocalSeaLevel(ptMinCoord.y(), ptMinCoord.x());
        ptMinCoord.z() -= dblSeaLevel;

        dblSeaLevel = pEllipsoidModel->computeLocalSeaLevel(ptMaxCoord.y(), ptMaxCoord.x());
        ptMaxCoord.z() -= dblSeaLevel;

        dblSeaLevel = pEllipsoidModel->computeLocalSeaLevel(ptCenterCoord.y(), ptCenterCoord.x());
        ptCenterCoord.z() -= dblSeaLevel;
    }

    osg::Vec3d ptMinPos, ptMaxPos, ptCenterPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), ptMinCoord.z(), ptMinPos.x(), ptMinPos.y(), ptMinPos.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), ptMinCoord.z(), ptMaxPos.x(), ptMaxPos.y(), ptMaxPos.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), ptCenterCoord.z(), ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

    const double dblRadius1 = (ptMinPos - ptMaxPos).length() * 0.5;
    const double dblRadius2 = (ptCenterPos - ptMinPos).length();
    const double dblRadius3 = (ptCenterPos - ptMaxPos).length();
    const double dblRadius  = std::max(dblRadius1, std::max(dblRadius2, dblRadius3));

    setCenter(ptCenterPos);
    setRadius(dblRadius);

    m_nFragmentCount = pVCManager->getCubeFragmentCount();
    const double   dblFragCount      = m_nFragmentCount;
    const double   dblFragCount2     = dblFragCount * dblFragCount;
    const double   dblFragCount3     = dblFragCount * dblFragCount2;
    const double   dblCubeRangeRatio = pVCManager->getCubeRangeRatio();
    const double   dblCubeRange      = dblRadius * dblCubeRangeRatio;

    const osg::Vec3d vecFragInterval((ptMaxCoord.x() - ptMinCoord.x()) / dblFragCount, (ptMaxCoord.y() - ptMinCoord.y()) / dblFragCount, (ptMaxCoord.z() - ptMinCoord.z()) / dblFragCount);

    ID idCubeFragmentNode = id;
    idCubeFragmentNode.CubeID.m_nType = V_CUBE_FRAGMENT_NODE_ID;

    for(unsigned y = 0u; y < m_nFragmentCount; y++)
    {
        const ObjectMatrix &objectMatrix = cubeObjectIDs[y];
        for(unsigned x = 0u; x < m_nFragmentCount; x++)
        {
            const ObjectRow &objectRow = objectMatrix[x];
            for(unsigned z = 0u; z < m_nFragmentCount; z++)
            {
                const IDList &objSet = objectRow[z];

                VCubeFragmentNode *pFragmentNode = new VCubeFragmentNode;
                pFragmentNode->setID(idCubeFragmentNode);
                for(IDList::const_iterator itorObj = objSet.begin(); itorObj != objSet.end(); ++itorObj)
                {
                    const ID &idObj = *itorObj;
                    pFragmentNode->addChildObject(idObj);
                }

                const double dblX = x + 0.5;
                const double dblY = y + 0.5;
                const double dblZ = z + 0.5;
                const osg::Vec3d ptFragCenterCoord(ptMinCoord.x() + dblX * vecFragInterval.x(), ptMinCoord.y() + dblY * vecFragInterval.y(), ptMinCoord.z() + dblZ * vecFragInterval.z());

                osg::Vec3d ptFragCenter;
                pEllipsoidModel->convertLatLongHeightToXYZ(ptFragCenterCoord.y(), ptFragCenterCoord.x(), ptFragCenterCoord.z(), ptFragCenter.x(), ptFragCenter.y(), ptFragCenter.z());

#if 0
                osg::LOD *pLOD = new osg::LOD;
                pLOD->setCenter(ptFragCenter);
                pLOD->setRadius(dblRadius / dblFragCount);
                pLOD->addChild(pFragmentNode, 0.0, dblCubeRange);
#else
                SmartLOD *pLOD = new SmartLOD;
                pLOD->setID(idCubeFragmentNode);
                pLOD->setCenter(ptFragCenter);
                pLOD->setRadius(dblRadius / dblFragCount);
                pLOD->addChild(pFragmentNode, 0.0, dblCubeRange);
                if(pLODFixer)
                {
                    pLOD->addEventCallback(pLODFixer);
                }
#endif

                if(objSet.empty())
                {
                    pLOD->setNodeMask(0u);
                }
                else
                {
                    pLOD->setNodeMask(~0u);
                }

                addChild(pLOD);
            }
        }
    }

    ID idThis = id;
    idThis.ObjectID.m_nType = V_CUBE_FRAGMENT_GROUP_ID;
    setID(idThis);

    return true;
}


VCubeFragmentNode *VCubeFragmentGroup::getFragmentNode(unsigned nFragmentCount, unsigned nRow, unsigned nCol, unsigned nHeight)
{
    const unsigned nChildIndex = nRow * nFragmentCount * nFragmentCount + nCol * nFragmentCount + nHeight;
    osg::Group *pProxyParent = getChild(nChildIndex)->asGroup();
    VCubeFragmentNode *pProxyNode = dynamic_cast<VCubeFragmentNode *>(pProxyParent->getChild(0u));
    return pProxyNode;
}


osg::BoundingSphere VCubeFragmentGroup::computeBound(void) const
{
    if(m_dblRadius >= 0.0f)
    {
        return osg::BoundingSphere(m_ptCenter, m_dblRadius);
    }
    else
    {
        return Group::computeBound();
    }
}


bool VCubeFragmentGroup::getChildByID(const ID &id, osg::ref_ptr<osg::Node> &pChildNode) const
{
    const unsigned nCount = m_nFragmentCount * m_nFragmentCount;
    for(unsigned i = 0u; i < nCount; i++)
    {
        const SmartLOD *pLOD = dynamic_cast<const SmartLOD *>(getChild(i));
        if(!pLOD)       continue;

        const VCubeFragmentNode *pProxyNode = dynamic_cast<const VCubeFragmentNode *>(pLOD->getChild(0u));
        if(!pProxyNode) continue;

        if(pProxyNode->getChildByID(id, pChildNode))
        {
            return true;
        }
    }

    return false;
}


void VCubeFragmentGroup::getObjects(IDList &listIDs) const
{
    listIDs.clear();
    const unsigned nCount = m_nFragmentCount * m_nFragmentCount;
    for(unsigned i = 0u; i < nCount; i++)
    {
        const SmartLOD *pLOD = dynamic_cast<const SmartLOD *>(getChild(i));
        if(!pLOD)       continue;

        const VCubeFragmentNode *pProxyNode = dynamic_cast<const VCubeFragmentNode *>(pLOD->getChild(0u));
        if(!pProxyNode) continue;

        pProxyNode->getObjects(listIDs);
    }
}


bool VCubeFragmentGroup::setVisible(const ID &id, bool bVisible)
{
    const unsigned nCount = m_nFragmentCount * m_nFragmentCount;
    for(unsigned i = 0u; i < nCount; i++)
    {
        SmartLOD *pLOD = dynamic_cast<SmartLOD *>(getChild(i));
        if(!pLOD)       continue;

        VCubeFragmentNode *pProxyNode = dynamic_cast<VCubeFragmentNode *>(pLOD->getChild(0u));
        if(!pProxyNode) continue;

        if(pProxyNode->setVisible(id, bVisible))
        {
            return true;
        }
    }
    return false;
}

