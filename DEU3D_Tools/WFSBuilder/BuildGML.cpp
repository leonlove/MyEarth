#include "BuildGML.h"
#include <IDProvider/ID.h>
#include <IDProvider/Definer.h>
#include <ParameterSys/IParameter.h>
#include <ParameterSys/IPointParameter.h>
#include <ParameterSys/ILineParameter.h>
#include <ParameterSys/IFaceParameter.h>
#include <ParameterSys/IDetail.h>
#include <osg/Math>
#include <common/md2.h>

namespace wfsb
{
    BuildGML::BuildGML(void)
    {
        m_pBuilder = NULL;
        m_strUrl = "";
    }


    BuildGML::~BuildGML(void)
    {
    }

    void BuildGML::initialize(IModelBuilder* pModelBuilder,unsigned nDSCode,const std::string& strUrl,bool bReverse)
    {
        m_pBuilder = pModelBuilder;
        m_nDSCode = nDSCode;
        m_bReverse = bReverse;
        m_strUrl = strUrl;
    }

    bool BuildGML::convtParameter(logical::ILayer* pLayer,const std::string& strID,const std::vector<double>& ptVec,GMType gmType,const std::map<std::string,std::string>& attrMap)
    {
        std::string strLayer = pLayer->getName();
        std::string strIDText = strID + strLayer + m_strUrl;

        switch(gmType)
        {
        case GM_POINT:
            {
                if(ptVec.size() < 2)
                {
                    return false;
                }
                ID paramID(0,0,0);
                ID detailID(0,0,0);
                paramID.ObjectID.m_nType = PARAM_POINT_ID;
                paramID.ObjectID.m_nDataSetCode = m_nDSCode;
                cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&paramID.ObjectID.m_UniqueID);
                detailID.ObjectID.m_nType = DETAIL_DYN_POINT_ID;
                detailID.ObjectID.m_nDataSetCode = m_nDSCode;
                cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&detailID.ObjectID.m_UniqueID);

                param::IParameter* pParam = param::createParameter(paramID);
                param::IPointParameter* pPointParam = dynamic_cast<param::IPointParameter*>(pParam);

                param::IDetail* pDetail = param::createDetail(detailID);
                param::IDynPointDetail *pDynPointDetail = dynamic_cast<param::IDynPointDetail *>(pDetail);
                pDynPointDetail->setPointSize(5);
                cmm::FloatColor clr;
                clr.m_fltR = 1.0;
                clr.m_fltG = 0.0;
                clr.m_fltB = 0.0;
                clr.m_fltA = 0.0;
                pDynPointDetail->setPointColor(clr);
                m_pBuilder->buildDetail(pDynPointDetail);

                if(m_bReverse)
                {
                    cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
                    centerPoint.x() = osg::DegreesToRadians(ptVec[1]);
                    centerPoint.y() = osg::DegreesToRadians(ptVec[0]);
                    pPointParam->setCoordinate(centerPoint);
                }
                else
                {
                    cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
                    centerPoint.x() = osg::DegreesToRadians(ptVec[0]);
                    centerPoint.y() = osg::DegreesToRadians(ptVec[1]);
                    pPointParam->setCoordinate(centerPoint);
                }

                pPointParam->addDetail(detailID,0.0,10000000000.0);

                m_pBuilder->buildParamModel(pPointParam,NULL,attrMap);

                pLayer->addChild(paramID);
                return true;
            }
        case GM_LINE:
            {
                if(ptVec.size() < 4)
                {
                    return false;
                }

                ID paramID(0,0,0);
                ID detailID(0,0,0);

                paramID.ObjectID.m_nType = PARAM_LINE_ID;
                paramID.ObjectID.m_nDataSetCode = m_nDSCode;
                cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&paramID.ObjectID.m_UniqueID);
                detailID.ObjectID.m_nType = DETAIL_DYN_LINE_ID;
                detailID.ObjectID.m_nDataSetCode = m_nDSCode;
                cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&detailID.ObjectID.m_UniqueID);

                param::IParameter* pParam = param::createParameter(paramID);
                param::ILineParameter* pLineParam = dynamic_cast<param::ILineParameter*>(pParam);

                param::IDetail* pDetail = param::createDetail(detailID);
                param::IDynLineDetail *pDynLineDetail = dynamic_cast<param::IDynLineDetail *>(pDetail);
                cmm::FloatColor clr;
                clr.m_fltR = 1.0;
                clr.m_fltG = 0.0;
                clr.m_fltB = 0.0;
                clr.m_fltA = 0.0;
                pDynLineDetail->setLineColor(clr);
                pDynLineDetail->setLineWidth(1.0);

                m_pBuilder->buildDetail(pDynLineDetail);

                if(m_bReverse)
                {
                    for(unsigned n = 0;n < ptVec.size()/2;n++)
                    {
                        cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
                        centerPoint.x() = osg::DegreesToRadians(ptVec[n*2+1]);
                        centerPoint.y() = osg::DegreesToRadians(ptVec[n*2]);
                        pLineParam->addCoordinate(centerPoint);
                    }
                }
                else
                {
                    for(unsigned n = 0;n < ptVec.size()/2;n++)
                    {
                        cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
                        centerPoint.x() = osg::DegreesToRadians(ptVec[n*2]);
                        centerPoint.y() = osg::DegreesToRadians(ptVec[n*2+1]);
                        pLineParam->addCoordinate(centerPoint);
                    }
                }
                pLineParam->addPart(0,ptVec.size()/2);
                pLineParam->addDetail(detailID,0.0,10000000000.0);
                m_pBuilder->buildParamModel(pLineParam,NULL,attrMap);

                pLayer->addChild(paramID);
                return true;
            }  
        case GM_FACE:
            {
                if(ptVec.size() < 6)
                {
                    return false;
                }

                ID paramID(0,0,0);
                ID detailID(0,0,0);

                paramID.ObjectID.m_nType = PARAM_FACE_ID;
                paramID.ObjectID.m_nDataSetCode = m_nDSCode;
                cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&paramID.ObjectID.m_UniqueID);
                detailID.ObjectID.m_nType = DETAIL_DYN_FACE_ID;
                detailID.ObjectID.m_nDataSetCode = m_nDSCode;
                cmm::createHashMD2(strIDText.c_str(),strIDText.length(),(char*)&detailID.ObjectID.m_UniqueID);

                param::IParameter* pParam = param::createParameter(paramID);
                param::IFaceParameter* pFaceParam = dynamic_cast<param::IFaceParameter*>(pParam);

                param::IDetail* pDetail = param::createDetail(detailID);
                param::IDynFaceDetail *pDynFaceDetail = dynamic_cast<param::IDynFaceDetail *>(pDetail);
                cmm::FloatColor clr,fclr;
                clr.m_fltR = 1.0;
                clr.m_fltG = 0.0;
                clr.m_fltB = 0.0;
                clr.m_fltA = 1.0;

                fclr.m_fltR = 0.0;
                fclr.m_fltG = 0.0;
                fclr.m_fltB = 1.0;
                fclr.m_fltA = 1.0;

                pDynFaceDetail->setBorderColor(clr);
                pDynFaceDetail->setBorderWidth(1.0);
                pDynFaceDetail->setFaceColor(fclr);

                m_pBuilder->buildDetail(pDynFaceDetail);

                if(m_bReverse)
                {
                    for(unsigned n = 0;n < ptVec.size()/2;n++)
                    {
                        cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
                        centerPoint.x() = osg::DegreesToRadians(ptVec[n*2+1]);
                        centerPoint.y() = osg::DegreesToRadians(ptVec[n*2]);
                        pFaceParam->addCoordinate(centerPoint);
                    }
                }
                else
                {
                    for(unsigned n = 0;n < ptVec.size()/2;n++)
                    {
                        cmm::math::Point3d centerPoint(0.0, 0.0, 0.0);
                        centerPoint.x() = osg::DegreesToRadians(ptVec[n*2]);
                        centerPoint.y() = osg::DegreesToRadians(ptVec[n*2+1]);
                        pFaceParam->addCoordinate(centerPoint);
                    }
                }

                pFaceParam->addPart(0,ptVec.size()/2);

                pFaceParam->addDetail(detailID,0.0,10000000000.0);
                m_pBuilder->buildParamModel(pFaceParam,NULL,attrMap);
                pLayer->addChild(paramID);
                return true;
            }
        default:
            return false;
        }
        return false;
    }
}

