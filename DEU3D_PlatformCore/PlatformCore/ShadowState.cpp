#include "ShadowState.h"

#include <osg/NodeVisitor>
#include <osg/CoordinateSystemNode>
#include <osg/PagedLOD>
#include <ParameterSys/Detail.h>

//标识阴影接收对象
const int ReceivesShadowTraversalMask = 0x1;
//标识阴影投影对象
const int CastsShadowTraversalMask = 0x2;

ShadowState::ShadowState(const std::string &strName) : StateBase(strName)
{
    
}


ShadowState::~ShadowState(void)
{
    
}


bool ShadowState::applyState(osg::Node *pNode, bool bApply)
{

    if(!pNode)
    {
        return false;
    }

    if (bApply)
    {
        //pNode->setNodeMask(CastsShadowTraversalMask);
        
		/*osg::ref_ptr<osg::Group> pPointParam = dynamic_cast<osg::Group*>(pNode);
		osg::ref_ptr<osg::PagedLOD> pPagedLOD = dynamic_cast<osg::PagedLOD*>(pPointParam->getChild(0));

		const param::Detail::PointCreationInfo *pInfo = dynamic_cast<const param::Detail::PointCreationInfo *>(pPagedLOD->getChildCreationInfo());

		osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
		osg::Vec3d ptCenter;
		pEllipsoidModel->convertXYZToLatLongHeight(pNode->getBound().center().x(), pNode->getBound().center().y(), pNode->getBound().center().z(),
		ptCenter.y(), ptCenter.x(), ptCenter.z());

		osg::Vec3d ptCreationPoints = pInfo->m_pPoints->front();*/

        

        pNode->setFlag(1);

    }
    else
    {
        //pNode->setNodeMask(ReceivesShadowTraversalMask);
        pNode->setFlag(0);
    }

    return true;

}