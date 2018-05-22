/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osg/PagedLOD>
#include <osg/CullStack>
#include <osg/Notify>
#include <osg/LineNode>
#include <algorithm>

using namespace osg;

float PagedLOD::_fltMemRangeRatio = 1.0f;

void PagedLOD::setMemRangeRatio(float fltMemRangeRatio)
{
    _fltMemRangeRatio = osg::clampAbove(fltMemRangeRatio, 0.1f);
}


PagedLOD::PerRangeData::PerRangeData():
    _timeStamp(0.0f),
    _frameNumber(0),
    _frameNumberOfLastReleaseGLObjects(0) {}

PagedLOD::PerRangeData::PerRangeData(const PerRangeData& prd):
    _id(prd._id),
    _filename(prd._filename),
    _timeStamp(prd._timeStamp),
    _frameNumber(prd._frameNumber),
    _frameNumberOfLastReleaseGLObjects(prd._frameNumberOfLastReleaseGLObjects),
    _databaseRequest(prd._databaseRequest){}

PagedLOD::PerRangeData& PagedLOD::PerRangeData::operator = (const PerRangeData& prd)
{
    if (this==&prd) return *this;
    _id = prd._id;
    _filename = prd._filename;
    _timeStamp = prd._timeStamp;
    _frameNumber = prd._frameNumber;
    _frameNumberOfLastReleaseGLObjects = prd._frameNumberOfLastReleaseGLObjects;
    _databaseRequest = prd._databaseRequest;
    return *this;
}

PagedLOD::PagedLOD()
{
    _frameNumberOfLastTraversal = 0;
    _centerMode = USER_DEFINED_CENTER;
    _radius = -1;
    _numChildrenThatCannotBeExpired = 0;
    _disableExternalChildrenPaging = false;
    _bHasLineNode = false;
}

PagedLOD::PagedLOD(const PagedLOD& plod,const CopyOp& copyop):
    LOD(plod,copyop),
    _databaseOptions(plod._databaseOptions),
    _databasePath(plod._databasePath),
    _frameNumberOfLastTraversal(plod._frameNumberOfLastTraversal),
    _numChildrenThatCannotBeExpired(plod._numChildrenThatCannotBeExpired),
    _disableExternalChildrenPaging(plod._disableExternalChildrenPaging),
    _perRangeDataList(plod._perRangeDataList),
    _childCreationInfo(plod._childCreationInfo),
    _bHasLineNode(plod._bHasLineNode)
{
}

PagedLOD::~PagedLOD()
{
}

void PagedLOD::setDatabasePath(const std::string& path)
{
    _databasePath = path;
    if (!_databasePath.empty())
    {
        char& lastCharacter = _databasePath[_databasePath.size()-1];
        const char unixSlash = '/';
        const char winSlash = '\\';

        if (lastCharacter==winSlash)
        {
            lastCharacter = unixSlash;
        }
        else if (lastCharacter!=unixSlash)
        {
            _databasePath += unixSlash;
        }
    }
}


void PagedLOD::traverse(NodeVisitor& nv)
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
                required_range = nv.getDistanceToViewPoint(getCenter());
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
                        required_range = osg::maximum(required_range, range.first * _fltMemRangeRatio);
                    }
                }
            }

            if(getParent(0)->getID().ObjectID.m_nType == 53 || getParent(0)->getID().ObjectID.m_nType == 54 || getParent(0)->getID().ObjectID.m_nType == 55)
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


void PagedLOD::expandPerRangeDataTo(unsigned int pos)
{
    if (pos>=_perRangeDataList.size())
    {
        _perRangeDataList.resize(pos+1);
    }
}

bool PagedLOD::addChild( Node *child )
{
    osg::LineNode *pLineNode = dynamic_cast<osg::LineNode *>(child);
    if(pLineNode)
    {
        _bHasLineNode = true;
    }

    if (LOD::addChild(child))
    {
        expandPerRangeDataTo(_children.size()-1);
        return true;
    }
    return false;
}

bool PagedLOD::addChild(Node *child, float min, float max)
{
    osg::LineNode *pLineNode = dynamic_cast<osg::LineNode *>(child);
    if(pLineNode)
    {
        _bHasLineNode = true;
    }

    if (LOD::addChild(child,min,max))
    {
        expandPerRangeDataTo(_children.size()-1);
        return true;
    }
    return false;
}


bool PagedLOD::addChild(Node *child, float min, float max,const std::string& filename)
{
    osg::LineNode *pLineNode = dynamic_cast<osg::LineNode *>(child);
    if(pLineNode)
    {
        _bHasLineNode = true;
    }

    if (LOD::addChild(child,min,max))
    {
        setFileName(_children.size()-1,filename);
        return true;
    }
    return false;
}

bool PagedLOD::addChild(Node *child, float min, float max,const ID& id)
{
    osg::LineNode *pLineNode = dynamic_cast<osg::LineNode *>(child);
    if(pLineNode)
    {
        _bHasLineNode = true;
    }

    if (LOD::addChild(child,min,max))
    {
        setFileID(_children.size()-1,id);
        return true;
    }
    return false;
}

bool PagedLOD::removeChildren( unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos<_rangeList.size()) _rangeList.erase(_rangeList.begin()+pos, osg::minimum(_rangeList.begin()+(pos+numChildrenToRemove), _rangeList.end()) );
    if (pos<_perRangeDataList.size()) _perRangeDataList.erase(_perRangeDataList.begin()+pos, osg::minimum(_perRangeDataList.begin()+ (pos+numChildrenToRemove), _perRangeDataList.end()) );

    return Group::removeChildren(pos,numChildrenToRemove);
}

bool PagedLOD::removeExpiredChildren(double expiryTime, unsigned int expiryFrame, NodeList& removedChildren)
{
    if (_children.size() <= _numChildrenThatCannotBeExpired)
    {
        return false;
    }

    if(_bHasLineNode)
    {
        for(unsigned cindex = 0u; cindex < _children.size() - 1u; cindex++)
        {
            const PerRangeData &rangeData = _perRangeDataList[cindex];
            if (rangeData._id.isValid() &&
                rangeData._timeStamp < expiryTime &&
                rangeData._frameNumber < expiryFrame)
            {
                osg::Node* node = _children[cindex].get();
                LineNode *pLineNode = dynamic_cast<osg::LineNode *>(node);
                if(pLineNode)
                {
                    pLineNode->OnPagedLODExpired();
                }
            }
        }
    }


    if(getParent(0)->getID().ObjectID.m_nType == 53 || getParent(0)->getID().ObjectID.m_nType == 54 || getParent(0)->getID().ObjectID.m_nType == 55)
    {

        for(NodeList::iterator itor = _children.begin(); itor != _children.end(); ++itor)
        {
            const ID &id = (*itor)->getID();

            for(PerRangeDataList::iterator itor_p = _perRangeDataList.begin(); itor_p != _perRangeDataList.end(); ++itor_p)
            {
                if(id == itor_p->_id && itor_p->_timeStamp < expiryTime && itor_p->_frameNumber < expiryFrame)
                {
                    removedChildren.push_back((*itor));
                }
            }
        }

        for(NodeList::iterator itor = removedChildren.begin(); itor != removedChildren.end(); ++itor)
        {
            Group::removeChildren(getChildIndex((*itor).get()), 1u);
        }
        return true;
    }
    else
    {
        const unsigned cindex = _children.size() - 1u;
        const PerRangeData &rangeData = _perRangeDataList[cindex];
        if ((rangeData._id.isValid() || !rangeData._filename.empty()) &&
            rangeData._timeStamp < expiryTime &&
            rangeData._frameNumber < expiryFrame)
        {
            osg::Node* nodeToRemove = _children[cindex].get();
            removedChildren.push_back(nodeToRemove);
            return Group::removeChildren(cindex, 1u);
        }
    }

    return false;
}
