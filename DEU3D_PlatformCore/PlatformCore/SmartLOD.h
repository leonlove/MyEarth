#ifndef SMART_LOG_H_ED734088_F234_4C2D_994B_4620FB60CC94_INCLUDE
#define SMART_LOG_H_ED734088_F234_4C2D_994B_4620FB60CC94_INCLUDE

#include <osg/LOD>

class SmartLOD : public osg::LOD
{
public:
    SmartLOD(void);
    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    SmartLOD(const SmartLOD &, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY){}
    META_Node(osg, SmartLOD);

public:
    static void     setSightBiasRatio(float fltRatio)       {   ms_fltSightBiasRatio = fltRatio;    }
    void            setPositionBiasRatio(float fltRatio)    {   m_fltPositionBiasRatio = fltRatio;   }

protected:
    virtual void traverse(osg::NodeVisitor &nv);

protected:
    static float    ms_fltSightBiasRatio;
    float           m_fltPositionBiasRatio;
};

#endif
