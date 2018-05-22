#ifndef BUILDER_H_65B2DA84_738C_4033_A994_687604D77E16_INCLUDE
#define BUILDER_H_65B2DA84_738C_4033_A994_687604D77E16_INCLUDE

#include "IModelBuilder.h"
#include <Common\DEUBson.h>
#include <EventAdapter\EventAdapter.h>
#include <Common\DEUException.h>
#include <DEUDBProxy/IDEUDBProxy.h>

class Builder
{
public:
    Builder(void);
    ~Builder(void);

    void    setEventAdapter(ea::IEventAdapter *ea);
    static  void writeLog(const char *info);
    static  void updateProgress(unsigned progress, unsigned total);

    static void                setResolution(double h, double v){_resolutionH = h; _resolutionV = v;}
    static cmm::math::Point2i getResolution(){return cmm::math::Point2i(_resolutionH, _resolutionV);}

    static void    setPixelSize(double min, double max){_min_pixel_size = min, _max_pixel_size = max;}
    static cmm::math::Vector2d getPixelSize(){return cmm::math::Vector2d(_min_pixel_size, _max_pixel_size);}

protected:
    //水平分辨率
	static unsigned int   _resolutionH;
    //垂直分辨率
	static unsigned int   _resolutionV;
   //最大可视像素大小
	static float          _max_pixel_size;
    //最小可视像素大小
	static float          _min_pixel_size;

    static OpenSP::sp<ea::IEventAdapter> _ea;
    std::string                          _db_path;

    bson::bsonArrayEle* readOrCreateIDList(bson::bsonDocument &doc, deudbProxy::IDEUDBProxy *db);
    cmm::IDEUException *writeIDList(bson::bsonDocument &doc, deudbProxy::IDEUDBProxy *db);
};

#endif
