#include "FaceTool.h"

FaceTool::FaceTool(const std::string &strName)
    : PolylineTool(strName)
{
    m_clrFaceColor.m_fltR = 0.0;
    m_clrFaceColor.m_fltG = 1.0;
    m_clrFaceColor.m_fltB = 0.0;
    m_clrFaceColor.m_fltA = 0.3;
}


FaceTool::~FaceTool(void)
{
}


void FaceTool::setFaceColor(const cmm::FloatColor &color)
{
    m_clrFaceColor = color;
}




