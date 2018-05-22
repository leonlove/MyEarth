#include <osg/PagedLOD_MultiCenter>
#include <algorithm>
#include <osg/CullStack>
#include <IDProvider/Definer.h>

namespace osg
{

PagedLOD_MultiCenter::PagedLOD_MultiCenter(void)
{
}


PagedLOD_MultiCenter::PagedLOD_MultiCenter(const PagedLOD_MultiCenter& plodm,const CopyOp& copyop):
    PagedLOD(plodm, copyop),
    m_vecReferencedPoints(plodm.m_vecReferencedPoints)
{
}


PagedLOD_MultiCenter::~PagedLOD_MultiCenter(void)
{
}


void PagedLOD_MultiCenter::traverse(NodeVisitor& nv)
{
    // set the frame number of the traversal so that external nodes can find out how active this
    // node is.
    if (nv.getFrameStamp() && 
        nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR) 
    {
        setFrameNumberOfLastTraversal(nv.getFrameStamp()->getFrameNumber());
    }

    const double timeStamp = nv.getFrameStamp() ? nv.getFrameStamp()->getReferenceTime() : 0.0;
    const unsigned int frameNumber = nv.getFrameStamp() ? nv.getFrameStamp()->getFrameNumber() : 0u;
    const bool updateTimeStamp = nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR;

    switch(nv.getTraversalMode())
    {
        case(NodeVisitor::TRAVERSE_ALL_CHILDREN):
            std::for_each(_children.begin(), _children.end(), NodeAcceptOp(nv));
            break;
        case(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN):
        {
            float required_range = 0.0f;
            if (_rangeMode == DISTANCE_FROM_EYE_POINT)
            {
                if(m_vecReferencedPoints.empty())
                {
                    required_range = nv.getDistanceToViewPoint(getCenter());
                }
                else
                {
                    required_range = FLT_MAX;
                    std::vector<Vec3>::const_iterator itorPoint = m_vecReferencedPoints.begin();
                    for( ; itorPoint != m_vecReferencedPoints.end(); ++itorPoint)
                    {
                        const Vec3 &point = *itorPoint;
                        const float flt = nv.getDistanceToViewPoint(point);
                        required_range = osg::minimum(required_range, flt);
                    }
                }
            }
            else
            {
                osg::CullStack* cullStack = dynamic_cast<osg::CullStack*>(&nv);
                if (cullStack)
                {
                    required_range = cullStack->clampedPixelSize(getBound());
                }
                else
                {
                    // fallback to selecting the highest res tile by
                    // finding out the max range
                    for(unsigned int i = 0u; i < _rangeList.size(); ++i)
                    {
                        const MinMaxPair &range = _rangeList[i];
                        required_range = osg::maximum(required_range, range.first);
                    }
                }
            }

            const ID &id = getParent(0)->getID();
            const unsigned nIDType = id.ObjectID.m_nType;
            if(nIDType == PARAM_POINT_ID || nIDType == PARAM_LINE_ID || nIDType == PARAM_FACE_ID)
            {
                for(unsigned int i = 0u; i < _rangeList.size(); ++i)
                {
                    const MinMaxPair &range = _rangeList[i];
                    if ((range.first * _fltMemRangeRatio <= required_range) && (required_range < range.second * _fltMemRangeRatio))
                    {
                        bool hasLoad = false;
                        for(unsigned int j = 0; j < _children.size(); j++)
                        {
                            const ID &child_id =  _children[j]->getID();
                            if(child_id == _perRangeDataList[i]._id)
                            {
                                if (updateTimeStamp)
                                {
                                    _perRangeDataList[i]._timeStamp = timeStamp;
                                    _perRangeDataList[i]._frameNumber = frameNumber;
                                }

                                _children[j]->accept(nv);

                                hasLoad = true;
                                break;
                            }

                            if(j == _children.size() - 1)
                            {
                                for(unsigned int k = 0; k < _perRangeDataList.size(); k++)
                                {
                                    if(_perRangeDataList[k]._id == child_id)
                                    {
                                        if (updateTimeStamp)
                                        {
                                            _perRangeDataList[k]._timeStamp   = timeStamp;
                                            _perRangeDataList[k]._frameNumber = frameNumber;
                                        }
                                        _children[j]->accept(nv);
                                        break;
                                    }
                                }
                            }
                        }

                        if(!hasLoad)
                        {
                            nv.getDatabaseRequestHandler()->requestNodeFile(_perRangeDataList[i]._id,nv.getNodePath(),nv.getFrameStamp(), _perRangeDataList[i]._databaseRequest, _databaseOptions.get(), _childCreationInfo);
                        }
                    }
                }
            }
            else
            {
                int lastChildTraversed = -1;
                bool needToLoadChild = false;
                for(unsigned int i = 0u; i < _rangeList.size(); ++i)
                {
                    const MinMaxPair &range = _rangeList[i];
                    if ((range.first * _fltMemRangeRatio <= required_range) && (required_range < range.second * _fltMemRangeRatio))
                    {
                        if (i < _children.size())
                        {
                            if (updateTimeStamp)
                            {
                                _perRangeDataList[i]._timeStamp = timeStamp;
                                _perRangeDataList[i]._frameNumber = frameNumber;
                            }

                            _children[i]->accept(nv);
                            lastChildTraversed = (int)i;
                        }
                        else
                        {
                            needToLoadChild = true;
                        }
                    }
                }

                if (needToLoadChild)
                {
                    unsigned int numChildren = _children.size();

                    // select the last valid child.
                    if (numChildren > 0u && ((int)numChildren - 1) != lastChildTraversed)
                    {
                        if (updateTimeStamp)
                        {
                            _perRangeDataList[numChildren - 1]._timeStamp   = timeStamp;
                            _perRangeDataList[numChildren - 1]._frameNumber = frameNumber;
                        }
                        _children[numChildren-1]->accept(nv);
                    }

                    // now request the loading of the next unloaded child.
                    if (!_disableExternalChildrenPaging &&
                        nv.getDatabaseRequestHandler() &&
                        numChildren<_perRangeDataList.size())
                    {
                        if(_perRangeDataList[numChildren]._id.isValid())
                        {
                            nv.getDatabaseRequestHandler()->requestNodeFile(_perRangeDataList[numChildren]._id,nv.getNodePath(),nv.getFrameStamp(), _perRangeDataList[numChildren]._databaseRequest, _databaseOptions.get(), _childCreationInfo);
                        }
                        else
                        {
                            if(_databasePath.empty())
                            {
                                nv.getDatabaseRequestHandler()->requestNodeFile(_perRangeDataList[numChildren]._filename,nv.getNodePath(),nv.getFrameStamp(), _perRangeDataList[numChildren]._databaseRequest, _databaseOptions.get(), _childCreationInfo);
                            }
                            else
                            {
                                nv.getDatabaseRequestHandler()->requestNodeFile(_databasePath+_perRangeDataList[numChildren]._filename,nv.getNodePath(),nv.getFrameStamp(), _perRangeDataList[numChildren]._databaseRequest, _databaseOptions.get(), _childCreationInfo);
                            }
                        }
                    }
                }
            }

           break;
        }
        default:
            break;
    }
}



}
