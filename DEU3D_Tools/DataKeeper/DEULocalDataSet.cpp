#include "DEULocalDataSet.h"
#include <Network/IDEUNetwork.h>
#include "DEUDataLog.h"
#include "DEUUrlServer.h"
#include <Common/DEUBson.h>
#include <VirtualTileManager/IVirtualTileManager.h>
#include <VirtualTileManager/IVirtualCubeManager.h>
#include <direct.h>
#include <LogicalManager/ILayerManager.h>
#include <LogicalManager/ILayer.h>
#include <LogicalManager/IPropertyManager.h>
#include <IDProvider/Definer.h>
#include <common/deuMath.h>
#include "DEUDefine.h"

#define DBOPEN_ERROR    "�������ݴ�ʧ��"
#define NOTDEM_EERROR   "���ǵ�������"
#define NOTDOM_EERROR   "����Ӱ������"

std::string strLogPath;
std::string strLogRoot;
std::string strLogCfg;

DEULocalData::DEULocalData(ID* pId)
: m_pEventFeature(pId)
, m_bIsFinish(false)
, m_bIsErrFlag(false)
, m_bBatch(false)
, m_pProxy(NULL)
{
    m_strDBPath = "";
}

DEULocalData::~DEULocalData()
{
    if(m_pProxy != NULL)
    {
        m_pProxy->closeDB();
        delete m_pProxy;
        m_pProxy = NULL;
    }
}   

//�������������أ�Ҫ�ϴ����Ѿ���װ�õ�����
inline void GetPostPack(const void*BlockBuf, unsigned nLen, bson::bsonStream& bStreamOutput)
{
    if (BlockBuf == NULL)
    {
        return;
    }

    bson::bsonStream bStreamInput;
    bStreamInput.Write(BlockBuf, nLen);
    bStreamInput.Reset();

    bson::bsonDocument bTmpDoc;
    bTmpDoc.AddBinElement("Data", bStreamInput.Data(), bStreamInput.DataLen());
    bTmpDoc.Write(&bStreamOutput);
}


inline void GetPostPack(bson::bsonDocument& bDoc, bson::bsonStream& bStreamOutput)
{
    bson::bsonStream bStreamInput;
    bDoc.Write(&bStreamInput);
    
    bson::bsonDocument bTmpDoc;
    bTmpDoc.AddBinElement("Data", bStreamInput.Data(), bStreamInput.DataLen());
    bTmpDoc.Write(&bStreamOutput);
}

void DEULocalData::OutputLog(const ID* pID, LOGPACKAGE& _LOGPACKAGE, ea::IEventAdapter *pEventAdapter)
{
    DEUDataLog log;
    log.initPath(strLogCfg.c_str(), strLogRoot.c_str(), strLogPath.c_str());

    std::string strDesc = _LOGPACKAGE.id.toString();
    m_pLogInfo->setCommitFaildID.insert(strDesc);

    if (_LOGPACKAGE.vecErr.size() > 0)
    {
        for(unsigned n=0; n<_LOGPACKAGE.vecErr.size(); n++)
        {
            strDesc += " �ύ ";
            strDesc += _LOGPACKAGE.vecErr[n].c_str();
            strDesc += "���ɹ�";
            strDesc += " ԭ��";
            strDesc += _LOGPACKAGE.strErr.c_str();
            CDeuCallEvent::CallEventState(n, strDesc.c_str(), pID, pEventAdapter);
            m_pLogInfo->setLogInfo.insert(strDesc);

            //������־
            std::string strHost = _LOGPACKAGE.vecErr[n];

            DEUDataLogInfo info;
            info.m_nOperateType = _LOGPACKAGE.nOperateType;
            info.m_nErrorType   = _LOGPACKAGE.nError;
            info.m_strDBPath    = _LOGPACKAGE.m_strDBPath;
            info.m_strID        = _LOGPACKAGE.id.toString().c_str();
            info.m_strIP        = strHost.substr(0, strHost.find(":"));
            info.m_strPort      = strHost.substr(strHost.find(":")+1);
            info.m_strErrorDesp = _LOGPACKAGE.strErr.c_str();
            log.AddDataLog(info);
        }
    }
    else
    {
        strDesc += " �ύ���ɹ�ԭ�� ";
        strDesc += _LOGPACKAGE.strErr.c_str();
        CDeuCallEvent::CallEventState(0, strDesc.c_str(), pID, pEventAdapter);
        m_pLogInfo->setLogInfo.insert(strDesc);
    }

    return;
}

void DEULocalData::setDEUNetWork(OpenSP::sp<deunw::IDEUNetwork> pIUrlNetwork)
{
    m_pIUrlNetwork = pIUrlNetwork;
}

OpenSP::sp<deunw::IDEUNetwork> DEULocalData::getDEUNetWork()
{
    return m_pIUrlNetwork;
}

bool DEULocalData::IsStop(unsigned int nIndex, std::string& TaskName, ea::IEventAdapter *pEventAdapter)
{
    if(m_bIsFinish)
    {
        std::string strTmp = TaskName;
        //strTmp += " �ύ����ֹ.";
        strTmp += " ȡ���ύ";
        CDeuCallEvent::CallEventState(nIndex, strTmp.c_str(), m_pEventFeature, pEventAdapter);
    }

    return m_bIsFinish;
}

void DEULocalData::setLogInfo(OpenSP::sp<LogInfo> pLogInfo)
{
    m_pLogInfo = pLogInfo;
}

//- �����Ƿ�Ϊ�����ϴ�
void DEULocalData::setBatchUpload(bool bBatch)
{
    m_bBatch = bBatch;
}

DEULocalTile::DEULocalTile(ID* pId)
: DEULocalData(pId)
{
}


DEULocalTile::~DEULocalTile(void)
{
}

// ��ʼ��
bool DEULocalTile::initialize(const std::string& strDBPath)
{
    m_strDBPath = strDBPath;
    m_pProxy    = deudbProxy::createDEUDBProxy();
    if(m_pProxy == NULL)
    {
        return false;
    }

    if(!m_pProxy->openDB(m_strDBPath))
    {
        delete m_pProxy;
        m_pProxy = NULL;

        return false;
    }

    //����Ԥ���أ����ݼ��
    std::vector<ID> vecID = m_pProxy->getAllIndices();
    unsigned nCount = vecID.size();

    if(nCount == 0) return false;

    //�ж��ύ����ʲô��������
    if(vecID[0].TileID.m_nType == TERRAIN_TILE_HEIGHT_FIELD ||
       vecID[0].TileID.m_nType == TERRAIN_DEM_ID ||
       vecID[0].TileID.m_nType == TERRAIN_DEM_LAYER_ID) //����
    {
        m_DATATYPE = DEM;
    }
    else if(vecID[0].TileID.m_nType == TERRAIN_TILE_IMAGE ||
            vecID[0].TileID.m_nType == TERRAIN_DOM_ID ||
            vecID[0].TileID.m_nType == TERRAIN_DOM_LAYER_ID) //Ӱ��
    {
        m_DATATYPE = DOM;
    }

    m_setIDCollection.clear();
    m_setIDCollection.insert(vecID.cbegin(), vecID.cend());

    return true;
}

/*
 �ύ������/Ӱ����Ƭ����ҵ���߼�������
     1. �ύ���ݿ鵽�������ϣ�
     2. �ҽ����ݼ�������Ϣ������ͼ���ϣ�����ע��������ж���������ֱ�Ӻϲ��ύ�����򣬴���һ���ύ��
    3. ����ύ�����ļ����������ϵ����ݼ��У�
*/
unsigned DEULocalTile::submit(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect)
{
    if(m_setIDCollection.size() == 0) return 0;

    unsigned nRec=0;
    unsigned nProccesor=0;
    unsigned nTaskCount=m_setIDCollection.size();

    bool isDelAction=false;
    void* BlockBuf = NULL;
    unsigned nLen, nIndex=0;
    std::vector<std::string> vecErr;
    OpenSP::sp<cmm::IDEUException> pOutExcep = cmm::createDEUException();

    std::vector<ID> vecIDs;

// 1>  �ύ���ݿ鵽��������
// 1>  BeginRegion
    CDeuCallEvent::CallEventState(0, "��ʼ���е���/Ӱ�������ύ...", m_pEventFeature, pEventAdapter);
    m_pLogInfo->sCurCommitFile = m_strDBPath;
    m_pLogInfo->nTotalIDCount = m_setIDCollection.size();

    if(IDCollect.size() == 0)
    {
        for(std::set<ID>::iterator it = m_setIDCollection.begin(); it != m_setIDCollection.end(); ++it)
        {
            if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

            if(m_pProxy->readBlock(*it, BlockBuf, nLen))
            {
                if(nLen == 0) 
                {
                    std::string strtmp = (*it).toString();
                    strtmp += " ��Ч������";
                    CDeuCallEvent::CallEventState(nIndex, strtmp.c_str(), m_pEventFeature, pEventAdapter);
                    nIndex++;
                    nRec++;
                    continue;
                }

                if(!m_pIUrlNetwork->replaceData(*it, BlockBuf, nLen, vecErr, pOutExcep))
                {
                    //��־���
                    LOGPACKAGE _LOGPACKAGE;
                    _LOGPACKAGE.id = *it;
                    _LOGPACKAGE.m_strDBPath = m_strDBPath;
                    _LOGPACKAGE.nError = pOutExcep->getReturnCode();
                    _LOGPACKAGE.strErr = pOutExcep->getMessage();
                    _LOGPACKAGE.nOperateType = DEU_ADD_DATA;
                    _LOGPACKAGE.vecErr.assign(vecErr.begin(), vecErr.end());
                    OutputLog(m_pEventFeature, _LOGPACKAGE, pEventAdapter);
                    if (m_bBatch)
                    {
                        m_bIsErrFlag = true;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                if ((*it).TileID.m_nType == TERRAIN_DOM_ID || (*it).TileID.m_nType == TERRAIN_DEM_ID)
                {
                    //��ȡ�½�ͼ��������ʵ����������
                    std::string strName = "";
                    OpenSP::sp<logical::IPropertyManager> pIPropertyManager = NULL;
                    OpenSP::sp<deunw::IDEUNetwork>        pIDEUNetwork      = NULL;
                    OpenSP::sp<logical::IProperty>        pIProperty        = NULL;

                    pIPropertyManager = logical::createPropertyManager();
                    pIProperty = pIPropertyManager->createPropertyByBsonStream(BlockBuf, nLen);
                    if (pIProperty != NULL)
                    {
                        OpenSP::sp<logical::IProperty> pIPropertyName = pIProperty->findProperty("Name", true);
                        if (pIPropertyName != NULL)
                        {
                            strName = pIPropertyName->getValueAsString();
                        }
                    }

                    //��������/Ӱ��ͼ�㣬�����ʵ��
                    ID rootLayerID;
                    ID subLayerID = ID::genNewID();
                    subLayerID.ObjectID.m_nDataSetCode = 6;
                    if (m_DATATYPE == DOM)
                    {
                        rootLayerID = ID::getTerrainDOMLayerRootID();
                        subLayerID.ObjectID.m_nType = TERRAIN_DOM_LAYER_ID;
                    }
                    else
                    {
                        rootLayerID = ID::getTerrainDEMLayerRootID();
                        subLayerID.ObjectID.m_nType = TERRAIN_DEM_LAYER_ID;
                    }

                    bson::bsonDocument bsonDoc;
                    bsonDoc.AddStringElement("ID", subLayerID.toString().c_str());
                    bsonDoc.AddStringElement("Name", strName.c_str());

                    bson::bsonArrayEle* bsonBoundArr = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("BoundingSphere"));
                    double dBoundingSphere[4] = {0.0, 0.0, 0.0, 0.0};
                    if (pIProperty != NULL)
                    {
                        OpenSP::sp<logical::IProperty> pIPropertyBoundingSphere = pIProperty->findProperty("BoundingSphere", true);
                        if (pIPropertyBoundingSphere != NULL)
                        {
                            OpenSP::sp<logical::IProperty> pIPropertyBoundingSphere0 = pIPropertyBoundingSphere->getChild(0);
                            OpenSP::sp<logical::IProperty> pIPropertyBoundingSphere1 = pIPropertyBoundingSphere->getChild(1);
                            OpenSP::sp<logical::IProperty> pIPropertyBoundingSphere2 = pIPropertyBoundingSphere->getChild(2);
                            OpenSP::sp<logical::IProperty> pIPropertyBoundingSphere3 = pIPropertyBoundingSphere->getChild(3);
                            dBoundingSphere[0] = (double)pIPropertyBoundingSphere0->getValue();
                            dBoundingSphere[1] = (double)pIPropertyBoundingSphere1->getValue();
                            dBoundingSphere[2] = (double)pIPropertyBoundingSphere2->getValue();
                            dBoundingSphere[3] = (double)pIPropertyBoundingSphere3->getValue();
                        }
                    }
                    bsonBoundArr->AddDblElement(dBoundingSphere[0]);
                    bsonBoundArr->AddDblElement(dBoundingSphere[1]);
                    bsonBoundArr->AddDblElement(dBoundingSphere[2]);
                    bsonBoundArr->AddDblElement(dBoundingSphere[3]);

                    bson::bsonArrayEle* bsonChildIDArr  = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("ChildrenID"));
                    bsonChildIDArr->AddStringElement((*it).toString().c_str());
                    bson::bsonArrayEle* bsonParentIDArr = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("ParentID"));
                    bsonParentIDArr->AddStringElement(rootLayerID.toString().c_str());

                    bson::bsonStream bsonStream;
                    bsonDoc.Write(&bsonStream);
                    void*    pBuffer = bsonStream.Data();
                    unsigned nBufLen = bsonStream.DataLen();

#ifdef _DEBUG
                    std::string strJsonString;
                    bsonDoc.JsonString(strJsonString);
#endif
                    if (!m_pIUrlNetwork->addLayer(subLayerID, rootLayerID, pBuffer, nBufLen, vecErr, pOutExcep))
                    {
                        CDeuCallEvent::CallEventState(nIndex, "�ύͼ��ʧ��...", m_pEventFeature, pEventAdapter);
                    }
                }

                if(BlockBuf)
                {
                    deudbProxy::freeMemory(BlockBuf);
                    BlockBuf = NULL;
                }
            }

            nIndex++;

            if(nRec < 999)
            {
                nRec++;
            }
            else
            {
                nProccesor += nRec;
                char szDesc[50]={0};
                sprintf(szDesc, "����� %.2f%%", ((double)nProccesor / (double)nTaskCount)*100);
                CDeuCallEvent::CallEventState(nIndex, szDesc, m_pEventFeature, pEventAdapter);
                nRec = 0;
            }
        }
        
    }
    else
    {
        // �ֲ����ݵ��ύ
        for(unsigned int m=0; m<IDCollect.size(); m++)
        {
            if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

            std::set<ID>::iterator it = m_setIDCollection.find(IDCollect[m]);
            
            if(m_pProxy->readBlock(*it, BlockBuf, nLen))
            {
                if(nLen == 0)
                {
                    std::string strtmp = (*it).toString();
                    strtmp += " ��Ч������";
                    CDeuCallEvent::CallEventState(nIndex, strtmp.c_str(), m_pEventFeature, pEventAdapter);
                    nIndex++;
                    nProccesor++;
                    continue;
                }

                if(!m_pIUrlNetwork->replaceData(*it, BlockBuf, nLen, vecErr, pOutExcep))
                {
                    LOGPACKAGE _LOGPACKAGE;
                    _LOGPACKAGE.id = *it;
                    _LOGPACKAGE.m_strDBPath = m_strDBPath;
                    _LOGPACKAGE.nError = pOutExcep->getReturnCode();
                    _LOGPACKAGE.strErr = pOutExcep->getMessage();
                    _LOGPACKAGE.nOperateType = DEU_ADD_DATA;
                    _LOGPACKAGE.vecErr.assign(vecErr.begin(), vecErr.end());
                    OutputLog(m_pEventFeature, _LOGPACKAGE, pEventAdapter);
                    if (m_bBatch)
                    {
                        m_bIsErrFlag = true;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                if ((*it).TileID.m_nType == TERRAIN_DOM_ID || (*it).TileID.m_nType == TERRAIN_DEM_ID)
                {
                    //��ȡ�½�ͼ��������ʵ����������
                    std::string strName = "";
                    OpenSP::sp<logical::IPropertyManager> pIPropertyManager = NULL;
                    OpenSP::sp<deunw::IDEUNetwork>        pIDEUNetwork      = NULL;
                    OpenSP::sp<logical::IProperty>        pIProperty        = NULL;

                    pIPropertyManager = logical::createPropertyManager();
                    pIProperty = pIPropertyManager->createPropertyByBsonStream(BlockBuf, nLen);
                    if (pIProperty != NULL)
                    {
                        OpenSP::sp<logical::IProperty> pIPropertyName = pIProperty->findProperty("Name", true);
                        if (pIPropertyName != NULL)
                        {
                            strName = pIPropertyName->getValueAsString();
                        }
                    }

                    //��������/Ӱ��ͼ�㣬�����ʵ��
                    ID rootLayerID;
                    ID subLayerID = ID::genNewID();
                    subLayerID.ObjectID.m_nDataSetCode = 6;
                    if (m_DATATYPE == DOM)
                    {
                        rootLayerID = ID::getTerrainDOMLayerRootID();
                        subLayerID.ObjectID.m_nType = TERRAIN_DOM_LAYER_ID;
                    }
                    else
                    {
                        rootLayerID = ID::getTerrainDEMLayerRootID();
                        subLayerID.ObjectID.m_nType = TERRAIN_DEM_LAYER_ID;
                    }

                    bson::bsonDocument bsonDoc;
                    bsonDoc.AddStringElement("ID", subLayerID.toString().c_str());
                    bsonDoc.AddStringElement("Name", strName.c_str());

                    bson::bsonArrayEle* bsonBoundArr = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("BoundingSphere"));
                    bsonBoundArr->AddDblElement(0.0);
                    bsonBoundArr->AddDblElement(0.0);
                    bsonBoundArr->AddDblElement(0.0);
                    bsonBoundArr->AddDblElement(0.0);

                    bson::bsonArrayEle* bsonChildIDArr  = dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("ChildrenID"));
                    bsonChildIDArr->AddStringElement((*it).toString().c_str());
                    bson::bsonArrayEle* bsonParentIDArr = NULL;//ParentID����ȡ����--�²�dynamic_cast<bson::bsonArrayEle*>(bsonDoc.AddArrayElement("ParentID"));
                    bsonParentIDArr->AddStringElement(rootLayerID.toString().c_str());

                    bson::bsonStream bsonStream;
                    bsonDoc.Write(&bsonStream);
                    void*    pBuffer = bsonStream.Data();
                    unsigned nBufLen = bsonStream.DataLen();

#ifdef _DEBUG
                    std::string strJsonString;
                    bsonDoc.JsonString(strJsonString);
#endif

                    /*m_pIUrlNetwork->addLayer(subLayerID, rootLayerID, pBuffer, nBufLen, vecErr, pOutExcep);*/
                    if (!m_pIUrlNetwork->addLayer(subLayerID, rootLayerID, pBuffer, nBufLen, vecErr, pOutExcep))
                    {
                        CDeuCallEvent::CallEventState(nIndex, "�ύͼ��ʧ��...", m_pEventFeature, pEventAdapter);
                    }
                }

                if(BlockBuf)
                {
                    deudbProxy::freeMemory(BlockBuf);
                    BlockBuf = NULL;
                }
            }

            nIndex++;

            if(nRec < 999)
            {
                nRec++;
            }
            else
            {
                nProccesor += nRec;
                char szDesc[50]={0};
                sprintf(szDesc, "����� %.2f%%", ((double)nProccesor / (double)nTaskCount)*100);
                CDeuCallEvent::CallEventState(nIndex, szDesc, m_pEventFeature, pEventAdapter);
                nRec = 0;
            }
        }
    }

    if (m_pLogInfo->setCommitFaildID.size() == 0)
    {
        m_pLogInfo->bFinishFlag = true;
    }

    if(isDelAction == true)
    {
        return 0;
    }

    if(IsStop(nIndex, m_strDBPath, pEventAdapter))
    {
        return 1;
    }

    if (m_bIsFinish)
    {
        std::string strTemp = m_strDBPath;
        strTemp += "�ϴ�ʧ��";
        CDeuCallEvent::CallEventState(nIndex, strTemp.c_str(), m_pEventFeature, pEventAdapter);
        return 1;
    }

    CDeuCallEvent::CallEventState(nIndex, "����� 100%", m_pEventFeature, pEventAdapter);
    std::string strTmp = m_strDBPath;
    strTmp += " ����/Ӱ�������ύ�������";
    CDeuCallEvent::CallEventState(nIndex, strTmp.c_str(), m_pEventFeature, pEventAdapter);
    CDeuCallEvent::CallBreakEventState("�ύ����", m_pEventFeature, pEventAdapter);
    return 0;
}


//�ύ����buffer
unsigned DEULocalTile::submit(const ID& id, void* Buffer, const unsigned int nLen, void (*ErrorDesc)(const char*)) //�����ύbuf
{
    if (Buffer == NULL)
    {
        return 0;
    }

    std::vector<std::string> vecErr;
    OpenSP::sp<cmm::IDEUException> pOutExcep = cmm::createDEUException();

    bson::bsonStream StreamInput, StreamOutput;
    bson::bsonDocument bDoc,bDocTmp;
    bDoc.FromJsonString((const char*)Buffer);
    bDoc.Write(&StreamInput);

    bDocTmp.AddBinElement("Data", StreamInput.Data(), StreamInput.DataLen());
    bDocTmp.Write(&StreamOutput);

    if(!m_pIUrlNetwork->addData(id, StreamOutput.Data(), StreamOutput.DataLen(), vecErr, pOutExcep))
    {
        ErrorDesc(pOutExcep->getMessage().c_str());
        for(unsigned n=0; n<vecErr.size(); n++)
        {
            ErrorDesc(vecErr[n].c_str());
        }
    }

    return 0;
}

//================================= DEULocalModel ===========================================
DEULocalModel::DEULocalModel(ID* pId)
: DEULocalData(pId)
{
}


DEULocalModel::~DEULocalModel(void)
{
}

// ��ʼ��
bool DEULocalModel::initialize(const std::string& strDBPath)
{
    m_strDBPath = strDBPath;

    m_pProxy = deudbProxy::createDEUDBProxy();
    if(m_pProxy == NULL)
        return false;
    if(!m_pProxy->openDB(m_strDBPath))
    {
        delete m_pProxy;
        m_pProxy = NULL;
        return false;
    }

    //����Ԥ����
    std::vector<ID> vecID = m_pProxy->getAllIndices();
    unsigned nCount = vecID.size();

    if(nCount == 0 ) return true;
    int nType = 0;

    for (unsigned n=0; n<nCount; n++)
    {
        if (vecID[n].m_nHighBit == 0 && vecID[n].m_nLowBit == 0 && vecID[n].m_nMidBit == 0)
        {
            continue;
        }

        nType = vecID[n].TileID.m_nType;
        break;
    }

    if((nType == IMAGE_ID) ||(nType == MODEL_ID))
    {
        m_DATATYPE = MODAL;
    }
    else if(nType == PARAM_POINT_ID)
    {
        m_DATATYPE = VECTOR_PT;
    }
    else if(nType == PARAM_LINE_ID)
    {
        m_DATATYPE = VECTOR_LINE;
    }
    else if(nType == PARAM_FACE_ID)
    {
        m_DATATYPE = VECTOR_FACE;
    }

    m_setIDCollection.clear();
    m_setIDCollection.insert(vecID.cbegin(), vecID.cend());

    return true;
}

//�������ϲ����ύ������Ƭ
unsigned DEULocalModel::CreateVirtualTile(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect)
{
    void* BlockBuf; unsigned nLen=0;
    std::vector<std::string> vecErr;
    std::string strErr;
    unsigned nIndex=0;

    // 1> ����������Ƭ
    std::string strTilePath = cmm::genRandomLocalDB();
    DeleteFile(strTilePath.c_str());
    OpenSP::sp<vcm::IVirtualCubeManager> ivt = vcm::createVirtualCubeManager();

    CDeuCallEvent::CallEventState(0, "��ʼ����ģ�͹ҽӵ������ύ...", m_pEventFeature, pEventAdapter);
    if(IDCollect.size() == 0)
    {
        std::set<ID>::iterator it;
        for(it = m_setIDCollection.begin(); it != m_setIDCollection.end(); ++it)
        {
            if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

            if ((*it).m_nHighBit == 0 && (*it).m_nLowBit == 0 && (*it).m_nMidBit == 0)
            {
                continue;
            }

            if ((*it).ObjectID.m_nType != PARAM_POINT_ID && (*it).ObjectID.m_nType!= PARAM_LINE_ID && (*it).ObjectID.m_nType!= PARAM_FACE_ID)
            {
                continue;
            }

            if(m_pProxy->readBlock((*it), BlockBuf, nLen))
            {
                if(nLen == 0) continue;

                bson::bsonStream bs;
                bs.Write(BlockBuf,nLen);
                bs.Reset();
                bson::bsonDocument bDoc;
                bDoc.Read(&bs);

                bson::bsonArrayEle* p_arybs= dynamic_cast<bson::bsonArrayEle*>(bDoc.GetElement("BoundingSphere"));
                if(!p_arybs) continue;

                const double X  = ((bson::bsonElement*)p_arybs->GetElement(0u))->DblValue();
                const double Y  = ((bson::bsonElement*)p_arybs->GetElement(1u))->DblValue();
                const double Z  = ((bson::bsonElement*)p_arybs->GetElement(2u))->DblValue();
                double Radius   = ((bson::bsonElement*)p_arybs->GetElement(3u))->DblValue();
                bson::bsonDoubleEle *pMaxRangeEle = dynamic_cast<bson::bsonDoubleEle *>(bDoc.GetElement("MaxRange"));
                if(pMaxRangeEle)
                {
                    const double dblMaxRange = pMaxRangeEle->DblValue();
                    if(dblMaxRange > Radius)
                    {
                        Radius = dblMaxRange;
                    }
                }

                const cmm::math::Sphered sphere(cmm::math::Point3d(X, Y, Z), Radius);
                ivt->addObject(*it, sphere);

                deudbProxy::freeMemory(BlockBuf);
                m_VirtualIDCollect.insert(*it);
            }
            nIndex++;
        }
    }
    else //�ֲ��ύ����
    {
        for(unsigned n=0; n<IDCollect.size(); n++)
        {
            if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

            std::set<ID>::iterator it = m_setIDCollection.find(IDCollect[n]);
            if(it == m_setIDCollection.end())
                continue;

            if ((*it).m_nHighBit == 0 && (*it).m_nLowBit == 0 && (*it).m_nMidBit == 0)
            {
                continue;
            }

            if(m_pProxy->readBlock((*it), BlockBuf, nLen))
            {

                if(nLen == 0) continue;

                bson::bsonStream bs;
                bs.Write(BlockBuf,nLen);
                bs.Reset();
                bson::bsonDocument bDoc;
                bDoc.Read(&bs);

                bson::bsonArrayEle* p_arybs= dynamic_cast<bson::bsonArrayEle*>(bDoc.GetElement("BoundingSphere"));
                if(!p_arybs) continue;

                double X        = ((bson::bsonElement*)p_arybs->GetElement(0u))->DblValue();
                double Y        = ((bson::bsonElement*)p_arybs->GetElement(1u))->DblValue();
                double Z        = ((bson::bsonElement*)p_arybs->GetElement(2u))->DblValue();
                double Radius    = ((bson::bsonElement*)p_arybs->GetElement(3u))->DblValue();

                const cmm::math::Sphered sphere(cmm::math::Point3d(X, Y, Z), Radius);
                ivt->addObject(*it, sphere);

                deudbProxy::freeMemory(BlockBuf);
            }

            nIndex++;
        }
    }

    ivt->save2DB(strTilePath);


    // EndRegion

    if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

    // 2> �ϲ�������Ƭ
    // BeginRegion

    OpenSP::sp<deudbProxy::IDEUDBProxy> DBTile = deudbProxy::createDEUDBProxy();
    if(!DBTile->openDB(strTilePath)) return 2;

    std::vector<ID> vecID = DBTile->getAllIndices();
    if(vecID.empty())
    {
        return 0;
    }

    bson::bsonDocument bsonDoc;
    for(unsigned n=0; n<vecID.size(); n++)
    {
        if(IsStop(nIndex, m_strDBPath, pEventAdapter)) 
        {
            DBTile->closeDB();
            return 1;
        }

        void* pBuffer = NULL;
        unsigned nLength = 0;
        if(DBTile->readBlock(vecID[n],pBuffer,nLength))
        {
            if(pBuffer)
            {
                bsonDoc.AddBinElement(vecID[n].toString().c_str(),pBuffer,nLength);
                deudbProxy::freeMemory(pBuffer);
            }
        }
        nIndex++;
    }
    DBTile->closeDB();
    DBTile = NULL;

    //ɾ��DBTile�ļ�    
    DeleteVirtualTileFile( strTilePath ); 

    //submit virtual layer
    std::string strDesc = "";
    bson::bsonStream bstreamOutput;
    GetPostPack(bsonDoc, bstreamOutput);

    std::vector<std::string> errVec;
    OpenSP::sp<cmm::IDEUException> pOutExcep = cmm::createDEUException();
    if(!m_pIUrlNetwork->addVirtTile(bstreamOutput.Data(), bstreamOutput.DataLen(), errVec, pOutExcep))
    {
        std::string strDesc = pOutExcep->getMessage();
        CDeuCallEvent::CallEventState(0, strDesc.c_str(), m_pEventFeature, pEventAdapter);
        return 2;
    }
    // EndRegion
    return 0;
}


bool DEULocalModel::DeleteVirtualTileFile( std::string& strTilePath )
{
    std::string strFileName = strTilePath + std::string( ".idx");
    std::string strFileName2 = strTilePath + std::string( "_0.db");;
    BOOL b = DeleteFile( strFileName.c_str() );
    b = DeleteFile( strFileName2.c_str() );

    return b?true:false;
}


unsigned DEULocalModel::PostDataBlock(ea::IEventAdapter *pEventAdapter, std::vector<ID>& IDCollect)    //�ύ��������
{
    void* BlockBuf = NULL; 
    unsigned nLen;
    std::vector<std::string> vecErr;
    std::string strErr;
    int nIndex = 0;
    unsigned nRec=0;
    unsigned nProccesor=0;
    unsigned nTaskCount=m_setIDCollection.size();
    OpenSP::sp<cmm::IDEUException> pOutExcep = cmm::createDEUException();
    if (pOutExcep == NULL)
    {
        return -1;
    }

    CDeuCallEvent::CallEventState(0, "��ʼ����ģ�������ύ...", m_pEventFeature, pEventAdapter);
    m_pLogInfo->sCurCommitFile = m_strDBPath;
    m_pLogInfo->nTotalIDCount = m_setIDCollection.size();

    if(IDCollect.size() == 0)
    {
        std::set<ID>::iterator it;
        for(it = m_setIDCollection.begin(); it != m_setIDCollection.end(); ++it)
        {
            if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

            if ((*it).m_nHighBit == 0 && (*it).m_nLowBit == 0 && (*it).m_nMidBit == 0)
            {
                continue;
            }

            if(m_pProxy->readBlock((*it), BlockBuf, nLen))
            {
                if(nLen == 0)
                {
                    std::string strtmp = (*it).toString();
                    strtmp += " ��Ч������";
                    CDeuCallEvent::CallEventState(nIndex, strtmp.c_str(), m_pEventFeature, pEventAdapter);
                    nIndex++;
                    nRec++;
                    continue;
                }

                if ((*it).TileID.m_nDataSetCode == 6 && (*it).ObjectID.m_nType == CULTURE_LAYER_ID)
                {
                    ID IdRoot = ID::getCultureLayerRootID();
                    if (!m_pIUrlNetwork->addLayer((*it), IdRoot, BlockBuf, nLen, vecErr, pOutExcep))
                    {
                        CDeuCallEvent::CallEventState(nIndex, "�ύͼ��ʧ��...", m_pEventFeature, pEventAdapter);
                    }

                    continue;
                }
                else if ((*it).TileID.m_nDataSetCode == 6 && (*it).ObjectID.m_nType == SYMBOL_CATEGORY_ID)
                {
                    ID IdRoot = ID::getSymbolCategoryRootID();
                    continue;
                }

                if(!m_pIUrlNetwork->replaceData((*it), BlockBuf, nLen, vecErr, pOutExcep))
                {
                    LOGPACKAGE _LOGPACKAGE;
                    _LOGPACKAGE.id = (*it);
                    _LOGPACKAGE.m_strDBPath = m_strDBPath;
                    _LOGPACKAGE.nError = pOutExcep->getReturnCode();
                    _LOGPACKAGE.strErr = pOutExcep->getMessage();
                    _LOGPACKAGE.nOperateType = DEU_ADD_DATA;
                    _LOGPACKAGE.vecErr.assign(vecErr.begin(), vecErr.end());
                    OutputLog(m_pEventFeature, _LOGPACKAGE, pEventAdapter);
                    if (m_bBatch)
                    {
                        m_bIsErrFlag = true;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                if(BlockBuf)
                {
                    deunw::freeMemory(BlockBuf);
                    BlockBuf = NULL;
                }
            }

            nIndex++;

            if(nRec < 999)
            {
                nRec++;
            }
            else
            {
                nProccesor += nRec;
                char szDesc[50]={0};
                sprintf(szDesc, "����� %.2f%%", ((double)nProccesor / (double)nTaskCount)*100);
                CDeuCallEvent::CallEventState(nIndex, szDesc, m_pEventFeature, pEventAdapter);
                nRec = 0;
            }
        }
    }
    else //����Ǿֲ��ύ
    {
        std::set<ID>::iterator it;
        for(unsigned n=0; n<IDCollect.size(); n++)
        {
            if(IsStop(nIndex, m_strDBPath, pEventAdapter)) return 1;

            it = m_setIDCollection.find(IDCollect[n]);

            if ((*it).m_nHighBit == 0 && (*it).m_nLowBit == 0 && (*it).m_nMidBit == 0)
            {
                continue;
            }

            if(m_pProxy->readBlock((*it), BlockBuf, nLen))
            {
                if(nLen == 0)
                {
                    std::string strtmp = (*it).toString();
                    strtmp += " ��Ч������";
                    CDeuCallEvent::CallEventState(nIndex, strtmp.c_str(), m_pEventFeature, pEventAdapter);
                    nIndex++;
                    nRec++;
                    continue;
                }

                if ((*it).TileID.m_nDataSetCode == 6 && (*it).ObjectID.m_nType == CULTURE_LAYER_ID)
                {
                    ID IdRoot = ID::getCultureLayerRootID();
                    if (!m_pIUrlNetwork->addLayer((*it), IdRoot, BlockBuf, nLen, vecErr, pOutExcep))
                    {
                        CDeuCallEvent::CallEventState(nIndex, "�ύͼ��ʧ��...", m_pEventFeature, pEventAdapter);
                    }

                    continue;
                }
                else if ((*it).TileID.m_nDataSetCode == 6 && (*it).ObjectID.m_nType == SYMBOL_CATEGORY_ID)
                {
                    ID IdRoot = ID::getSymbolCategoryRootID();
                    continue;
                }

                if(!m_pIUrlNetwork->replaceData((*it), BlockBuf, nLen, vecErr, pOutExcep))
                {
                    LOGPACKAGE _LOGPACKAGE;
                    _LOGPACKAGE.id = (*it);
                    _LOGPACKAGE.m_strDBPath = m_strDBPath;
                    _LOGPACKAGE.nError = pOutExcep->getReturnCode();
                    _LOGPACKAGE.strErr = pOutExcep->getMessage();
                    _LOGPACKAGE.nOperateType = DEU_ADD_DATA;
                    _LOGPACKAGE.vecErr.assign(vecErr.begin(), vecErr.end());
                    OutputLog(m_pEventFeature, _LOGPACKAGE, pEventAdapter);
                    if (m_bBatch)
                    {
                        m_bIsErrFlag = true;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                if(BlockBuf)
                {
                    deunw::freeMemory(BlockBuf);
                    BlockBuf = NULL;
                }
            }

            nIndex++;

            if(nRec < 999)
            {
                nRec++;
            }else
            {
                nProccesor += nRec;
                char szDesc[50]={0};
                sprintf(szDesc, "����� %.2f%%", ((double)nProccesor / (double)nTaskCount)*100);
                CDeuCallEvent::CallEventState(nIndex, szDesc, m_pEventFeature, pEventAdapter);
                nRec = 0;
            }
        }
    }

    //��������
    nIndex++;

    if (m_pLogInfo->setCommitFaildID.size() == 0)
    {
        m_pLogInfo->bFinishFlag = true;
    }

    if (IsStop(nIndex, m_strDBPath, pEventAdapter))
    {
        return 1;
    }
    else
    {
        if (m_bIsErrFlag)
        {
            std::string strtmp = m_strDBPath;
            strtmp += "�ϴ�ʧ��";
            CDeuCallEvent::CallEventState(nIndex, strtmp.c_str(), m_pEventFeature, pEventAdapter);
            return 1;
        }
        else
        {
            CDeuCallEvent::CallEventState(nIndex, "����� 100%", m_pEventFeature, pEventAdapter);
        }
    }

    return 0;
}



/*
 �ύ��ģ��/ʸ��������ҵ���߼�������
    1. �������ݶ�Ӧ��������Ƭ��
     2. �ڴӷ����������ض�Ӧ��������Ƭ��֮�ϲ������ύ��
     3. ���ύ�������ݿ飻
     4. ���ҽ����ݼ�������Ϣ������ͼ���ϣ�
    5. �ύ�������ݵ��������ϣ�
*/
unsigned DEULocalModel::submit(ea::IEventAdapter* pEventAdapter, std::vector<ID>& IDCollect)
{
    if(m_setIDCollection.size() == 0) return 0;
    std::string strDesc;
    unsigned nRet = CreateVirtualTile(pEventAdapter, IDCollect);
    if (nRet == 0)
    {
        strDesc = m_strDBPath+" ģ�͹ҽӵ������ύ���";
        CDeuCallEvent::CallEventState(0, strDesc.c_str(), m_pEventFeature, pEventAdapter);
    }
    else if (nRet == 1)
    {
        //strDesc = m_strDBPath+" ȡ���ύ";
        //CDeuCallEvent::CallEventState(0, strDesc.c_str(), m_pEventFeature, pEventAdapter);
        return 1;
    }
    else if (nRet == 2)
    {
        std::set<ID>::iterator it;
        for(it = m_VirtualIDCollect.begin(); it != m_VirtualIDCollect.end(); ++it)
        {
            if (m_bIsFinish)
            {
                break;
            }
            strDesc = (*it).toString();
            m_pLogInfo->setCommitFaildID.insert(strDesc);

            strDesc += " �ύ ";
            strDesc += "ʧ�� ������Ƭδ�ύ�ɹ�";
            CDeuCallEvent::CallEventState(0, strDesc.c_str(), m_pEventFeature, pEventAdapter);
            m_pLogInfo->setLogInfo.insert(strDesc);
        }
        if (m_pLogInfo->setCommitFaildID.size() == 0)
        {
            m_pLogInfo->bFinishFlag = true;
        }
    }

    nRet = PostDataBlock(pEventAdapter, IDCollect);
    if(nRet == 0)
    {
        strDesc = m_strDBPath+" �����ύ�������";
        CDeuCallEvent::CallEventState(0, strDesc.c_str(), m_pEventFeature, pEventAdapter);
    }
    else if (nRet == 1)
    {
        //strDesc = m_strDBPath+" ȡ���ύ";
        //CDeuCallEvent::CallEventState(0, strDesc.c_str(), m_pEventFeature, pEventAdapter);
        return 1;
    }

    CDeuCallEvent::CallBreakEventState("�ύ����", m_pEventFeature, pEventAdapter);

    return 0;
}

//�����ύ
unsigned DEULocalModel::submit(const ID id, ea::IEventAdapter *pEventAdapter)
{
    return 0;
}


