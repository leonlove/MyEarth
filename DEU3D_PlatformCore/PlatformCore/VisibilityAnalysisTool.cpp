#include "VisibilityAnalysisTool.h"
#include "Utility.h"
#include <vector>
#include "osg\AutoTransform"
#include "osg\SharedObjectPool"
#include "osgDB\ReadFile"

VisibilityAnalysisTool::VisibilityAnalysisTool(const std::string &strName)
	:AnalysisBaseTool(strName),
	m_bCenterPoint(false),
	m_bMovePoint(false),
	m_bEndPoint(false)
{
}



VisibilityAnalysisTool::~VisibilityAnalysisTool(void)
{
}

const std::string & VisibilityAnalysisTool::getType(void) const
{
	return VISIBILITYANALYSIS_TOOL;
}



bool VisibilityAnalysisTool::operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	const osgGA::GUIEventAdapter::EventType eEventType = ea.getEventType();
	//if(eEventType != osgGA::GUIEventAdapter::PUSH)
	//{
	//	if(m_pVertexArray->empty()) return false;
	//	if(!m_bLButtonDown)         return false;
	//}

	osg::Vec3d vIntersect(0.0, 0.0, 0.0);
	osg::Vec3d vIntersect2(0.0, 0.0, 0.0);
	osg::View *pView = dynamic_cast<osg::View *>(&aa);	
	osg::Camera *pCamera = pView->getCamera();
	const osg::Vec2d ptMouse(ea.getXnormalized(), ea.getYnormalized());
	if(!computeIntersection(m_pOperationTargetNode, pCamera, ptMouse, vIntersect2))
	{
		return false;
	
	}
	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
	pEllipsoidModel->convertXYZToLatLongHeight(vIntersect2.x(), vIntersect2.y(), vIntersect2.z(), vIntersect.y(), vIntersect.x(), vIntersect.z());
	double lat = osg::RadiansToDegrees(vIntersect.y());
	double lon = osg::RadiansToDegrees(vIntersect.x());
	vIntersect.set(lon,lat,vIntersect.z());
	/*if(eEventType != osgGA::GUIEventAdapter::PUSH)
	{
		return false;
	}*/

	switch(eEventType)
	{
	case(osgGA::GUIEventAdapter::PUSH):
		{
			if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				return false;
			}
			m_fMouseDownX = ea.getX();
			m_fMouseDownY = ea.getY();

		
			break;
		}
	case osgGA::GUIEventAdapter::MOVE:
	case osgGA::GUIEventAdapter::DRAG:
		{
			if(m_bCenterPoint)
			{
				m_bMovePoint = true;
				m_MovePoint = vIntersect;
				renderCircle(m_CenterPoint,m_MovePoint);
			}
			break;
		}
	case osgGA::GUIEventAdapter::RELEASE:
		{
			if(ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				return false;
			}
			m_fMouseUpX = ea.getX();
			m_fMouseUpY = ea.getY();

			double dx = m_fMouseDownX - m_fMouseUpX;
			double dy = m_fMouseDownY - m_fMouseUpY;
			//鼠标按下和松开，距离过远时，认为是在拖动球，而不是在做分析
			if ((dx * dx + dy * dy) > 5) 
			{
				return false;
			}
			if(!m_bCenterPoint)
			{
				m_bCenterPoint = true;
				m_CenterPoint = vIntersect;
				m_CenterPoint.set(vIntersect.x(),vIntersect.y(),vIntersect.z()+10);
			}
			else
			{
				m_bCenterPoint = false;
				m_EndPoint = vIntersect;
				//if(!isInit)
				{
					renderVisibilityResult(m_CenterPoint,m_EndPoint);
					//isInit = true;
				}
				
			}

		
			return true;
		}

	default:    return false;
	}



	return true;

}


void VisibilityAnalysisTool::setVisibilityMode(VISIBILITY_MODE mode)
{
	m_visibilityMode  = mode;
}

void VisibilityAnalysisTool::setCenterPoint(double lon,double lat,double height)
{
	//m_dCenterLon = lon;
	//m_dCenterLat = lat;
	//m_dCenterHeight = height;

	m_CenterPoint.set(lon,lat,height);
}

double VisibilityAnalysisTool::getVisibilityResult()
{
	return m_dResult;
}

double pi = 3.1415926;
double fDeg2Rad = pi / 180.0;
double fRad2Deg = 180.0 / pi;
double earthRadius = 6378137;


double calProjDistanceA2B(double lonA,double latA,double lonB,double latB)
{


	double dlon = lonB - lonA;
	double dlat = latB - latA;
	double l = sin(dlon * 0.5 * fDeg2Rad);
	double k = sin(dlat * 0.5 *fDeg2Rad);
	double a = k*k + cos(latA * fDeg2Rad) * cos(latB * fDeg2Rad) * l * l;
	double b = sqrt(a);
	double d = 1;
	double c = 2 * asin(b > d ? d : b)*180.0/pi;
	return c;
}

double lineProjectMeasure(const osg::Vec3d& point0, const osg::Vec3d& point1)
{
	double degree = calProjDistanceA2B(point0.x(),point0.y(),point1.x(),point1.y());
	return earthRadius * degree * fDeg2Rad;
}
void calculateInsertNum(osg::Vec3d& centerPoint,osg::Vec3d& targetPoint,double givenInterval,unsigned int& givenInsertNum,unsigned int& suggestInsertNum)
{
	double dis = lineProjectMeasure(centerPoint,targetPoint);
	givenInsertNum = ceil(dis / givenInterval);

	if(dis >10)
	{
		suggestInsertNum = ceil(sqrt(10 * log10(dis) * dis));
	}
	else
	{
		suggestInsertNum = givenInsertNum;
	}					

	if (suggestInsertNum > 20)
	{
		suggestInsertNum = 20;
	}
	else if (suggestInsertNum > givenInsertNum)
	{
		suggestInsertNum = givenInsertNum;
	}
}



void splitCircle2GeoPosition2(ISceneViewer* pViewer,double lati,double longi,double alt,double radius,double insertNum, std::vector<osg::Vec3d> &outPoints,double& minLon,double& minLat,double& maxLon,double& maxLat)
{
	double latd = lati * fDeg2Rad;
	double lond = longi * fDeg2Rad;
	double dir = radius / earthRadius;
	double dPI = pi;
	double longitude = 0.0;
	double latitude = 0.0;
	double altitude = 0.0;
	//outPoints.resize(insertNum);

	/*ev_int32 proMV = (this->mProgressM - this->mProgressV) >= 1 ? (this->mProgressM - this->mProgressV) : 1;
	ev_int32 proInterval = insertNum / proMV;
	ev_int32 counter = 0;*/

	for (int i = 0; i < insertNum; i++)
	{
		double tc = i * 2 * dPI / insertNum;
		latitude = asin(sin(latd) * cos(dir) + cos(latd) * sin(dir) * cos(tc));

		if (cos(latd) == 0)
		{
			longitude = lond;
		}
		else
		{
			double A = lond - sin(sin(tc) * sin(dir) / cos(latd)) + pi;
			double B = 2 * pi;
			longitude = A - (unsigned int)(A / B) * B - pi;
		}

		latitude *= fRad2Deg;
		longitude *= fRad2Deg;
		//altitude = pViewer->getHeightAt(longitude,latitude,true);
		altitude = alt;
		if(longitude > maxLon)
		{
			maxLon = longitude;
		}
		if(longitude < minLon)
		{
			minLon = longitude;
		}
		if(latitude > maxLat)
		{
			maxLat = latitude;
		}
		if(latitude < minLat)
		{
			minLat = latitude;
		}
		
		outPoints.push_back(osg::Vec3d(longitude,latitude,altitude));	

		
	}

	
}

void splitCircle2GeoPosition(ISceneViewer* pViewer,double lati,double longi,double radius,double insertNum, std::vector<osg::Vec3d> &outPoints,double& minLon,double& minLat,double& maxLon,double& maxLat)
{
	double latd = lati * fDeg2Rad;
	double lond = longi * fDeg2Rad;
	double dir = radius / earthRadius;
	double dPI = pi;
	double longitude = 0.0;
	double latitude = 0.0;
	double altitude = 0.0;
	//outPoints.resize(insertNum);

	/*ev_int32 proMV = (this->mProgressM - this->mProgressV) >= 1 ? (this->mProgressM - this->mProgressV) : 1;
	ev_int32 proInterval = insertNum / proMV;
	ev_int32 counter = 0;*/

	for (int i = 0; i < insertNum; i++)
	{
		double tc = i * 2 * dPI / insertNum;
		latitude = asin(sin(latd) * cos(dir) + cos(latd) * sin(dir) * cos(tc));

		if (cos(latd) == 0)
		{
			longitude = lond;
		}
		else
		{
			double A = lond - sin(sin(tc) * sin(dir) / cos(latd)) + pi;
			double B = 2 * pi;
			longitude = A - (unsigned int)(A / B) * B - pi;
		}

		latitude *= fRad2Deg;
		longitude *= fRad2Deg;
		altitude = pViewer->getHeightAt(longitude,latitude,true);
		if(longitude > maxLon)
		{
			maxLon = longitude;
		}
		if(longitude < minLon)
		{
			minLon = longitude;
		}
		if(latitude > maxLat)
		{
			maxLat = latitude;
		}
		if(latitude < minLat)
		{
			minLat = latitude;
		}
		
		outPoints.push_back(osg::Vec3d(longitude,latitude,altitude));	

		
	}

	
}
void splitLine2GeoPosition(ISceneViewer* pViewer,int number,double latA,double lonA,double latB,double lonB,bool isGetAlti,std::vector<osg::Vec3d> &outPoints)
{
	double altitude = 0;
	double d = calProjDistanceA2B(latA, lonA, latB, lonB) * fDeg2Rad;
	if(d == 0.0)
	{
		/*if (this->mpAltitudeListener)
		{
			altitude = this->mpAltitudeListener->getAltitude(latA, lonA);
		}*/
		
		outPoints.push_back(osg::Vec3d(lonA,latA,altitude));
		outPoints.push_back(osg::Vec3d(lonA,latA,altitude));
		return;
	}

	double sind = sin(d);
	double cosLat1 = cos(latA * fDeg2Rad);
	double cosLat2 = cos(latB * fDeg2Rad);
	//outPoints.resize(number + 1);

	/*ev_int32 proMV = (this->mProgressM - this->mProgressV) >= 1 ? (this->mProgressM - this->mProgressV) : 1;
	ev_int32 proInterval = (number + 1) / proMV;
	ev_int32 counter = 0;*/

	for (int i = 0; i <= number; i++)
	{
		double f = i * 1.0f / number;
		double A = sin((1-f) * d) / sind;
		double B = sin(f * d) / sind;
		double x = A * cosLat1 * cos(lonA * fDeg2Rad) + B * cosLat2 * cos(lonB * fDeg2Rad);
		double y = A * cosLat1 * sin(lonA * fDeg2Rad) + B * cosLat2 * sin(lonB * fDeg2Rad);
		double z = A * sin(latA * fDeg2Rad) + B * sin(latB * fDeg2Rad);
		double lat = atan2(z,(double)sqrt(x * x + y * y))*180.0/pi;
		double lon = atan2(y, x)*180.0/pi;
		altitude = pViewer->getHeightAt(lon,lat,true);
		//if(altitude)
		//{
		//	altitude = 0.0;
		//}

		/*	if (isGetAlti && NULL != this->mpAltitudeListener)
		{
		altitude = this->mpAltitudeListener->getAltitude(lat,lon);
		}*/
		outPoints.push_back(osg::Vec3d(lon,lat,altitude));


	}
}
struct VisibilityPoint
{
	bool IsVisibility;
	osg::Vec3d Position;
};
osg::Vec3d sphericalToCartesian(double lat,double lon,double height)
{
	/*lat *= fDeg2Rad;
	lon *= fDeg2Rad;
	double radCosLat = radius * cos(lat);
	return osg::Vec3d(radCosLat*cos(lon),radCosLat*sin(lon),radius*sin(lat));*/

	double dblLatitude  = osg::DegreesToRadians(lat);
	double dblLongitude = osg::DegreesToRadians(lon);
	osg::Vec3d xyzPos;
	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
	//从海拔10000米高空为起点构建射线
	pEllipsoidModel->convertLatLongHeightToXYZ(dblLatitude, dblLongitude, height, xyzPos.x(), xyzPos.y(), xyzPos.z());
	return xyzPos;
}
double calcuLineVisibility(ISceneViewer* pViewer,const osg::Vec3d& center, const double& centerH, const osg::Vec3d& target,
	const int& number, const bool& isViewShed,std::vector<VisibilityPoint> &outputPts)
{
	int lineShade = 0;
	int viewShedShade = 0;

	std::vector<osg::Vec3d> geoPoints; 
	
	splitLine2GeoPosition(pViewer,number,center.y(),center.x(),target.y(),target.x(),true,geoPoints);

	osg::Vec3d tempCenterPt = sphericalToCartesian(center.y(),center.x(),center.z()+ centerH );
	osg::Vec3d tempCenterPtBottom= sphericalToCartesian(center.y(),center.x(),center.z());
	VisibilityPoint pt;
	pt.IsVisibility = true;
	pt.Position = osg::Vec3d(tempCenterPt.x(),tempCenterPt.y(),tempCenterPt.z()-10);
	outputPts.push_back(pt);

	osg::Vec3d tempCenterPt2EarthCenter=-tempCenterPt;
	osg::Vec3d tempCenterPt2TargetPt(0.0,0.0,0.0);

	double tempCenterPt2EarthCenterLength=tempCenterPt2EarthCenter.length();
	double tempCenterPt2TargetPtLength=0.0;

	double tempCurrentAngle=0.0;
	//ev_real64 linek = 0;
	osg::Vec3d tempTargetPt(0.0,0.0,0.0);
	int geoPointCount = geoPoints.size();
	int num = (geoPointCount - 1) > 0 ? geoPointCount - 1 : 0;
	for (int i = 1; i <= num; i++)
	{
		tempTargetPt = sphericalToCartesian(geoPoints[i].y(), geoPoints[i].x(), geoPoints[i].z() );
		tempCenterPt2TargetPt=tempTargetPt-tempCenterPt;

		tempCenterPt2TargetPtLength=tempCenterPt2TargetPt.length();

		double tempNowAngle=acos(tempCenterPt2TargetPt*tempCenterPt2EarthCenter/
			(tempCenterPt2EarthCenterLength*tempCenterPt2TargetPtLength))*180.0/pi;
		if(i==1)
		{
			tempCurrentAngle=tempNowAngle;
			pt.IsVisibility=true;
		}
		else
		{
			if(tempNowAngle>=tempCurrentAngle)
			{
				tempCurrentAngle=tempNowAngle;
				pt.IsVisibility=true;
			}
			else
			{
				pt.IsVisibility=false;
				lineShade++;
				viewShedShade+=(i*2-1);
			}

		}
		
		//pt.Position = geoPoints[i];
		pt.Position = tempTargetPt;
		outputPts.push_back(pt);

	
	}

	//if (isViewShed)
	//{
	//	return viewShedShade * 1.0 / number / number;
	//}

	//return lineShade * 1.0 / number;
	if(number == 0)
		return 1;
	return lineShade * 1.0 / number;
}
double getVisibleAreaExtension(ISceneViewer* pViewer,osg::Vec3d& centerPoint,osg::Vec3d& targetPoint,std::vector<std::vector< VisibilityPoint>>& outPut,double& minLon,double& minLat,double& maxLon,double& maxLat)
{
	
	osg::Vec3d& tmpVie = centerPoint;
	osg::Vec3d& tmpTar = targetPoint;
	double alt = pViewer->getHeightAt(tmpTar.x(),tmpTar.y(),true);
	tmpTar.set(osg::Vec3d(tmpTar.x(),tmpTar.y(),alt));
	unsigned int givenInsertNum = 0;
	unsigned int suggestInsertNum = 0;
	calculateInsertNum(
		centerPoint,targetPoint,
		1,givenInsertNum,suggestInsertNum);
	int inNum = suggestInsertNum;
	std::vector<osg::Vec3d> mCircleGeoPoints;
	
	double radius = lineProjectMeasure(centerPoint,targetPoint);
	splitCircle2GeoPosition(pViewer,centerPoint.y(),centerPoint.x(),radius,360,mCircleGeoPoints,minLon,minLat,maxLon,maxLat);
	int CirclePointsCount = mCircleGeoPoints.size();
	double mShadeRate= 0.0;
	for (int i = 0;i < CirclePointsCount;i++)
	{
		std::vector< VisibilityPoint> outputPts;							
		mShadeRate += calcuLineVisibility(pViewer,
			centerPoint,0,
			mCircleGeoPoints.at(i),inNum,true,outputPts);
		outPut.push_back(outputPts);

	
	}

	mShadeRate /= mCircleGeoPoints.size();
	return mShadeRate;

}

osg::Node *createVisibiltyNode(osg::Vec3d& centerPoint,std::vector<std::vector< VisibilityPoint>>& points)
{
	osg::Vec3d center = sphericalToCartesian(centerPoint.y(),centerPoint.x(),centerPoint.z()-10);
	osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
	osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
	osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
	 osg::ref_ptr<osg::UIntArray> pIndexArray = new osg::UIntArray;
	
	osg::Vec3d vCenter = center;
	/*for(unsigned int i = 0; i < pVertexArray->size(); i++)
	{
	pVertex->push_back((*pVertexArray)[i]);
	vCenter += (*pVertexArray)[i];
	}

	vCenter /= (double)pVertex->size();*/
	int lineCount = points.size();
	if(lineCount < 1)
		return NULL;
	int linePointCount = points.at(0).size();
	int pointCount = lineCount * linePointCount;
	for(int i=0;i<lineCount;i++)
	{
		for(int j=0;j<linePointCount;j++)
		{
			VisibilityPoint& point = points.at(i).at(j);
			pVertex->push_back(point.Position - center);
			osg::Vec4 color = point.IsVisibility?osg::Vec4(0.f,1.f,0.f,1.f):osg::Vec4(1.f,0.f,0.f,1.f);			
			pColorArray->push_back(color);
			
			unsigned int index = j + i*linePointCount;
			unsigned int nextIndex = index + 1;
			if((j+1) != linePointCount)
			{
				pIndexArray->push_back(index);
				pIndexArray->push_back(nextIndex);
			}
			else
			{
				if((i == (lineCount -1)) && j == (linePointCount-1))
				{
					pIndexArray->push_back(index);
					pIndexArray->push_back(linePointCount-1);

				}
				else
				{
					pIndexArray->push_back(index);
					pIndexArray->push_back(index+linePointCount);

				}
				
			}
			
		}
	}

	

	osg::Matrix matrix;
	matrix.setTrans(vCenter);
	pMatrixTransform->setMatrix(matrix);

	osg::ref_ptr<osg::Geode>     pGeode      = new osg::Geode;
	osg::ref_ptr<osg::Geometry>  pGeometry   = new osg::Geometry;	


	//osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;

	osg::StateSet *pStateSet                 = pGeometry->getOrCreateStateSet();

	pGeometry->setVertexArray(pVertex);
	//pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, pVertex->size()));

	pGeometry->setColorArray(pColorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	//pGeometry->setNormalArray(n.get());
	//pGeometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	//n->push_back(osg::Vec3(0.f,-1.f,0.f));

		
	pGeometry->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::LINES, pIndexArray->size(), &pIndexArray->front()));

	
	
	pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	pGeode->addDrawable(pGeometry.get());

	pMatrixTransform->addChild(pGeode);
	return pMatrixTransform.release();
}

osg::Node * createTxtResult(double rate,double lon,double lat,double alt)
{
	std::string imagePath = "c:\\1.png";

	

	//上球矩阵
	osg::Vec3d vPoint(lon,lat,alt);

	osg::Vec3d vecTrans;
	osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
	pEllipsoidModel->convertLatLongHeightToXYZ(vPoint.y(), vPoint.x(), vPoint.z(), vecTrans.x(), vecTrans.y(), vecTrans.z());

	osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
	pMatrixTransform->setMatrix(osg::Matrix::translate(vecTrans));

	osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;

	osg::ref_ptr<osg::AutoTransform> pAutoTranseform = new osg::AutoTransform;
	pAutoTranseform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN );
	pAutoTranseform->setAutoScaleToScreen(true);
	pAutoTranseform->setCullingActive(false);

	pAutoTranseform->addChild(pGeode.get());

	osg::ref_ptr<osg::Image> pImage = osgDB::readImageFile(imagePath);;
	
	float m_fltImageWidth = 16;
	float m_fltImageHeight = 16;
	if(pImage.valid())
	{
		osg::ref_ptr<osg::Geometry> pImgGeometry = new osg::Geometry;
		osg::ref_ptr<osg::Texture> pTexture;

		osg::SharedObjectPool *pSharedObjectPool = osg::SharedObjectPool::instance();
		if(!pSharedObjectPool->findObject(pImage, pTexture))
		{
			osg::ref_ptr<osg::Texture2D> pTexture2D = new osg::Texture2D();
			pTexture2D->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
			pTexture2D->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
			pTexture2D->setResizeNonPowerOfTwoHint(false);
			pTexture2D->setImage(pImage.get());

			pSharedObjectPool->addTexture(pImage, pTexture2D);

			pTexture = pTexture2D;
		}

		osg::StateSet *pStateSet = new osg::StateSet;
		pStateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		pStateSet->setMode(GL_BLEND, osg::StateAttribute::ON); // redundant. AnnotationNode sets blending.
		pStateSet->setTextureAttributeAndModes(0, pTexture, osg::StateAttribute::ON);

		pImgGeometry->setUseVertexBufferObjects(true);

		pImgGeometry->setStateSet(pStateSet);

		osg::ref_ptr<osg::Vec3Array> pVertex = new osg::Vec3Array(4);
		
		(*pVertex)[0].set(-m_fltImageWidth * 0.5f, -m_fltImageHeight * 0.5f, 0.0f);
		(*pVertex)[1].set( m_fltImageWidth * 0.5f, -m_fltImageHeight * 0.5f, 0.0f);
		(*pVertex)[2].set( m_fltImageWidth * 0.5f,  m_fltImageHeight * 0.5f, 0.0f);
		(*pVertex)[3].set(-m_fltImageWidth * 0.5f,  m_fltImageHeight * 0.5f, 0.0f);

		pImgGeometry->setVertexArray(pVertex.get());
		if(pVertex->getVertexBufferObject())
		{
			pVertex->getVertexBufferObject()->setUsage(GL_STATIC_DRAW_ARB);
		}

		osg::ref_ptr<osg::Vec2Array> pTexcoords = new osg::Vec2Array(4);
		(*pTexcoords)[0].set(0, 0);
		(*pTexcoords)[1].set(1, 0);
		(*pTexcoords)[2].set(1, 1);
		(*pTexcoords)[3].set(0, 1);
		pImgGeometry->setTexCoordArray(0, pTexcoords.get());

		osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array(1);
		(*pColorArray)[0].set(1.0f,1.0f,1.0,1.0f);
		pImgGeometry->setColorArray(pColorArray.get());
		pImgGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

		pImgGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
		if (pImgGeometry.valid())
		{
			pGeode->addDrawable(pImgGeometry);
		}
	}

	std::string m_strTextFont = "SIMSUN.TTC";
	char tmpstr[50] = "";	
	sprintf(tmpstr, "%.2lf", rate);
	std::string m_strText = tmpstr;

	if(!m_strText.empty())
	{
		osg::ref_ptr<osgText::Text> pText = new osgText::Text();
		const std::wstring strStringW = cmm::ANSIToUnicode(m_strText);
		pText->setText(osgText::String(strStringW.c_str()));

		osg::ref_ptr<osgText::Font> pTextFont = osgText::readFontFile(m_strTextFont);
		pText->setFont(pTextFont);

		// osgText::Text turns on depth writing by default, even if you turned it off..
		pText->setEnableDepthWrites(false);

		pText->setLayout(osgText::TextBase::LEFT_TO_RIGHT);
		pText->setAlignment(osgText::Text::CENTER_TOP);

		pText->setPosition(osg::Vec3(0.0f, m_fltImageHeight + 16, 0.0f));

		pText->setAutoRotateToScreen(false);
		pText->setCharacterSizeMode(osgText::Text::OBJECT_COORDS);
		pText->setCharacterSize(16);

		pText->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		pText->setBackdropColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));
		pText->setBackdropType(osgText::Text::OUTLINE);

		//pText->getStateSet()->setRenderBinToInherit();
		pGeode->addDrawable(pText.get());
	}


	//pGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, false), 1);

	pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	pMatrixTransform->addChild(pAutoTranseform);

	return pMatrixTransform.release();
}

double VisibilityAnalysisTool::renderVisibilityResult(osg::Vec3d& centerPoint,osg::Vec3d& endPoint)
{
	 m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
	std::vector<std::vector< VisibilityPoint>> outPut;
	double maxLon = -180.0;
	double maxLat = -180.0;
	double minLon = 180.0;
	double minLat = 180.0;
	double rate = getVisibleAreaExtension(m_pSceneViewer,centerPoint,endPoint,outPut,minLon,minLat,maxLon,maxLat);
	osg::ref_ptr<osg::Node> pVisibiltyNode = createVisibiltyNode(centerPoint,outPut);
	osg::ref_ptr<osg::Node> pTxtNode = createTxtResult(rate,centerPoint.x(),centerPoint.y(),centerPoint.z()+10);

	m_pCurrentArtifactNode->addChild(pVisibiltyNode.get());
	m_pCurrentArtifactNode->addChild(pTxtNode.get());


	return rate;
}

void VisibilityAnalysisTool::renderCircle(osg::Vec3d& centerPoint,osg::Vec3d& endPoint)
{
	m_pCurrentArtifactNode->removeChildren(0u, m_pCurrentArtifactNode->getNumChildren());
	std::vector<std::vector< VisibilityPoint>> outPut;
	double maxLon = -180.0;
	double maxLat = -180.0;
	double minLon = 180.0;
	double minLat = 180.0;
	osg::Vec3d& tmpVie = centerPoint;
	osg::Vec3d& tmpTar = endPoint;
	double alt = m_pSceneViewer->getHeightAt(tmpTar.x(),tmpTar.y(),true);
	tmpTar.set(osg::Vec3d(tmpTar.x(),tmpTar.y(),alt));
	
	std::vector<osg::Vec3d> mCircleGeoPoints;

	double radius = lineProjectMeasure(centerPoint,endPoint);
	splitCircle2GeoPosition2(m_pSceneViewer,centerPoint.y(),centerPoint.x(),centerPoint.z()-10,radius,360,mCircleGeoPoints,minLon,minLat,maxLon,maxLat);

	osg::Vec3d center = sphericalToCartesian(centerPoint.y(),centerPoint.x(),centerPoint.z()-10);
	osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform;
	osg::ref_ptr<osg::Vec3dArray> pVertex = new osg::Vec3dArray;
	osg::ref_ptr<osg::Vec4Array> pColorArray = new osg::Vec4Array;
	

	osg::Vec3d vCenter = center;

	unsigned int count = mCircleGeoPoints.size();
	for(unsigned int i = 0;i < count ;i++)
	{
		osg::Vec3d point = sphericalToCartesian(mCircleGeoPoints.at(i).y(),mCircleGeoPoints.at(i).x(),mCircleGeoPoints.at(i).z());
		pVertex->push_back(point - center);				
		pColorArray->push_back(osg::Vec4(0.f,1.f,0.f,1.f));
	}



	osg::Matrix matrix;
	matrix.setTrans(vCenter);
	pMatrixTransform->setMatrix(matrix);

	osg::ref_ptr<osg::Geode>     pGeode      = new osg::Geode;
	osg::ref_ptr<osg::Geometry>  pGeometry   = new osg::Geometry;	

	pGeometry->setVertexArray(pVertex);
	pGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, pVertex->size()));

	pGeometry->setColorArray(pColorArray.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	pGeode->addDrawable(pGeometry.get());

	pMatrixTransform->addChild(pGeode);



	m_pCurrentArtifactNode->addChild(pMatrixTransform.release());

}
