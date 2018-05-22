#include "ParmRectifyThreadPool.h"

#include "PointParameterNode.h"
#include "LineParameterNode.h"
#include "Registry.h "

#include <Common/Pyramid.h>
#include <osgUtil/LineSegmentIntersector>
#include <Common/deuImage.h>

class FindTerrainNodeVisitor : public osg::NodeVisitor
{
public:
    explicit FindTerrainNodeVisitor(osg::Vec2d &vPos) : 
        NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
            setPosition(vPos);
        }

    virtual ~FindTerrainNodeVisitor(void)   {}
public:
    void setPosition(osg::Vec2d &vPos)
    {
        m_vPos.set(osg::RadiansToDegrees(vPos.x()), osg::RadiansToDegrees(vPos.y()));
        m_pTerrainNode = NULL;
    }

    virtual void apply(osg::Group &node)
    {
        osgTerrain::TerrainTile *pTT = dynamic_cast<osgTerrain::TerrainTile *>(&node);
        if(pTT == NULL)
        {
            return osg::NodeVisitor::apply(node);
        }

        const ID &id = pTT->getID();

        cmm::Pyramid pyr;
        double xMin, xMax, yMin, yMax;
        pyr.GetTilePos(id.TileID.m_nLevel, id.TileID.m_nCol, id.TileID.m_nRow, xMin, yMin, xMax, yMax);
        if(xMin <= m_vPos._v[0] && xMax >= m_vPos._v[0] && yMin <= m_vPos._v[1] && yMax >= m_vPos._v[1])
        {
            if(m_pTerrainNode.valid())
            {
                if(m_pTerrainNode->getID().TileID.m_nLevel <= id.TileID.m_nLevel && id.TileID.m_nLevel <= 15)
                {
                    m_pTerrainNode = pTT;
                }
            }
            else
            {
                m_pTerrainNode = pTT;
            }
        }
    }

    osgTerrain::TerrainTile *getTerrainNode(void)
    {
        return m_pTerrainNode.get();
    }

protected:
    osg::Vec2d m_vPos;
    osg::ref_ptr<osgTerrain::TerrainTile> m_pTerrainNode;
};

int ParmRectifyThreadPool::ParmRectifyThread::cancel()
{
    if(isRunning() )
    {
        setDone(true);

        while(isRunning())
        {
            OpenThreads::Thread::YieldCurrentThread();
        }
    }

    return 0;
}

void ParmRectifyThreadPool::ParmRectifyThread::run()
{
    OpenSP::sp<ParmRectifyThreadPool::ParmRectifyTaskQueue> task_queue;

    task_queue = m_pThreadPool->m_TaskQueue;

    do
    {
        m_bActive = false;

        if(m_Done)
        {
            break;
        }

        m_bActive = true;

        OpenSP::sp<ParmRectifyThreadPool::ParmRectifyTask> task;
        task_queue->takeFirst(task);

        if(task.valid())
        {
            osg::ref_ptr<ParameterNode> pParameterNode;
            if(!task->m_pParameterNode.lock(pParameterNode))
            {
                OpenThreads::Thread::YieldCurrentThread();
                continue;
            }

            PointParameterNode  *pPointParameterNode    = dynamic_cast<PointParameterNode *>(pParameterNode.get());
            LineParameterNode   *pLineParameterNode     = dynamic_cast<LineParameterNode *>(pParameterNode.get());

            osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();

            //点参数插值
            if(pPointParameterNode != NULL)
            {
                const osg::Vec3d &vPos = pPointParameterNode->getPosition();
                const osg::Vec2d vTempPos(vPos._v[0], vPos._v[1]);
                osg::HeightField *pHF = NULL;
                osg::Vec3d vInterPos;
                m_pThreadPool->computeHeightFiled(vTempPos, pHF, vInterPos);
                if(pHF != NULL)
                {
                    pPointParameterNode->setInterPosition(vInterPos);
                }
            }
            //线参数插值
            else if(pLineParameterNode != NULL)
            {
                const std::vector<osg::ref_ptr<osg::Vec3dArray> > &vecPoints = pLineParameterNode->getPoints();
                bool bMagnet = pLineParameterNode->isMagnet();
                std::vector<osg::ref_ptr<osg::Vec3dArray> > vecInterPoints;
                for(std::vector<osg::ref_ptr<osg::Vec3dArray> >::const_iterator citor = vecPoints.begin(); citor != vecPoints.end(); ++citor)
                {
                    osg::Vec3dArray *pArray = citor->get();
                    osg::ref_ptr<osg::Vec3dArray> pNewArray = new osg::Vec3dArray;
                    //只插值两端顶点
                    if(bMagnet)
                    {
                        for(unsigned int i = 0; i < pArray->size(); i++)
                        {
                            osg::Vec3d vPos = pArray->at(i);
                            const osg::Vec2d vTempPos(vPos._v[0], vPos._v[1]);
                            osg::HeightField *pHF = NULL;
                            osg::Vec3d vInterPos;
                            m_pThreadPool->computeHeightFiled(vTempPos, pHF, vInterPos);
                            if(pHF != NULL)
                            {
                                pNewArray->push_back(vInterPos);
                            }
                        }
                        vecInterPoints.push_back(pNewArray);
                    }
                    //按地形插值
                    else
                    {
                        for(unsigned int nLines = 0; nLines < pArray->size() - 1; nLines++)
                        {
                            osg::Vec3d vPos1 = pArray->at(nLines);
                            osg::Vec3d vPos2 = pArray->at(nLines + 1);

                            if(vPos1 == vPos2)
                            {
                                continue;
                            }

#if 0
                            osg::Vec3d inter_pos;
                            double a, b, c;
                            a = b = c = 0;
                            bool bInvalidK = false;

                            //斜率不存在
                            if(vPos2._v[0] == vPos1._v[0])
                            {
                                //ax + by + c = 0;
                                a = 1;
                                b = 0;
                                c = -vPos1._v[0];
                                bInvalidK = true;
                            }
                            //斜率存在，但要注意斜率为0的情况
                            else
                            {
                                //y = ax + b;
                                a = (vPos2._v[1] - vPos1._v[1]) / (vPos2._v[0] - vPos1._v[0]);
                                b = vPos1._v[1] - a * vPos1._v[0];
                            }

                            osg::Vec2d vNormal(vPos2._v[0] - vPos1._v[0], vPos2._v[1] - vPos1._v[1]);
                            vNormal.normalize();

                            bool bXIncrease = (vPos2._v[0] - vPos1._v[0]) >= 0 ? true : false;
                            bool bYIncrease = (vPos2._v[1] - vPos1._v[1]) >= 0 ? true : false;

                            //线段的范围
                            osg::Vec2d vMin(osg::minimum(vPos1._v[0], vPos2._v[0]), osg::minimum(vPos1._v[1], vPos2._v[1]));
                            osg::Vec2d vMax(osg::maximum(vPos1._v[0], vPos2._v[0]), osg::maximum(vPos1._v[1], vPos2._v[1]));

                            //计算第一个点的插值
                            osg::Vec2d vBegin(vPos1._v[0], vPos1._v[1]);
                            FindTerrainNodeVisitor fnv(vBegin);
                            m_pThreadPool->m_pTerrainNode->accept(fnv);
                            osgTerrain::TerrainTile *pTT = fnv.getTerrainNode();

                            osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
                            osg::HeightField *pHF = pHFLayer->getHeightField();

                            //瓦片的范围
                            double fInterval = osg::DegreesToRadians(pHF->getXInterval());
                            unsigned int nX = pHF->getNumRows();
                            unsigned int nY = pHF->getNumColumns();
                            osg::Vec3d vTileMin = pHF->getOrigin();
                            vTileMin._v[0] = osg::DegreesToRadians(vTileMin._v[0]);
                            vTileMin._v[1] = osg::DegreesToRadians(vTileMin._v[1]);
                            osg::Vec3d vTileMax(vTileMin._v[0] + (nX - 1) * fInterval, vTileMin._v[1] + (nY - 1) * fInterval, vTileMin._v[2]);

                            {
                                osg::Vec3d vvTileMin = pHF->getOrigin();
                                osg::Vec3d vvTileMax(vvTileMin._v[0] + pHF->getXInterval() * (nX - 1),
                                                     vvTileMin._v[1] + pHF->getYInterval() * (nY - 1),
                                                     vvTileMin._v[2]);
                                cmm::Pyramid pyr;
                                ID id = pTT->getID();
                                osg::Vec3d vvTileMin1, vvTileMax1;
                                pyr.GetTilePos(id.TileID.m_nLevel, id.TileID.m_nCol, id.TileID.m_nRow,
                                               vvTileMin1._v[0], vvTileMin1._v[1], vvTileMax1._v[0], vvTileMax1._v[1]);
                                int a = 0;
                            }

                            //瓦片每个网格的大小
                            osg::Vec3d vTempMin, vTempMax;
                            pEllipsoidModel->convertLatLongHeightToXYZ(vTileMin._v[1], vTileMin._v[0], 0, vTempMin._v[0], vTempMin._v[1], vTempMin._v[2]);
                            pEllipsoidModel->convertLatLongHeightToXYZ(vTileMax._v[1], vTileMax._v[0], 0, vTempMax._v[0], vTempMax._v[1], vTempMax._v[2]);
                            vTempMax -= vTempMin;
                            double dblDistance = sqrt(vTempMax.length() / double(nX - 1));

                            const double dblRadio = 10.0;

                            //起点的坐标
                            float fTempX = (vBegin._v[0] - vTileMin._v[0]) / fInterval;
                            float fTempY = (vBegin._v[1] - vTileMin._v[1]) / fInterval;
                            float fTempZ = cmm::image::linearInterpolation(pHF->getHeight(int(fTempX), int(fTempY)),
                                                                           pHF->getHeight(int(fTempX) + 1, int(fTempY)),
                                                                           pHF->getHeight(int(fTempX) + 1, int(fTempY) + 1),
                                                                           pHF->getHeight(int(fTempX), int(fTempY) + 1),
                                                                           fTempX - int(fTempX),
                                                                           fTempY - int(fTempY)) + dblDistance * dblRadio;
                            pNewArray->push_back(osg::Vec3(vBegin._v[0], vBegin._v[1], fTempZ));

                            int nTime = 0;
                            while(pHF != NULL)
                            {
                                nTime++;
                                //瓦片的范围
                                fInterval = osg::DegreesToRadians(pHF->getXInterval());
                                nX = pHF->getNumRows();
                                nY = pHF->getNumColumns();
                                vTileMin = pHF->getOrigin();
                                vTileMin._v[0] = osg::DegreesToRadians(vTileMin._v[0]);
                                vTileMin._v[1] = osg::DegreesToRadians(vTileMin._v[1]);
                                vTileMax.set(vTileMin._v[0] + (nX - 1) * fInterval, vTileMin._v[1] + (nY - 1) * fInterval, vTileMin._v[2]);

                                //瓦片网格的大小
                                pEllipsoidModel->convertLatLongHeightToXYZ(vTileMin._v[1], vTileMin._v[0], 0, vTempMin._v[0], vTempMin._v[1], vTempMin._v[2]);
                                pEllipsoidModel->convertLatLongHeightToXYZ(vTileMax._v[1], vTileMax._v[0], 0, vTempMax._v[0], vTempMax._v[1], vTempMax._v[2]);
                                vTempMax -= vTempMin;
                                dblDistance = sqrt(vTempMax.length() / double(nX - 1));

                                bool bContainEnd = false;
                                //判断此瓦片是否包含末端点
                                if(vPos2._v[0] <= vTileMax._v[0] && vPos2._v[0] >= vTileMin._v[0] && vPos2._v[1] <= vTileMax._v[1] && vPos2._v[1] >= vTileMin._v[1])
                                {
                                    bContainEnd = true;
                                }

                                //std::vector<osg::Vec3d> vecTemp;

                                double x, y, z;

                                //Column
                                if(!bInvalidK)
                                {
                                    for(unsigned int i = 0; i < nX; i++)
                                    {
                                        x = vTileMin._v[0] + i * fInterval;
                                        y = a * x + b;

                                        if(y < vMin._v[1] || y > vMax._v[1])
                                        {
                                            continue;
                                        }

                                        if(y < vTileMin._v[1] || y > vTileMax._v[1])
                                        {
                                            continue;
                                        }

                                        if(y == vTileMin._v[1] || y == vTileMax._v[1])
                                        {
                                            int xx = 0;
                                        }

                                        //是否越界
                                        float fTemp1 = (y - vTileMin._v[1]) / fInterval;

                                        if(fTemp1 >= nY - 1 || fTemp1 < 0)
                                        {
                                            continue;
                                        }

                                        //z = pHF->getVertex(i, (int)fTemp1)._v[2] * (1 + (int)fTemp1- fTemp1) + 
                                        //    pHF->getVertex(i, (int)fTemp1 + 1)._v[2] * (fTemp1 - (int)fTemp1) + 
                                        //    dblDistance * dblRadio;

                                        z = pHF->getHeight(i, (int)fTemp1) * (1 + (int)fTemp1- fTemp1) + 
                                            pHF->getHeight(i, (int)fTemp1 + 1) * (fTemp1 - (int)fTemp1) + 
                                            dblDistance * dblRadio;

                                        pNewArray->push_back(osg::Vec3(x, y, z));
                                        //vecTemp.push_back(osg::Vec3(x, y, z));
                                    }
                                }

                                //Row
                                if(a != 0)
                                {
                                    for(unsigned int i = 0; i < nY; i++)
                                    {
                                        y = vTileMin._v[1] + i * fInterval;
                                        if(bInvalidK)
                                        {
                                            x = c;
                                        }
                                        else
                                        {
                                            x = (y - b) / a;
                                        }
                                        if(x < vMin._v[0] || x > vMax._v[0])
                                        {
                                            continue;
                                        }

                                        if(x == vTileMin._v[0] || x == vTileMax._v[0])
                                        {
                                            int xx = 0;
                                        }

                                        //是否越界
                                        float fTemp1 = (x - vTileMin._v[0]) / fInterval;

                                        if(fTemp1 >= nX - 1 || fTemp1 < 0)
                                        {
                                            continue;
                                        }

                                        //z = pHF->getVertex((int)fTemp1, i)._v[2] * (1 + (int)fTemp1 - fTemp1) + 
                                        //    pHF->getVertex((int)fTemp1 + 1, i)._v[2] * (fTemp1 - (int)fTemp1) + 
                                        //    dblDistance * dblRadio;

                                        z = pHF->getHeight((int)fTemp1, i) * (1 + (int)fTemp1- fTemp1) + 
                                            pHF->getHeight((int)fTemp1 + 1, i) * (fTemp1 - (int)fTemp1) + 
                                            dblDistance * dblRadio;

                                        pNewArray->push_back(osg::Vec3(x, y, z));
                                        //vecTemp.push_back(osg::Vec3(x, y, z));
                                    }
                                }
#if _DEBUG
                                if(pNewArray->empty())
                                {
                                    printf("Times = %d, Size = %d\n", nTime, pNewArray->size());
                                }
                                //if(vecTemp.empty())
                                //{
                                //    printf("Size = %d\n", vecTemp.size());
                                //}
#endif

                                //std::vector<osg::Vec3d>::iterator itorEnd = std::unique(vecTemp.begin(), vecTemp.end());
                                //vecTemp.erase(itorEnd, vecTemp.end());
                                //std::sort(vecTemp.begin(), vecTemp.end(), SortFunctor(bXIncrease, bYIncrease));
                                //pNewArray->insert(pNewArray->end(), vecTemp.begin(), vecTemp.end());
                                std::sort(pNewArray->begin(), pNewArray->end(), SortFunctor(bXIncrease, bYIncrease));

                                if(bContainEnd)
                                {
                                    vBegin.set(vPos2._v[0], vPos2._v[1]);
                                    float fTempX = (vBegin._v[0] - vTileMin._v[0]) / fInterval;
                                    float fTempY = (vBegin._v[1] - vTileMin._v[1]) / fInterval;
                                    float fTempZ = cmm::image::linearInterpolation(pHF->getHeight(int(fTempX), int(fTempY)),
                                                                                   pHF->getHeight(int(fTempX) + 1, int(fTempY)),
                                                                                   pHF->getHeight(int(fTempX) + 1, int(fTempY) + 1),
                                                                                   pHF->getHeight(int(fTempX), int(fTempY) + 1),
                                                                                   fTempX - int(fTempX),
                                                                                   fTempY - int(fTempY)) + dblDistance * dblRadio;
                                    pNewArray->push_back(osg::Vec3(vPos2._v[0], vPos2._v[1], fTempZ));
                                    vecInterPoints.push_back(pNewArray);

                                    pHF = NULL;
                                }
                                else
                                {
                                    osg::Vec3d vLastPos = pNewArray->at(pNewArray->size() - 1);
                                    vBegin.set(vLastPos._v[0], vLastPos._v[1]);
                                    vecInterPoints.push_back(pNewArray);
                                    pNewArray = new osg::Vec3dArray;

                                    //vBegin.set(vecTemp[vecTemp.size() - 1]._v[0], vecTemp[vecTemp.size() - 1]._v[1]);
                                    //vBegin += (vNormal * fInterval * (pHF->getNumColumns() / 4));
                                    vBegin += vNormal * fInterval;
                                    fnv.setPosition(vBegin);
                                    m_pThreadPool->m_pTerrainNode->accept(fnv);

                                    osgTerrain::TerrainTile *pTT = fnv.getTerrainNode();
                                    osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
                                    osg::HeightField *pTempHF = pHFLayer->getHeightField();
                                    if(pHF == pTempHF)
                                    {
                                        printf("==========>Error<==========\n");
                                    }
                                    pHF = pTempHF;
                                }
                            }
#else
                            //线段方向
                            osg::Vec2d vNormal(vPos2._v[0] - vPos1._v[0], vPos2._v[1] - vPos1._v[1]);
                            double dblLen1 = vNormal.length2();
                            vNormal.normalize();

                            //计算第一个点的插值
                            osg::Vec2d vBegin(vPos1._v[0], vPos1._v[1]);
                            FindTerrainNodeVisitor fnv(vBegin);
                            m_pThreadPool->m_pTerrainNode->accept(fnv);
                            osgTerrain::TerrainTile *pTT = fnv.getTerrainNode();
                            //printf("==========>Terrain Level == %d<==========\n", pTT->getID().TileID.m_nLevel);

                            osgTerrain::HeightFieldLayer *pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
                            osg::HeightField *pHF = pHFLayer->getHeightField();
                            //if(pHF == NULL)
                            //{
                            //    printf("==========>1:pHF == NULL<==========\n");
                            //}

                            //瓦片的范围
                            double fInterval = osg::DegreesToRadians(pHF->getXInterval());
                            unsigned int nX = pHF->getNumRows();
                            unsigned int nY = pHF->getNumColumns();
                            osg::Vec3d vTileMin = pHF->getOrigin();
                            vTileMin._v[0] = osg::DegreesToRadians(vTileMin._v[0]);
                            vTileMin._v[1] = osg::DegreesToRadians(vTileMin._v[1]);
                            osg::Vec3d vTileMax(vTileMin._v[0] + (nX - 1) * fInterval, vTileMin._v[1] + (nY - 1) * fInterval, vTileMin._v[2]);

                            //瓦片每个网格的大小
                            osg::Vec3d vTempMin, vTempMax;
                            pEllipsoidModel->convertLatLongHeightToXYZ(vTileMin._v[1], vTileMin._v[0], 0, vTempMin._v[0], vTempMin._v[1], vTempMin._v[2]);
                            pEllipsoidModel->convertLatLongHeightToXYZ(vTileMax._v[1], vTileMax._v[0], 0, vTempMax._v[0], vTempMax._v[1], vTempMax._v[2]);
                            vTempMax -= vTempMin;
                            double dblDistance = sqrt(vTempMax.length() / double(nX - 1));

                            const double dblRadio = 10.0;

                            //起点的坐标
                            float fTempX = (vBegin._v[0] - vTileMin._v[0]) / fInterval;
                            float fTempY = (vBegin._v[1] - vTileMin._v[1]) / fInterval;
                            float fTempZ = cmm::image::linearInterpolation(pHF->getHeight(int(fTempX),      int(fTempY)),
                                                                           pHF->getHeight(int(fTempX) + 1,  int(fTempY)),
                                                                           pHF->getHeight(int(fTempX) + 1,  int(fTempY) + 1),
                                                                           pHF->getHeight(int(fTempX),      int(fTempY) + 1),
                                                                           fTempX - int(fTempX),
                                                                           fTempY - int(fTempY)) + dblDistance * dblRadio;

                            pNewArray->push_back(osg::Vec3(vBegin._v[0], vBegin._v[1], fTempZ));

                            while(true)
                            {
                                vBegin += vNormal * fInterval;
                                osg::Vec2d vTemp(vBegin._v[0] - vPos1._v[0], vBegin._v[1] - vPos1._v[1]);
                                double dblLen2 = vTemp.length2();

                                //计算终点
                                if(dblLen2 > dblLen1)
                                {
                                    vBegin.set(vPos2._v[0], vPos2._v[1]);
                                }

                                if(vBegin._v[0] < vTileMin._v[0] || vBegin._v[0] > vTileMax._v[0] || vBegin._v[1] < vTileMin._v[1] || vBegin._v[1] > vTileMax._v[1])
                                {
                                    vecInterPoints.push_back(pNewArray);
                                    osg::Vec3d vTemp = pNewArray->at(pNewArray->size() - 1);
                                    pNewArray = new osg::Vec3dArray;
                                    pNewArray->push_back(vTemp);

                                    fnv.setPosition(vBegin);
                                    m_pThreadPool->m_pTerrainNode->accept(fnv);
                                    osgTerrain::TerrainTile *pTT = fnv.getTerrainNode();
                                    //printf("==========>Terrain Level == %d<==========\n", pTT->getID().TileID.m_nLevel);

                                    pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
                                    pHF = pHFLayer->getHeightField();
                                    //if(pHF == NULL)
                                    //{
                                    //    printf("==========>2:pHF == NULL<==========\n");
                                    //}

                                    //瓦片的范围
                                    fInterval = osg::DegreesToRadians(pHF->getXInterval());
                                    nX = pHF->getNumRows();
                                    nY = pHF->getNumColumns();
                                    vTileMin = pHF->getOrigin();
                                    vTileMin._v[0] = osg::DegreesToRadians(vTileMin._v[0]);
                                    vTileMin._v[1] = osg::DegreesToRadians(vTileMin._v[1]);
                                    vTileMax.set(vTileMin._v[0] + (nX - 1) * fInterval, vTileMin._v[1] + (nY - 1) * fInterval, vTileMin._v[2]);

                                    //瓦片每个网格的大小
                                    pEllipsoidModel->convertLatLongHeightToXYZ(vTileMin._v[1], vTileMin._v[0], 0, vTempMin._v[0], vTempMin._v[1], vTempMin._v[2]);
                                    pEllipsoidModel->convertLatLongHeightToXYZ(vTileMax._v[1], vTileMax._v[0], 0, vTempMax._v[0], vTempMax._v[1], vTempMax._v[2]);
                                    vTempMax -= vTempMin;
                                    dblDistance = sqrt(vTempMax.length() / double(nX - 1));
                                }

                                fTempX = (vBegin._v[0] - vTileMin._v[0]) / fInterval;
                                fTempY = (vBegin._v[1] - vTileMin._v[1]) / fInterval;
                                fTempZ = cmm::image::linearInterpolation(pHF->getHeight(int(fTempX),      int(fTempY)),
                                                                         pHF->getHeight(int(fTempX) + 1,  int(fTempY)),
                                                                         pHF->getHeight(int(fTempX) + 1,  int(fTempY) + 1),
                                                                         pHF->getHeight(int(fTempX),      int(fTempY) + 1),
                                                                         fTempX - int(fTempX),
                                                                         fTempY - int(fTempY)) + dblDistance * dblRadio;

                                pNewArray->push_back(osg::Vec3(vBegin._v[0], vBegin._v[1], fTempZ));

                                if(dblLen2 > dblLen1)
                                {
                                    break;
                                }
                            }
#endif
                        }
                    }
                }
                //pLineParameterNode->setInterPositions(vecInterPoints);
                osg::ref_ptr<osg::Node> pNode = pLineParameterNode->createNodeByParameter(vecInterPoints);
                pLineParameterNode->setRectifiedNode(pNode.get());
            }
            else
            {
                OpenThreads::Thread::YieldCurrentThread();
            }
        }
        else
        {
            OpenThreads::Thread::YieldCurrentThread();
        }

    }while(!m_Done);
}

ParmRectifyThreadPool::ParmRectifyThreadPool(void)
{
    m_bStartThreadCalled = false;
    setUpThreads(Registry::instance()->getParmRectifyThreadCount());
    m_TaskQueue = new ParmRectifyTaskQueue;
}

ParmRectifyThreadPool::~ParmRectifyThreadPool(void)
{
    cancel();

    m_vecParamThreadList.clear();

    m_TaskQueue = NULL;
}

int ParmRectifyThreadPool::cancel()
{
    for(std::vector<osg::ref_ptr<ParmRectifyThread> >::iterator itor = m_vecParamThreadList.begin(); itor != m_vecParamThreadList.end();++itor)
    {
        (*itor)->setDone(true);
    }

    for(std::vector<osg::ref_ptr<ParmRectifyThread> >::iterator itor = m_vecParamThreadList.begin(); itor != m_vecParamThreadList.end();++itor)
    {
        (*itor)->cancel();
    }

    return 0;
}

bool ParmRectifyThreadPool::isRunning() const
{
    for(std::vector<osg::ref_ptr<ParmRectifyThread> >::const_iterator citor = m_vecParamThreadList.begin(); citor != m_vecParamThreadList.end(); ++citor)
    {
        if ((*citor)->isRunning())
        {
            return true;
        }
    }

    return false;
}

void ParmRectifyThreadPool::setUpThreads(unsigned int nTotalNumThreads)
{
    m_vecParamThreadList.clear();

    for(unsigned int i = 0; i < nTotalNumThreads; ++i)
    {
        addParmRectifyThread();
    }
}

unsigned int ParmRectifyThreadPool::addParmRectifyThread()
{
    unsigned int pos = m_vecParamThreadList.size();

    ParmRectifyThreadPool::ParmRectifyThread *pThread = new ParmRectifyThreadPool::ParmRectifyThread(this);
    m_vecParamThreadList.push_back(pThread);

    if(m_bStartThreadCalled)
    {
        pThread->startThread();
    }

    return pos;
}

void ParmRectifyThreadPool::computeHeightFiled(const osg::Vec2d &pos, osg::HeightField *&pHeightField, osg::Vec3d &inter_pos)
{
    if(!m_pTerrainNode.valid())
    {
        return;
    }

    osg::Vec3d vTemp1, vTemp2;
    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ben, end;
    pEllipsoidModel->convertLatLongHeightToXYZ(pos._v[1], pos._v[0], 10000, ben._v[0], ben._v[1], ben._v[2]);

    osgUtil::LineSegmentIntersector *pTerrainPicker = new osgUtil::LineSegmentIntersector(ben, end);
    osgUtil::IntersectionVisitor iv(pTerrainPicker);

    m_pTerrainNode->accept(iv);

    if(!pTerrainPicker->containsIntersections())
    {
        return;
    }

    osgTerrain::HeightFieldLayer *pHFLayer = NULL;

    osgUtil::LineSegmentIntersector::Intersection inter = pTerrainPicker->getFirstIntersection();
    osg::NodePath::const_reverse_iterator ritor = inter.nodePath.crbegin();
    for(; ritor != inter.nodePath.crend(); ++ritor)
    {
        osg::Node *pNode = (*ritor);
        osgTerrain::TerrainTile *pTT = dynamic_cast<osgTerrain::TerrainTile *>(pNode);
        if(pTT == NULL)
        {
            continue;
        }

        osgTerrain::TileID tile_ID = pTT->getTileID();
        pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
        if(pHFLayer == NULL)
        {
            continue;
        }
        break;
    }
    if(pHFLayer == NULL)
    {
        return;
    }

    pHeightField = pHFLayer->getHeightField();
    osg::Vec3d vPos = inter.getWorldIntersectPoint();
    pEllipsoidModel->convertXYZToLatLongHeight(vPos._v[0], vPos._v[1], vPos._v[2], inter_pos._v[1], inter_pos._v[0], inter_pos._v[2]);

    vTemp1 =  ben;
    vTemp1.normalize();

    vTemp2 = vPos;
    vTemp2.normalize();

    inter_pos._v[0] = pos._v[0];
    inter_pos._v[1] = pos._v[1];
    if(inter_pos._v[2] < 0)
    {
        inter_pos._v[2] = 0;
    }
}

void ParmRectifyThreadPool::computeHeightFiled(const osg::Vec2d &pos, osg::HeightField *&pHeightField, osg::Vec3d &inter_pos, double &dblDistance)
 {
    computeHeightFiled(pos, pHeightField, inter_pos);
    assert(pHeightField != NULL);

    float fInterval = osg::DegreesToRadians(pHeightField->getXInterval());
    unsigned int nX = pHeightField->getNumRows();
    unsigned int nY = pHeightField->getNumColumns();
    osg::Vec3d vTileMin = pHeightField->getOrigin();
    vTileMin._v[0] = osg::DegreesToRadians(vTileMin._v[0]);
    vTileMin._v[1] = osg::DegreesToRadians(vTileMin._v[1]);
    osg::Vec3d vTileMax(vTileMin._v[0] + (nX - 1) * fInterval, vTileMin._v[1] + (nY - 1) * fInterval, vTileMin._v[2]);
    osg::Vec3d vMin, vMax;

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    pEllipsoidModel->convertLatLongHeightToXYZ(vTileMin._v[1], vTileMin._v[0], 0, vMin._v[0], vMin._v[1], vMin._v[2]);
    pEllipsoidModel->convertLatLongHeightToXYZ(vTileMax._v[1], vTileMax._v[0], 0, vMax._v[0], vMax._v[1], vMax._v[2]);

    vMax -= vMin;
    dblDistance = sqrt(vMax.length() / double(nX - 1));
    inter_pos._v[2] += dblDistance * 10;
}

void ParmRectifyThreadPool::requestParmRectify(ParameterNode * pParameterNode, OpenSP::sp<OpenSP::Ref>& ParmRectifyRequestRef)
{
#if 0
    bool foundEntry = false;

    if(ParmRectifyRequestRef.valid())
    {
        ParmRectifyTask *pParmRectifyTask = dynamic_cast<ParmRectifyTask *>(ParmRectifyRequestRef.get());
        bool requeue = false;
        if(pParmRectifyTask != NULL)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> Lock(m_RequestMutex);

            pParmRectifyTask->m_pParameterNode = pParameterNode;

            foundEntry = true;

            if (ParmRectifyRequestRef->referenceCount() == 1)
            {
                requeue = true;
            }
        }
        if(requeue)
        {
            m_TaskQueue->addTask(pParmRectifyTask);
        }
    }

    if (!foundEntry)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_TaskQueue->m_TaskMutex);

        if(!ParmRectifyRequestRef.valid() || ParmRectifyRequestRef->referenceCount()==1)
        {
            OpenSP::sp<ParmRectifyTask> pParmRectifyTask = new ParmRectifyTask;

            ParmRectifyRequestRef = pParmRectifyTask.get();

            pParmRectifyTask->m_pParameterNode = pParameterNode;

            m_TaskQueue->addTaskNoLock(pParmRectifyTask.get());
        }
    }
#else
    if(!ParmRectifyRequestRef.valid())
    {
        OpenSP::sp<ParmRectifyTask> pParmRectifyTask = new ParmRectifyTask;

        ParmRectifyRequestRef = pParmRectifyTask.get();

        pParmRectifyTask->m_pParameterNode = pParameterNode;

        m_TaskQueue->addTask(pParmRectifyTask);
    }
#endif

    if(!m_bStartThreadCalled)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_RunMutex);

        if(!m_bStartThreadCalled)
        {
            m_bStartThreadCalled = true;

            if(m_vecParamThreadList.empty()) 
            {
                setUpThreads(Registry::instance()->getParmRectifyThreadCount());
            }

            for(std::vector<osg::ref_ptr<ParmRectifyThread> >::const_iterator citor = m_vecParamThreadList.begin(); citor != m_vecParamThreadList.end(); ++citor)
            {
                (*citor)->startThread();
            }
        }
    }
}

void ParmRectifyThreadPool::computeHeightFiled1(const osg::Vec2d &pos, osg::HeightField *&pHeightField, osg::Vec3d &inter_pos)
{
    if(!m_pTerrainNode.valid())
    {
        return;
    }

    osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
    osg::Vec3d ben, end;
    pEllipsoidModel->convertLatLongHeightToXYZ(pos._v[1], pos._v[0], 10000, ben._v[0], ben._v[1], ben._v[2]);

    osgUtil::LineSegmentIntersector *pTerrainPicker = new osgUtil::LineSegmentIntersector(ben, end);
    osgUtil::IntersectionVisitor iv(pTerrainPicker);

    m_pTerrainNode->accept(iv);

    if(!pTerrainPicker->containsIntersections())
    {
        return;
    }

    osgTerrain::HeightFieldLayer *pHFLayer = NULL;

    osgUtil::LineSegmentIntersector::Intersection inter = pTerrainPicker->getFirstIntersection();
    osg::NodePath::const_reverse_iterator ritor = inter.nodePath.crbegin();
    for(; ritor != inter.nodePath.crend(); ++ritor)
    {
        osg::Node *pNode = (*ritor);
        osgTerrain::TerrainTile *pTT = dynamic_cast<osgTerrain::TerrainTile *>(pNode);
        if(pTT == NULL)
        {
            continue;
        }

        osgTerrain::TileID tile_ID = pTT->getTileID();
        pHFLayer = dynamic_cast<osgTerrain::HeightFieldLayer *>(pTT->getElevationLayer());
        if(pHFLayer == NULL)
        {
            continue;
        }
        break;
    }
    if(pHFLayer == NULL)
    {
        return;
    }

    ben.normalize();

    pHeightField = pHFLayer->getHeightField();
    inter_pos = inter.getWorldIntersectPoint();
    float fInterval = osg::DegreesToRadians(pHeightField->getXInterval());

    unsigned int nX = pHeightField->getNumRows();
    unsigned int nY = pHeightField->getNumColumns();
    osg::Vec3d vTileMin = pHeightField->getOrigin();
    vTileMin._v[0] = osg::DegreesToRadians(vTileMin._v[0]);
    vTileMin._v[1] = osg::DegreesToRadians(vTileMin._v[1]);
    osg::Vec3d vTileMax(vTileMin._v[0] + (nX - 1) * fInterval, vTileMin._v[1] + (nY - 1) * fInterval, vTileMin._v[2]);
    osg::Vec3d vMin, vMax;

    pEllipsoidModel->convertLatLongHeightToXYZ(vTileMin._v[1], vTileMin._v[0], 0, vMin._v[0], vMin._v[1], vMin._v[2]);
    pEllipsoidModel->convertLatLongHeightToXYZ(vTileMax._v[1], vTileMax._v[0], 0, vMax._v[0], vMax._v[1], vMax._v[2]);

    vMax -= vMin;
    double dblDistance = sqrt(vMax.length() / double(nX - 1));
    inter_pos += (ben * dblDistance * 10);
}