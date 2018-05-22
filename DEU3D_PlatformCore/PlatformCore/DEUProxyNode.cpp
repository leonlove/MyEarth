#include "DEUProxyNode.h"

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

OpenSP::op<cmm::IStateQuerier>      DEUProxyNode::ms_pStateQuerier = NULL;

unsigned DEUProxyNode::ms_nLoadingCountPerFrame = 0u;

DEUProxyNode::DEUProxyNode(void) :
    m_nNextRequestChild(0u)
{
    m_vecChildrenInfo.reserve(64u);
}


DEUProxyNode::DEUProxyNode(const DEUProxyNode &proxynode, const osg::CopyOp &copyop)
{

}


DEUProxyNode::~DEUProxyNode(void)
{
    m_vecChildrenInfo.clear();
}


//添加一个子节点
bool DEUProxyNode::addChild(osg::Node *pChild)
{
    // 警告：
    // 本函数的std::find_if性能不会太高，而本函数一定是在update traverse调用的
    // 因此std::find_if的性能会制约update traverse的性能
    // 但是现阶段它似乎不明显，等待后期做性能比较的时候，再来考虑m_vecChildrenInfo是否要改成一个std::map对象

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

    osg::LOD *pLOD = dynamic_cast<osg::LOD *>(getParent(0));
    osg::BoundingSphere bs = pChild->getBound();
    osg::BoundingSphere bs1(pLOD->getCenter(), pLOD->getRadius());
    bs1.expandBy(bs);
    pLOD->setCenter(bs1.center());
    pLOD->setRadius(bs1.radius());
    return true;
}


bool DEUProxyNode::setVisible(const ID &id, bool bVisible)
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


void DEUProxyNode::setVisible(const std::set<ID> &listIDs, bool bVisible)
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


bool DEUProxyNode::addChildObject(const ID &id)
{
    m_vecChildrenInfo.resize(m_vecChildrenInfo.size() + 1);
    TileChildInfo &info = m_vecChildrenInfo.back();
    info.m_id = id;
    info.m_eVisible = cmm::SV_Unknown;

    return true;
}


bool DEUProxyNode::removeChildObject(const ID &id)
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


bool DEUProxyNode::getChildByID(const ID &id, osg::ref_ptr<osg::Node> &pChildNode) const
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


void DEUProxyNode::getObjects(IDList &listIDs) const
{
    std::vector<TileChildInfo>::const_iterator itorChild = m_vecChildrenInfo.begin();
    for( ; itorChild != m_vecChildrenInfo.end(); ++itorChild)
    {
        const TileChildInfo &child = *itorChild;
        listIDs.push_back(child.m_id);
    }
}


void DEUProxyNode::traverse(osg::NodeVisitor& nv)
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
            cmm::StateValue eStateValue = cmm::SV_Unknown;
            OpenSP::sp<cmm::IStateQuerier>  pStateQuerier;
            if(ms_pStateQuerier.lock(pStateQuerier))
            {
                eStateValue = pStateQuerier->queryObjectState(childInfo.m_id, cmm::STATE_VISIBLE);
            }

            childInfo.m_eVisible = eStateValue;
            if(eStateValue != cmm::SV_Enable)
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
    Group::traverse(nv);
}


DEUProxyGroup::DEUProxyGroup(void) : m_dblRadius(-1.0), m_nFragmentCount(0u)
{
}


DEUProxyGroup::~DEUProxyGroup(void)
{
}


bool DEUProxyGroup::create(const vtm::IVirtualTileManager *pVTManager, const ID &id, const ObjectMatrix &mtxObjectIDs, LODFixer *pLODFixer)
{
    const cmm::Pyramid *pPyramid = cmm::Pyramid::instance();

    osg::Vec2d ptMinCoord, ptMaxCoord;
    pPyramid->getTilePos(id.TileID.m_nLevel, id.TileID.m_nRow, id.TileID.m_nCol, ptMinCoord.x(), ptMinCoord.y(), ptMaxCoord.x(), ptMaxCoord.y());
    const osg::Vec2d ptCenterCoord((ptMinCoord + ptMaxCoord) * 0.5);

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    osg::Vec3d ptMinPos, ptMaxPos, ptCenterPos;
    pEllipsoidModel->convertLatLongHeightToXYZ(ptMinCoord.y(), ptMinCoord.x(), 0.0, ptMinPos.x(), ptMinPos.y(), ptMinPos.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptMaxCoord.y(), ptMaxCoord.x(), 0.0, ptMaxPos.x(), ptMaxPos.y(), ptMaxPos.z());
    pEllipsoidModel->convertLatLongHeightToXYZ(ptCenterCoord.y(), ptCenterCoord.x(), 0.0, ptCenterPos.x(), ptCenterPos.y(), ptCenterPos.z());

    const double dblRadius1 = (ptMinPos - ptMaxPos).length() * 0.5;
    const double dblRadius2 = (ptCenterPos - ptMinPos).length();
    const double dblRadius3 = (ptCenterPos - ptMaxPos).length();
    const double dblRadius  = std::max(dblRadius1, std::max(dblRadius2, dblRadius3));

    setCenter(ptCenterPos);
    setRadius(dblRadius);

    m_nFragmentCount = pVTManager->getTileFragmentCount();
    const double   dblFragCount      = m_nFragmentCount;
    const double   dblFragCount2     = m_nFragmentCount * m_nFragmentCount;
    const double   dblTileRangeRatio = pVTManager->getTileRangeRatio();
    const double   dblTileRange      = dblRadius * dblTileRangeRatio;

    const osg::Vec2d vecFragInterval((ptMaxCoord.x() - ptMinCoord.x()) / dblFragCount, (ptMaxCoord.y() - ptMinCoord.y()) / dblFragCount);

    ID idTileFragment = id;
    ID idTileProxyNode = id;
    idTileFragment.TileID.m_nType = V_TILE_FRAGMENT_ID;
    idTileProxyNode.TileID.m_nType = V_TILE_PROXY_NODE_ID;

    for(unsigned y = 0u; y < m_nFragmentCount; y++)
    {
        const ObjectRow &objectRow = mtxObjectIDs[y];
        for(unsigned x = 0u; x < m_nFragmentCount; x++)
        {
            const IDList &objSet = objectRow[x];

            DEUProxyNode *pProxyNode = new DEUProxyNode;
            pProxyNode->setID(idTileProxyNode);
            for(IDList::const_iterator itorObj = objSet.begin(); itorObj != objSet.end(); ++itorObj)
            {
                const ID &idObj = *itorObj;
                pProxyNode->addChildObject(idObj);
            }

            const double dblX = x + 0.5;
            const double dblY = y + 0.5;
            const osg::Vec2d ptFragCenterCoord(ptMinCoord.x() + dblX * vecFragInterval.x(), ptMinCoord.y() + dblY * vecFragInterval.y());

            osg::Vec3d ptFragCenter;
            pEllipsoidModel->convertLatLongHeightToXYZ(ptFragCenterCoord.y(), ptFragCenterCoord.x(), 0.0, ptFragCenter.x(), ptFragCenter.y(), ptFragCenter.z());

#if 0
            osg::LOD *pLOD = new osg::LOD;
            pLOD->setCenter(ptFragCenter);
            pLOD->setRadius(dblRadius / dblFragCount);
            pLOD->addChild(pProxyNode, 0.0, dblTileRange);
#else
            SmartLOD *pLOD = new SmartLOD;
            pLOD->setID(idTileFragment);
            pLOD->setCenter(ptFragCenter);
            pLOD->setRadius(dblRadius / dblFragCount);
            pLOD->addChild(pProxyNode, 0.0, dblTileRange);
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

    ID idThis = id;
    idThis.ObjectID.m_nType = V_TILE_PROXY_GROUP_ID;
    setID(idThis);

    return true;
}


DEUProxyNode *DEUProxyGroup::getProxyNode(unsigned nFragmentCount, unsigned nRow, unsigned nCol)
{
    const unsigned nChildIndex = nRow * nFragmentCount + nCol;
    osg::Group *pProxyParent = getChild(nChildIndex)->asGroup();
    DEUProxyNode *pProxyNode = dynamic_cast<DEUProxyNode *>(pProxyParent->getChild(0u));
    return pProxyNode;
}


osg::BoundingSphere DEUProxyGroup::computeBound(void) const
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


bool DEUProxyGroup::getChildByID(const ID &id, osg::ref_ptr<osg::Node> &pChildNode) const
{
    const unsigned nCount = m_nFragmentCount * m_nFragmentCount;
    for(unsigned i = 0u; i < nCount; i++)
    {
        const SmartLOD *pLOD = dynamic_cast<const SmartLOD *>(getChild(i));
        if(!pLOD)       continue;

        const DEUProxyNode *pProxyNode = dynamic_cast<const DEUProxyNode *>(pLOD->getChild(0u));
        if(!pProxyNode) continue;

        if(pProxyNode->getChildByID(id, pChildNode))
        {
            return true;
        }
    }

    return false;
}


void DEUProxyGroup::getObjects(IDList &listIDs) const
{
    listIDs.clear();
    const unsigned nCount = m_nFragmentCount * m_nFragmentCount;
    for(unsigned i = 0u; i < nCount; i++)
    {
        const SmartLOD *pLOD = dynamic_cast<const SmartLOD *>(getChild(i));
        if(!pLOD)       continue;

        const DEUProxyNode *pProxyNode = dynamic_cast<const DEUProxyNode *>(pLOD->getChild(0u));
        if(!pProxyNode) continue;

        pProxyNode->getObjects(listIDs);
    }
}


bool DEUProxyGroup::setVisible(const ID &id, bool bVisible)
{
    const unsigned nCount = m_nFragmentCount * m_nFragmentCount;
    for(unsigned i = 0u; i < nCount; i++)
    {
        SmartLOD *pLOD = dynamic_cast<SmartLOD *>(getChild(i));
        if(!pLOD)       continue;

        DEUProxyNode *pProxyNode = dynamic_cast<DEUProxyNode *>(pLOD->getChild(0u));
        if(!pProxyNode) continue;

        if(pProxyNode->setVisible(id, bVisible))
        {
            return true;
        }
    }
    return false;
}

