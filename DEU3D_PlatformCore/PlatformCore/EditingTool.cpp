#include "EditingTool.h"

#include <IDProvider/Definer.h>
#include <osgUtil/FindNodeVisitor.h>
#include <ParameterSys/Detail.h>
#include <osgDB/ReadFile>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include "Utility.h"

EditingTool::EditingTool(const std::string &strName) : ToolBase(strName), m_dblUnit(1.0), m_nEditingIndex(-1), m_clrPoint(0.0, 1.0, 1.0, 1.0)
{
}


EditingTool::~EditingTool(void)
{
}

bool EditingTool::setEditingTarget(const ID &id)
{
    printf("%s\n", id.toString().c_str());
    m_TargetID = id;
    m_pEditingNode = NULL;
    return true;
}
const ID &EditingTool::getEditingTarget(void) const
{
    return m_TargetID;
}

void EditingTool::setUnit(double dblUnit)
{
    m_dblUnit = dblUnit;
}

double EditingTool::getUnit(void)
{
    return m_dblUnit;
}

bool EditingTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    if(!m_TargetID.isValid())
    {
        return false;
    }

    const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

    //查找编辑对象，
    if(m_pEditingNode == NULL)
    {
        osg::ref_ptr<osgUtil::FindNodeByIDVisitor>    pFinder1 = new osgUtil::FindNodeByIDVisitor(m_TargetID);
        m_pOperationTargetNode->asGroup()->getChild(1)->accept(*pFinder1);
        m_pEditingNode = pFinder1->getTargetNode();

        osg::Group *pGroup = m_pEditingNode->asGroup();

        osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(0));
        param::Detail::CreationInfo *pCreateInfo = dynamic_cast<param::Detail::CreationInfo *>(pPagedLOD->getChildCreationInfo());

        osg::ref_ptr<osg::Group> pNewGroup = new osg::Group;
        pNewGroup->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

        double dblX, dblY, dblZ;

        osg::Vec3dArray *pPoints = pCreateInfo->m_pPoints.get();
        unsigned int nPointCount = pPoints->size();
        for(unsigned int i = 0; i < nPointCount; i++)
        {
            osg::Vec3d &vPoint = (*pPoints)[i];
            pEllipsoidModel->convertLatLongHeightToXYZ(vPoint._v[1], vPoint._v[0], vPoint._v[2], dblX, dblY, dblZ);
            osg::Matrixd mtx;
            mtx.setTrans(osg::Vec3d(dblX, dblY, dblZ));

            osg::ref_ptr<osg::MatrixTransform> pMT = new osg::MatrixTransform(mtx);
            char dest[255];
            sprintf(dest, "%d\n", i);
            std::string strPointName = dest;
            pMT->setName(strPointName);

            osg::ref_ptr<osg::Sphere> pSphere = new osg::Sphere(osg::Vec3d(0.0, 0.0, 0.0), 1.0);
            osg::ref_ptr<osg::ShapeDrawable> pShapeDrawable = new osg::ShapeDrawable(pSphere.get());
            pShapeDrawable->setColor(m_clrPoint);
            osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
            pGeode->addDrawable(pShapeDrawable.get());

            pMT->addChild(pGeode.get());

            pNewGroup->addChild(pMT.get());
        }

        m_pCurrentArtifactNode->addChild(pNewGroup.get());
    }
    else
    {
        switch(ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::PUSH):
            {
                if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                {
                    return false;
                }

                osg::Vec3d vIntersect(0.0, 0.0, 0.0);
                osg::View *pView = dynamic_cast<osg::View *>(&aa);
                osg::Camera *pCamera = pView->getCamera();
                const osg::Vec2d ptMouse(ea.getXnormalized(), ea.getYnormalized());
                osg::Node *pInterNode = NULL;
                if(!computeIntersection(m_pCurrentArtifactNode, pCamera, ptMouse, vIntersect, &pInterNode))
                {
                    if(m_nEditingIndex >= 0)
                    {
                        osg::ShapeDrawable *pShapeDrawable = dynamic_cast<osg::ShapeDrawable *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(m_nEditingIndex)->asGroup()->getChild(0)->asGeode()->getDrawable(0));
                        pShapeDrawable->setColor(m_clrPoint);
                        m_nEditingIndex = -1;
                    }
                    return false;
                }

                unsigned int nTempIndex = atoi(pInterNode->getParent(0)->getName().c_str());
                if(m_nEditingIndex != nTempIndex)
                {
                    if(m_nEditingIndex >= 0)
                    {
                        osg::ShapeDrawable *pShapeDrawable = dynamic_cast<osg::ShapeDrawable *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(m_nEditingIndex)->asGroup()->getChild(0)->asGeode()->getDrawable(0));
                        pShapeDrawable->setColor(m_clrPoint);
                    }

                    osg::ShapeDrawable *pShapeDrawable = dynamic_cast<osg::ShapeDrawable *>(pInterNode->asGeode()->getDrawable(0));
                    pShapeDrawable->setColor(osg::Vec4(0.0, 0.0, 1.0, 1.0));

                    m_nEditingIndex = nTempIndex;
                }

                m_bLButtonDown = true;
                return true;
            }
        case(osgGA::GUIEventAdapter::RELEASE):
            {
                if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                {
                    return false;
                }

                m_bLButtonDown = false;
                if(m_pEventAdapter)
                {
                    osg::Group *pGroup = m_pEditingNode->asGroup();
                    osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(0));
                    param::Detail::CreationInfo *pCreateInfo = dynamic_cast<param::Detail::CreationInfo *>(pPagedLOD->getChildCreationInfo());
                    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                    pEventObject->setAction(ea::ACTION_EDITING_TOOL);

                    pEventObject->putExtra("ToolName", m_strName);
                    pEventObject->putExtra("ID", m_TargetID.toString());
                    bool bFinished = true;
                    pEventObject->putExtra("Finished", bFinished);
                    std::vector<double> vecPointList;
                    unsigned int nNumPoints = pCreateInfo->m_pPoints->size();
                    for(unsigned int i = 0; i < nNumPoints; i++)
                    {
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[0]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[1]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[2]);
                    }
                    pEventObject->putExtra("Vertices", vecPointList);
                    m_pEventAdapter->sendBroadcast(pEventObject);
                }
                return true;
            }
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::DRAG):
            {
                if(!m_bLButtonDown)     return false;

                osg::Vec3d vIntersect(0.0, 0.0, 0.0);
                osg::View *pView = dynamic_cast<osg::View *>(&aa);
                osg::Camera *pCamera = pView->getCamera();
                const osg::Vec2d ptMouse(ea.getXnormalized(), ea.getYnormalized());
                if(!computeIntersection(m_pOperationTargetNode->asGroup()->getChild(0), pCamera, ptMouse, vIntersect))
                {
                    return false;
                }

                osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
                double dblLon, dblLat, dblHeight;
                pEllipsoidModel->convertXYZToLatLongHeight(vIntersect._v[0], vIntersect._v[1], vIntersect._v[2], dblLat, dblLon, dblHeight);

                osg::Group *pGroup = m_pEditingNode->asGroup();
                osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(0));
                param::Detail::CreationInfo *pCreateInfo = dynamic_cast<param::Detail::CreationInfo *>(pPagedLOD->getChildCreationInfo());
                (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[0] = dblLon;
                (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[1] = dblLat;

                double dblX, dblY, dblZ;
                pEllipsoidModel->convertLatLongHeightToXYZ(dblLat, dblLon, (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[2], dblX, dblY, dblZ);
                osg::Matrixd mtx;
                mtx.setTrans(osg::Vec3d(dblX, dblY, dblZ));
                osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(m_nEditingIndex));
                pMT->setMatrix(mtx);

                if(m_TargetID.ObjectID.m_nType == PARAM_POINT_ID)
                {
                    for(unsigned int i = 0; i < pPagedLOD->getNumChildren(); i++)
                    {
                        osg::Node *pChild = pPagedLOD->getChild(i);
                        const ID c_id = pChild->getID();
                        osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pCreateInfo);
                        pPagedLOD->replaceChild(pChild, pNewChild.get());
                    }
                }
                else
                {
                    unsigned int nNumChild = pGroup->getNumChildren();
                    for(unsigned int i = 0; i < nNumChild; i++)
                    {
                        osg::PagedLOD *pTempPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(i));
                        param::Detail::PolyCreationInfo *pPolyCreateInfo = dynamic_cast<param::Detail::PolyCreationInfo *>(pTempPagedLOD->getChildCreationInfo());
                        if(m_nEditingIndex >= pPolyCreateInfo->m_nOffset && m_nEditingIndex <= pPolyCreateInfo->m_nOffset + pPolyCreateInfo->m_nCount)
                        {
                            for(unsigned int j = 0; j < pTempPagedLOD->getNumChildren(); j++)
                            {
                                osg::Node *pChild = pTempPagedLOD->getChild(j);
                                const ID c_id = pChild->getID();
                                osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pPolyCreateInfo);
                                pTempPagedLOD->replaceChild(pChild, pNewChild.get());
                            }
                        }
                    }
                }

                if(m_pEventAdapter)
                {
                    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                    pEventObject->setAction(ea::ACTION_EDITING_TOOL);

                    pEventObject->putExtra("ToolName", m_strName);
                    pEventObject->putExtra("ID", m_TargetID.toString());
                    bool bFinished = false;
                    pEventObject->putExtra("Finished", bFinished);
                    std::vector<double> vecPointList;
                    unsigned int nNumPoints = pCreateInfo->m_pPoints->size();
                    for(unsigned int i = 0; i < nNumPoints; i++)
                    {
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[0]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[1]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[2]);
                    }
                    pEventObject->putExtra("Vertices", vecPointList);
                    m_pEventAdapter->sendBroadcast(pEventObject);
                }
                return true;
            }
        case(osgGA::GUIEventAdapter::SCROLL):
            {
                double dblDeltaZ = ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP ? m_dblUnit : - m_dblUnit;

                osg::Group *pGroup = m_pEditingNode->asGroup();
                osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(0));
                param::Detail::CreationInfo *pCreateInfo = dynamic_cast<param::Detail::CreationInfo *>(pPagedLOD->getChildCreationInfo());
                unsigned int nPointCount = pCreateInfo->m_pPoints->size();
                //对每一个点进行编辑
                if(m_nEditingIndex < 0)
                {
                    for(unsigned int i = 0; i < nPointCount; i++)
                    {
                        (*pCreateInfo->m_pPoints)[i]._v[2] += dblDeltaZ;

                        double dblX, dblY, dblZ;
                        pEllipsoidModel->convertLatLongHeightToXYZ((*pCreateInfo->m_pPoints)[i]._v[1], (*pCreateInfo->m_pPoints)[i]._v[0], (*pCreateInfo->m_pPoints)[i]._v[2], dblX, dblY, dblZ);
                        osg::Matrixd mtx;
                        mtx.setTrans(osg::Vec3d(dblX, dblY, dblZ));
                        osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(i));
                        pMT->setMatrix(mtx);
                    }

                    unsigned int nNumChild = pGroup->getNumChildren();
                    for(unsigned int i = 0; i < nNumChild; i++)
                    {
                        osg::PagedLOD *pTempPageLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(i));
                        for(unsigned int j = 0; j < pTempPageLOD->getNumChildren(); j++)
                        {
                            osg::Node *pChild = pTempPageLOD->getChild(j);
                            const ID c_id = pChild->getID();
                            osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pTempPageLOD->getChildCreationInfo());
                            pTempPageLOD->replaceChild(pChild, pNewChild.get());
                        }

                    }
                }
                //对指定点进行编辑
                else
                {
                    (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[2] += dblDeltaZ;

                    double dblX, dblY, dblZ;
                    pEllipsoidModel->convertLatLongHeightToXYZ((*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[1], (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[0], (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[2], dblX, dblY, dblZ);
                    osg::Matrixd mtx;
                    mtx.setTrans(osg::Vec3d(dblX, dblY, dblZ));
                    osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(m_nEditingIndex));
                    pMT->setMatrix(mtx);

                    if(m_TargetID.ObjectID.m_nType == PARAM_POINT_ID)
                    {
                        for(unsigned int i = 0; i < pPagedLOD->getNumChildren(); i++)
                        {
                            osg::Node *pChild = pPagedLOD->getChild(i);
                            const ID c_id = pChild->getID();
                            osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pCreateInfo);
                            pPagedLOD->replaceChild(pChild, pNewChild.get());
                        }
                    }
                    else
                    {
                        unsigned int nNumChild = pGroup->getNumChildren();
                        for(unsigned int i = 0; i < nNumChild; i++)
                        {
                            osg::PagedLOD *pTempPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(i));
                            param::Detail::PolyCreationInfo *pPolyCreateInfo = dynamic_cast<param::Detail::PolyCreationInfo *>(pTempPagedLOD->getChildCreationInfo());
                            if(m_nEditingIndex >= pPolyCreateInfo->m_nOffset && m_nEditingIndex <= pPolyCreateInfo->m_nOffset + pPolyCreateInfo->m_nCount)
                            {
                                for(unsigned int j = 0; j < pTempPagedLOD->getNumChildren(); j++)
                                {
                                    osg::Node *pChild = pTempPagedLOD->getChild(j);
                                    const ID c_id = pChild->getID();
                                    osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pPolyCreateInfo);
                                    pTempPagedLOD->replaceChild(pChild, pNewChild.get());
                                }
                            }
                        }
                    }
                }
                if(m_pEventAdapter)
                {
                    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                    pEventObject->setAction(ea::ACTION_EDITING_TOOL);

                    pEventObject->putExtra("ToolName", m_strName);
                    pEventObject->putExtra("ID", m_TargetID.toString());
                    bool bFinished = true;
                    pEventObject->putExtra("Finished", bFinished);
                    std::vector<double> vecPointList;
                    for(unsigned int i = 0; i < nPointCount; i++)
                    {
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[0]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[1]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[2]);
                    }
                    pEventObject->putExtra("Vertices", vecPointList);
                    m_pEventAdapter->sendBroadcast(pEventObject);
                }
            }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        case(osgGA::GUIEventAdapter::KEYUP):
            {
                osg::Vec3d vtrans, vscale;
                osg::Quat qr, qs;

                unsigned int nPoints = m_pCurrentArtifactNode->getChild(0)->asGroup()->getNumChildren();
                osg::Group *pGroup = m_pEditingNode->asGroup();
                osg::PagedLOD *pPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(0));
                param::Detail::CreationInfo *pCreateInfo = dynamic_cast<param::Detail::CreationInfo *>(pPagedLOD->getChildCreationInfo());
                unsigned int nPointCount = pCreateInfo->m_pPoints->size();

                if(m_nEditingIndex < 0)
                {
                    int nKey = ea.getKey();
                    if(nKey == 'a' || nKey == 'A')//osgGA::GUIEventAdapter::KEY_Left)
                    {
                        for(unsigned int i = 0; i < nPoints; i++)
                        {
                            osg::Matrixd mtx;
                            pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight((*pCreateInfo->m_pPoints)[i]._v[1], (*pCreateInfo->m_pPoints)[i]._v[0], (*pCreateInfo->m_pPoints)[i]._v[2], mtx);
                            mtx.preMultTranslate(osg::Vec3d(-m_dblUnit, 0.0, 0.0));
                            osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(i));
                            pMT->setMatrix(mtx);

                            mtx.decompose(vtrans, qr, vscale, qs);
                            double dblLon, dblLat, dblHeight;
                            pEllipsoidModel->convertXYZToLatLongHeight(vtrans._v[0], vtrans._v[1], vtrans._v[2], dblLat, dblLon, dblHeight);
                            (*pCreateInfo->m_pPoints)[i]._v[0] = dblLon;
                            (*pCreateInfo->m_pPoints)[i]._v[1] = dblLat;
                        }
                    }
                    else if(nKey == 'd' || nKey == 'D')//osgGA::GUIEventAdapter::KEY_Right)
                    {
                        for(unsigned int i = 0; i < nPoints; i++)
                        {
                            osg::Matrixd mtx;
                            pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight((*pCreateInfo->m_pPoints)[i]._v[1], (*pCreateInfo->m_pPoints)[i]._v[0], (*pCreateInfo->m_pPoints)[i]._v[2], mtx);
                            mtx.preMultTranslate(osg::Vec3d(m_dblUnit, 0.0, 0.0));
                            osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(i));
                            pMT->setMatrix(mtx);

                            mtx.decompose(vtrans, qr, vscale, qs);
                            double dblLon, dblLat, dblHeight;
                            pEllipsoidModel->convertXYZToLatLongHeight(vtrans._v[0], vtrans._v[1], vtrans._v[2], dblLat, dblLon, dblHeight);
                            (*pCreateInfo->m_pPoints)[i]._v[0] = dblLon;
                            (*pCreateInfo->m_pPoints)[i]._v[1] = dblLat;
                        }
                    }
                    else if(nKey == 'w' || nKey == 'W')//osgGA::GUIEventAdapter::KEY_Up)
                    {
                        for(unsigned int i = 0; i < nPoints; i++)
                        {
                            osg::Matrixd mtx;
                            pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight((*pCreateInfo->m_pPoints)[i]._v[1], (*pCreateInfo->m_pPoints)[i]._v[0], (*pCreateInfo->m_pPoints)[i]._v[2], mtx);
                            mtx.preMultTranslate(osg::Vec3d(0.0, m_dblUnit, 0.0));
                            osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(i));
                            pMT->setMatrix(mtx);

                            mtx.decompose(vtrans, qr, vscale, qs);
                            double dblLon, dblLat, dblHeight;
                            pEllipsoidModel->convertXYZToLatLongHeight(vtrans._v[0], vtrans._v[1], vtrans._v[2], dblLat, dblLon, dblHeight);
                            (*pCreateInfo->m_pPoints)[i]._v[0] = dblLon;
                            (*pCreateInfo->m_pPoints)[i]._v[1] = dblLat;
                        }
                    }
                    else if(nKey == 's' || nKey == 'S')//osgGA::GUIEventAdapter::KEY_Down)
                    {
                        for(unsigned int i = 0; i < nPoints; i++)
                        {
                            osg::Matrixd mtx;
                            pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight((*pCreateInfo->m_pPoints)[i]._v[1], (*pCreateInfo->m_pPoints)[i]._v[0], (*pCreateInfo->m_pPoints)[i]._v[2], mtx);
                            mtx.preMultTranslate(osg::Vec3d(0.0, -m_dblUnit, 0.0));
                            osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(i));
                            pMT->setMatrix(mtx);

                            mtx.decompose(vtrans, qr, vscale, qs);
                            double dblLon, dblLat, dblHeight;
                            pEllipsoidModel->convertXYZToLatLongHeight(vtrans._v[0], vtrans._v[1], vtrans._v[2], dblLat, dblLon, dblHeight);
                            (*pCreateInfo->m_pPoints)[i]._v[0] = dblLon;
                            (*pCreateInfo->m_pPoints)[i]._v[1] = dblLat;
                        }
                    }
                    else return true;

                    unsigned int nNumChild = pGroup->getNumChildren();
                    for(unsigned int i = 0; i < nNumChild; i++)
                    {
                        osg::PagedLOD *pTempPagedLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(i));
                        for(unsigned int i = 0; i < pTempPagedLOD->getNumChildren(); i++)
                        {
                            osg::Node *pChild = pTempPagedLOD->getChild(i);
                            const ID c_id = pChild->getID();
                            osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pTempPagedLOD->getChildCreationInfo());
                            pTempPagedLOD->replaceChild(pChild, pNewChild.get());
                        }
                    }
                }
                else
                {
                    osg::Matrixd mtx;
                    pEllipsoidModel->computeLocalToWorldTransformFromLatLongHeight((*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[1], (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[0], (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[2], mtx);

                    int nKey = ea.getKey();
                    if(nKey == 'a' || nKey == 'A')//osgGA::GUIEventAdapter::KEY_Left)
                    {
                        mtx.preMultTranslate(osg::Vec3d(-m_dblUnit, 0.0, 0.0));
                    }
                    else if(nKey == 'd' || nKey == 'D')//osgGA::GUIEventAdapter::KEY_Right)
                    {
                        mtx.preMultTranslate(osg::Vec3d(m_dblUnit, 0.0, 0.0));
                    }
                    else if(nKey == 'w' || nKey == 'W')//osgGA::GUIEventAdapter::KEY_Up)
                    {
                        mtx.preMultTranslate(osg::Vec3d(0.0, m_dblUnit, 0.0));
                    }
                    else if(nKey == 's' || nKey == 'S')//osgGA::GUIEventAdapter::KEY_Down)
                    {
                        mtx.preMultTranslate(osg::Vec3d(0.0, -m_dblUnit, 0.0));
                    }
                    else return true;

                    osg::MatrixTransform *pMT = dynamic_cast<osg::MatrixTransform *>(m_pCurrentArtifactNode->getChild(0)->asGroup()->getChild(m_nEditingIndex));
                    pMT->setMatrix(mtx);

                    mtx.decompose(vtrans, qr, vscale, qs);
                    double dblLon, dblLat, dblHeight;
                    pEllipsoidModel->convertXYZToLatLongHeight(vtrans._v[0], vtrans._v[1], vtrans._v[2], dblLat, dblLon, dblHeight);
                    (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[0] = dblLon;
                    (*pCreateInfo->m_pPoints)[m_nEditingIndex]._v[1] = dblLat;

                    if(m_TargetID.ObjectID.m_nType == PARAM_POINT_ID)
                    {
                        for(unsigned int i = 0; i < pPagedLOD->getNumChildren(); i++)
                        {
                            osg::Node *pChild = pPagedLOD->getChild(i);
                            const ID c_id = pChild->getID();
                            osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pCreateInfo);
                            pPagedLOD->replaceChild(pChild, pNewChild.get());
                        }
                    }
                    else
                    {
                        unsigned int nNumChild = pGroup->getNumChildren();
                        for(unsigned int i = 0; i < nNumChild; i++)
                        {
                            osg::PagedLOD *pTempPageLOD = dynamic_cast<osg::PagedLOD *>(pGroup->getChild(i));
                            param::Detail::PolyCreationInfo *pPolyCreateInfo = dynamic_cast<param::Detail::PolyCreationInfo *>(pTempPageLOD->getChildCreationInfo());
                            if(m_nEditingIndex >= pPolyCreateInfo->m_nOffset && m_nEditingIndex <= pPolyCreateInfo->m_nOffset + pPolyCreateInfo->m_nCount)
                            {
                                for(unsigned int j = 0; j < pTempPageLOD->getNumChildren(); j++)
                                {
                                    osg::Node *pChild = pTempPageLOD->getChild(j);
                                    const ID c_id = pChild->getID();
                                    osg::ref_ptr<osg::Node> pNewChild = osgDB::readNodeFile(c_id, pPolyCreateInfo);
                                    pTempPageLOD->replaceChild(pChild, pNewChild.get());
                                }
                            }
                        }
                    }
                }
                if(m_pEventAdapter)
                {
                    OpenSP::sp<ea::IEventObject> pEventObject = ea::createEventObject();
                    pEventObject->setAction(ea::ACTION_EDITING_TOOL);

                    pEventObject->putExtra("ToolName", m_strName);
                    pEventObject->putExtra("ID", m_TargetID.toString());
                    bool bFinished = true;
                    pEventObject->putExtra("Finished", bFinished);
                    std::vector<double> vecPointList;
                    for(unsigned int i = 0; i < nPointCount; i++)
                    {
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[0]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[1]);
                        vecPointList.push_back((*pCreateInfo->m_pPoints)[i]._v[2]);
                    }
                    pEventObject->putExtra("Vertices", vecPointList);
                    m_pEventAdapter->sendBroadcast(pEventObject);
                }
                return true;
            }
        default: return false;
        }
    }
    return false;
}

