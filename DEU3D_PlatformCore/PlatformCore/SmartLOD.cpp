#include "SmartLOD.h"
#include <algorithm>
#include <osg/CullStack>
#include <OpenThreads/Atomic>
#include <iostream>

float SmartLOD::ms_fltSightBiasRatio = 1.0f;

SmartLOD::SmartLOD(void) : m_fltPositionBiasRatio(1.0f)
{
}


void SmartLOD::traverse(osg::NodeVisitor &nv)
{
    switch(nv.getTraversalMode())
    {
    case(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(), _children.end(), osg::NodeAcceptOp(nv));
            break;
    case(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float required_range = 0.0f;
            if (_rangeMode == DISTANCE_FROM_EYE_POINT)
            {
                required_range = nv.getDistanceToViewPoint(getCenter());
            }
            else
            {
                osg::CullStack *cullStack = dynamic_cast<osg::CullStack *>(&nv);
                if (cullStack)
                {
                    required_range = cullStack->clampedPixelSize(getBound());
                }
                else
                {
                    const float fltRangeRatio = ms_fltSightBiasRatio * m_fltPositionBiasRatio;
                    for(unsigned int i = 0u; i < _rangeList.size(); ++i)
                    {
                        const MinMaxPair &range = _rangeList[i];
                        required_range = osg::maximum(required_range, range.first * fltRangeRatio);
                    }
                }
            }

            const unsigned int numChildren = std::min(_children.size(), _rangeList.size());
            const float fltRangeRatio = ms_fltSightBiasRatio * m_fltPositionBiasRatio;
            for(unsigned int i = 0u; i < numChildren; ++i)
            {
                const MinMaxPair &range = _rangeList[i];
                if (range.first * fltRangeRatio <= required_range && required_range < range.second * fltRangeRatio)
                {
                    _children[i]->accept(nv);
                }
            }
           break;
        }
        default:    break;
    }
}

