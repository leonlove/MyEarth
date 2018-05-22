#ifndef POLYLINE_TOOL_H_D36590F9_11DF_47A0_BA7B_7FD94B383C8D_INCLUDE
#define POLYLINE_TOOL_H_D36590F9_11DF_47A0_BA7B_7FD94B383C8D_INCLUDE

#include "IPolylineTool.h"
#include "LineTool.h"

class PolylineTool : virtual public IPolylineTool, public LineTool
{
public:
    explicit PolylineTool(const std::string &strName);
    virtual ~PolylineTool(void);

protected:  // virtual methods from IToolBase
    virtual const   std::string &getType(void) const    {   return POLYLINE_TOOL; }

protected:  // virtual methods from itself
    virtual bool operateOnScreen(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual bool operateOnCulture(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:
    void    sendEvent(bool bFinishOperation);

protected:
    bool    m_bRButtonDown;
};


#endif
