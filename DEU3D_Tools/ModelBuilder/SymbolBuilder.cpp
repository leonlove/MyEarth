//#include "SymbolBuilder.h"
//#include <Common\DEUBson.h>
//#include "Model.h"
//#include "Source3D.h"
//#include <osgDB\ReadFile>
//#include <strstream>
//#include "SimplifyTexture.h"
//#include "ErrorCode.h"
//
//
//ISymbolBuilder* createSymbolBuilder(void)
//{
//    return (ISymbolBuilder*)new SymbolBuilder();
//}
//
//SymbolBuilder::SymbolBuilder(void)
//{
//    _dataset_code = 0;
//}
//
//bool SymbolBuilder::writeCategory(bsonArrayEle *pSymbolIDs)
//{
//	bson::bsonDocument bDoc;
//
//	FILE *pfile;	
//	std::string path = strTargetPath;
//	path += ".category";
//
//	bool bFileExist = cmm::isFileExist(path);
//	std::string strJson;
//
//	if(bFileExist){
//
//		pfile = fopen(path.c_str(), "r+");
//		if(pfile)
//		{
//			char* szbuff = NULL; unsigned int nLen = 0;
//			fseek(pfile, 0, SEEK_END);
//			nLen = ftell(pfile);
//			fseek(pfile, 0, SEEK_SET);
//			szbuff = new char[nLen+1];
//			memset(szbuff, 0, sizeof(nLen+1));
//
//			fread(szbuff, nLen, 1, pfile);
//			fseek(pfile, 0, SEEK_SET);
//
//			bDoc.FromJsonString(szbuff);
//			bson::bsonArrayEle* p = dynamic_cast<bson::bsonArrayEle*>(bDoc.GetElement("ChildrenID"));
//			for(unsigned i=0; i<pSymbolIDs->ChildCount(); i++)
//			{
//				std::string str;
//				pSymbolIDs->GetElement(i)->ValueString(str, false);
//				p->AddStringElement(str.c_str());
//			}
//
//			bDoc.JsonString(strJson);
//			fwrite(strJson.c_str(), strJson.length(), 1, pfile);
//
//			delete []szbuff;
//
//			fclose(pfile);
//		}
//
//	}
//	else{
//
//		ID id = ID::genNewID();
//
//		bDoc.AddStringElement("ID", id.toString().c_str());
//		bDoc.AddStringElement("Name", "");
//		bDoc.AddStringElement("Descriptions", "本地符号库");
//
//		bson::bsonArrayEle* p = dynamic_cast<bson::bsonArrayEle*>(bDoc.AddArrayElement("ChildrenID"));
//
//		for(unsigned i=0; i<pSymbolIDs->ChildCount(); i++)
//		{
//			std::string str;
//			pSymbolIDs->GetElement(i)->ValueString(str, false);
//			p->AddStringElement(str.c_str());
//		}
//
//        bDoc.AddArrayElement("ParentID");
//
//		pfile = fopen(path.c_str(), "w+");
//		if(pfile)
//		{
//			bDoc.JsonString(strJson);
//			fwrite(strJson.c_str(), strJson.length(), 1, pfile);
//			fclose(pfile);
//		}
//        else
//        {
//            return false;
//        }
//	}
//    return true;
//}
//
//void SymbolBuilder::writeConfigFile(sp<IDEUException> e)
//{
//    const std::string &strFile = _db_path + ".dscfg";
//
//    bsonDocument doc;
//    bsonArrayEle *pSymbolIDs = readOrCreateIDList(doc, _db);
//
//    if (pSymbolIDs)
//    {
//        std::string strJson;
//        if (!doc.JsonString(strJson) && e.valid())
//        {
//            e->setReturnCode(MAKE_ERR_CODE(1));
//            e->setMessage("bson转json失败");
//            e->setTargetSite("SymbolBuilder::writeConfigFile()");
//            writeLog(e->getMessage().c_str());
//            return;
//        }
//            
//        FILE *pFile = fopen(strFile.c_str(), "w");
//
//        if (pFile == NULL && e.valid())
//        {
//            e->Reset();
//            e->setReturnCode(MAKE_ERR_CODE(2));
//            std::string tmp = strFile + " 创建失败";
//            e->setMessage(tmp);
//            e->setTargetSite("SymbolBuilder::writeConfigFile()");
//            writeLog(tmp.c_str());
//            return;
//        }
//
//        fwrite(strJson.c_str(), 1, strJson.size(), pFile);
//        fclose(pFile);
//
//        if (!writeCategory(pSymbolIDs))
//        {
//            e->Reset();
//            e->setReturnCode(MAKE_ERR_CODE(2));
//            std::string tmp = strTargetPath + ".category 创建失败";
//            e->setMessage(tmp);
//            e->setTargetSite("SymbolBuilder::writeConfigFile()");
//            writeLog(tmp.c_str());
//            return;
//        }
//    }
//
//    writeLog("完成");
//    if (e.valid()) e->Reset();
//}
//
//void SymbolBuilder::initialize(const string &strTargetDB, unsigned nDataSetCode, ea::IEventAdapter *ea, sp<IDEUException> e)
//{
//    _ea = ea;
//
//    if(strTargetDB.empty())
//    {
//        _db_path = cmm::genLocalTempDB();
//    }
//    else
//    {
//        _db_path = strTargetDB;
//    }
//
//    _dataset_code = nDataSetCode;
//
//    _db = deudbProxy::createDEUDBProxy();
//
//    const unsigned nReadBufferSize = 256u * 1024u * 1024u;
//    const unsigned nWriteBufferSize = 64u * 1024u * 1024u;
//    
//	strTargetPath = strTargetDB;
//    if (!_db->openDB(strTargetDB, nReadBufferSize, nWriteBufferSize) && e.valid())
//    {
//        e->setReturnCode(MAKE_ERR_CODE(6));
//        e->setMessage("写数据库失败");
//        e->setTargetSite("SymbolBuilder::initialize()");
//    }
//}
//
//void SymbolBuilder::buildIveSymbol(const std::string &strFile, sp<IDEUException> e)
//{
//    Source3D        src;
//
//    //获取符号ID表(整个DB里会存一个符号ID列表，这条记录自身的ID设置成全0,用来保存DB里所有的符号ID)
//    bsonDocument doc;
//    bsonArrayEle *pSymbolIDs = readOrCreateIDList(doc, _db);
//
//    if (!src.createSymbols(strFile, _dataset_code, pSymbolIDs, _db) && e.valid())
//    {
//        e->setReturnCode(MAKE_ERR_CODE(7));
//        e->setMessage("创建Ive符号失败");
//        e->setTargetSite("SymbolBuilder::buildIveSymbol()");
//        return;
//    }
//
//    //4. 把ID列表入库
//    sp<IDEUException> eInternal = writeIDList(doc, _db);
//    if (eInternal->getReturnCode())
//    {
//        writeLog("符号列表入库失败\r\n");
//        *e.get() = *eInternal.get();
//        return;
//    }
//
//    if (e.valid()) e->Reset();
//}
//
//void SymbolBuilder::buildParamSymbol(ISymbol *pSymbol, sp<IDEUException> e)
//{
//    // 获取符号ID表(整个DB里会存一个符号ID列表，这条记录自身的ID设置成全0,用来保存DB里所有的符号ID)
//    bsonDocument id_list;
//    bsonArrayEle *pIDs = readOrCreateIDList(id_list, _db);
//
//    bson::bsonDocument doc;
//    pSymbol->toBson(doc);
//
//    bsonStream stream;
//    doc.Write(&stream);
//
//    if (!doc.Write(&stream) && e.valid())
//    {
//        e->setReturnCode(MAKE_ERR_CODE(5));
//        e->setMessage("bson流转换失败");
//        e->setTargetSite("SymbolBuilder::buildParamSymbol()");
//        return;
//    }
//
//    if (!_db->updateBlock(pSymbol->getID().toString(), stream.Data(), stream.DataLen()) && e.valid())
//    {
//        e->setReturnCode(MAKE_ERR_CODE(6));
//        e->setMessage("写数据库失败");
//        e->setTargetSite("SymbolBuilder::buildParamSymbol()");
//        return;
//    }
//
//    pIDs->AddStringElement(pSymbol->getID().toString().c_str());
//
//    //4. 把ID列表入库
//    sp<IDEUException> eInternal = writeIDList(id_list, _db);
//    if (eInternal->getReturnCode() && e.valid())
//    {
//        writeLog("符号列表入库失败\r\n");
//        return;
//    }
//
//    if (e.valid()) e->Reset();
//}
//
//ID SymbolBuilder::insertImage(const std::string &strFile, sp<IDEUException> e)
//{
//    ID id;
//    id.setInvalid();
//
//    osg::ref_ptr<osg::Image> img = osgDB::readImageFile(strFile);
//
//    if (img.get())
//    {
//        osg::ref_ptr<osg::Image> dds_img = CompressImage2DDS(img.get(), 1);
//        id = ID::genNewID();
//
//        ostrstream os;
//        osgDB::ReaderWriter             *w = osgDB::Registry::instance()->getReaderWriterForExtension("dds");
//        osgDB::ReaderWriter::WriteResult r = w->writeImage(*dds_img, os);
//
//        if (!_db->updateBlock(id, os.str(), os.pcount()))
//        {
//            id.setInvalid();
//        }
//    }
//
//    return id;
//}