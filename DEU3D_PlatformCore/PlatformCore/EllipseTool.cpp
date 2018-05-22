#include "EllipseTool.h"

EllipseTool::EllipseTool(const std::string &strName)
    : FaceTool(strName)
{
}


EllipseTool::~EllipseTool(void)
{
}


bool EllipseTool::handleEvent(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    return false;
}

