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

#include <osg/Node>
#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/Notify>
#include <osg/OccluderNode>
#include <osg/Transform>
#include <osg/UserDataContainer>

#include <algorithm>

using namespace osg;

namespace osg
{
    /// Helper class for generating NodePathList.
    class CollectParentPaths : public NodeVisitor
    {
    public:
        CollectParentPaths(const osg::Node* haltTraversalAtNode=0) : 
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_PARENTS),
            _haltTraversalAtNode(haltTraversalAtNode)
        {
        }

        virtual void apply(osg::Node& node)
        {
            if (node.getNumParents()==0 || &node==_haltTraversalAtNode)
            {
                _nodePaths.push_back(getNodePath());
            }
            else
            {
                traverse(node);
            }
       }

        const Node*     _haltTraversalAtNode;
        NodePath        _nodePath;
        NodePathList    _nodePaths;
    };
}

Node::Node()
    :Object(true)
{
    _boundingSphereComputed = false;
    _nodeMask = 0xffffffff;
    
    _numChildrenRequiringUpdateTraversal = 0;

    _numChildrenRequiringEventTraversal = 0;

    _cullingActive = true;
    _numChildrenWithCullingDisabled = 0;

	m_iFlag = 0;
}

Node::Node(const Node& node,const CopyOp& copyop):
        Object(node,copyop),
        _boundingSphere(node._boundingSphere),
        _boundingSphereComputed(node._boundingSphereComputed),
        _parents(), // leave empty as parentList is managed by Group.
        _updateCallbacks(node._updateCallbacks),
        _numChildrenRequiringUpdateTraversal(0), // assume no children yet.
        _numChildrenRequiringEventTraversal(0), // assume no children yet.
        _cullCallbacks(node._cullCallbacks),
        _cullingActive(node._cullingActive),
        _numChildrenWithCullingDisabled(0), // assume no children yet.
        _nodeMask(node._nodeMask)
{
    setStateSet(copyop(node._stateset.get()));
}

Node::~Node()
{
    // cleanly detatch any associated stateset (include remove parent links)
    setStateSet(0);   
}

void Node::addParent(osg::Group* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    _parents.push_back(node);
}

void Node::removeParent(osg::Group* node)
{
    OpenThreads::ScopedPointerLock<OpenThreads::Mutex> lock(getRefMutex());

    ParentList::iterator pitr = std::find(_parents.begin(),_parents.end(),node);
    if (pitr!=_parents.end()) _parents.erase(pitr);
}

void Node::accept(NodeVisitor& nv)
{
    if (nv.validNodeMask(*this)) 
    {
        nv.pushOntoNodePath(this);
        nv.apply(*this);
        nv.popFromNodePath();
    }
}


void Node::ascend(NodeVisitor& nv)
{
    std::for_each(_parents.begin(),_parents.end(),NodeAcceptOp(nv));
}

void Node::setStateSet(osg::StateSet* stateset)
{
    // do nothing if nothing changed.
    if (_stateset==stateset) return;
    
    // track whether we need to account for the need to do a update or event traversal.
    int delta_update = 0;
    int delta_event = 0;

    // remove this node from the current statesets parent list 
    if (_stateset.valid())
    {
        _stateset->removeParent(this);
        //if (_stateset->requiresUpdateTraversal()) --delta_update;
        //if (_stateset->requiresEventTraversal()) --delta_event;
    }
    
    // set the stateset.
    _stateset = stateset;
    
    // add this node to the new stateset to the parent list.
    if (_stateset.valid())
    {
        _stateset->addParent(this);
        //if (_stateset->requiresUpdateTraversal()) ++delta_update;
        //if (_stateset->requiresEventTraversal()) ++delta_event;
    }
    
    if (delta_update!=0)
    {
        setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+delta_update);
    }

    if (delta_event!=0)
    {
        setNumChildrenRequiringEventTraversal(getNumChildrenRequiringEventTraversal()+delta_event);
    }
}

osg::StateSet* Node::getOrCreateStateSet()
{
    if (!_stateset) setStateSet(new StateSet);
    return _stateset.get();
}

NodePathList Node::getParentalNodePaths(osg::Node* haltTraversalAtNode) const
{
    CollectParentPaths cpp(haltTraversalAtNode);
    const_cast<Node*>(this)->accept(cpp);
    return cpp._nodePaths;
}

MatrixList Node::getWorldMatrices(const osg::Node* haltTraversalAtNode) const
{
    CollectParentPaths cpp(haltTraversalAtNode);
    const_cast<Node*>(this)->accept(cpp);
    
    MatrixList matrices;
    
    for(NodePathList::iterator itr = cpp._nodePaths.begin();
        itr != cpp._nodePaths.end();
        ++itr)
    {
        NodePath& nodePath = *itr;
        if (nodePath.empty())
        {
            matrices.push_back(osg::Matrix::identity());
        }
        else
        {
            matrices.push_back(osg::computeLocalToWorld(nodePath));
        }
    }
    
    return matrices;
}


void Node::addUpdateCallback(NodeCallback* nc)
{
    if(!nc) return;
    NodeCallbackList::const_iterator itorFind = std::find(_updateCallbacks.begin(), _updateCallbacks.end(), nc);
    if(itorFind != _updateCallbacks.end())
    {
        return;
    }

    if(_updateCallbacks.empty())
    {
        applyUpdateCallbackCounter(1);
    }

    _updateCallbacks.push_back(nc);
}


void Node::removeUpdateCallback(NodeCallback* nc)
{
    if(!nc) return;
    NodeCallbackList::const_iterator itorFind = std::find(_updateCallbacks.begin(), _updateCallbacks.end(), nc);
    if(itorFind == _updateCallbacks.end())
    {
        return;
    }

    if(_updateCallbacks.size() == 1u)
    {
        applyUpdateCallbackCounter(-1);
    }
    _updateCallbacks.erase(itorFind);
}


void Node::clearUpdateCallback(void)
{
    applyUpdateCallbackCounter(-1);
    _updateCallbacks.clear();
}


void Node::applyUpdateCallbackCounter(int nIncrease)
{
    if(nIncrease == 0)  return;
    if(_numChildrenRequiringUpdateTraversal != 0u || _parents.empty())
    {
        return;
    }

    // the number of callbacks has changed, need to pass this
    // on to parents so they know whether app traversal is
    // required on this subgraph.
    for(ParentList::iterator itr =_parents.begin();
        itr != _parents.end();
        ++itr)
    {
        Group *pGroup = *itr;
        const unsigned nCount = pGroup->getNumChildrenRequiringUpdateTraversal();
        pGroup->setNumChildrenRequiringUpdateTraversal(nCount + nIncrease);
    }
}


void Node::addEventCallback(NodeCallback* nc)
{
    if(!nc) return;
    NodeCallbackList::const_iterator itorFind = std::find(_eventCallbacks.begin(), _eventCallbacks.end(), nc);
    if(itorFind != _eventCallbacks.end())
    {
        return;
    }

    if(_eventCallbacks.empty())
    {
        applyEventCallbackCounter(1);
    }
    _eventCallbacks.push_back(nc);
}


void Node::removeEventCallback(NodeCallback* nc)
{
    if(!nc) return;
    NodeCallbackList::const_iterator itorFind = std::find(_eventCallbacks.begin(), _eventCallbacks.end(), nc);
    if(itorFind == _eventCallbacks.end())
    {
        return;
    }

    if(_eventCallbacks.size() == 1u)
    {
        applyEventCallbackCounter(-1);
    }

    _eventCallbacks.erase(itorFind);
}


void Node::clearEventCallback(void)
{
    applyEventCallbackCounter(-1);
    _eventCallbacks.clear();
}


void Node::applyEventCallbackCounter(int nIncrease)
{
    if(nIncrease == 0)  return;
    if(_numChildrenRequiringEventTraversal != 0u || _parents.empty())
    {
        return;
    }

    // the number of callbacks has changed, need to pass this
    // on to parents so they know whether app traversal is
    // required on this subgraph.
    for(ParentList::iterator itr =_parents.begin();
        itr != _parents.end();
        ++itr)
    {
        Group *pGroup = *itr;
        const unsigned nCount = pGroup->getNumChildrenRequiringEventTraversal();
        pGroup->setNumChildrenRequiringEventTraversal(nCount + nIncrease);
    }
}


void Node::setNumChildrenRequiringUpdateTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringUpdateTraversal==num) return;

    // note, if _updateCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringUpdateTraversal so no need to inform them.
    if (_updateCallbacks.empty() && !_parents.empty())
    {
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenRequiringUpdateTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                Group *pParent = *itr;
                pParent->setNumChildrenRequiringUpdateTraversal(pParent->getNumChildrenRequiringUpdateTraversal() + delta);
            }

        }
    }
    
    // finally update this objects value.
    _numChildrenRequiringUpdateTraversal=num;
    
}


void Node::setNumChildrenRequiringEventTraversal(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenRequiringEventTraversal==num) return;

    // note, if _EventCallback is set then the
    // parents won't be affected by any changes to
    // _numChildrenRequiringEventTraversal so no need to inform them.
    if (_eventCallbacks.empty() && !_parents.empty())
    {
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenRequiringEventTraversal>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {
                Group *pParent = *itr;
                pParent->setNumChildrenRequiringEventTraversal(pParent->getNumChildrenRequiringEventTraversal() + delta);
            }
        }
    }
    
    // finally Event this objects value.
    _numChildrenRequiringEventTraversal=num;
}

void Node::setCullingActive(bool active)
{
    // if no changes just return.
    if (_cullingActive == active) return;
    
    // culling active has been changed, will need to update
    // both _cullActive and possibly the parents numChildrenWithCullingDisabled
    // if culling disabled changes.

    // update the parents _numChildrenWithCullingDisabled
    // note, if _numChildrenWithCullingDisabled!=0 then the
    // parents won't be affected by any app callback change,
    // so no need to inform them.
    if (_numChildrenWithCullingDisabled==0 && !_parents.empty())
    {
        int delta = 0;
        if (!_cullingActive) --delta;
        if (!active) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenWithCullingDisabled(
                        (*itr)->getNumChildrenWithCullingDisabled()+delta );
            }

        }
    }

    // set the cullingActive itself.
    _cullingActive = active;
}

void Node::setNumChildrenWithCullingDisabled(unsigned int num)
{
    // if no changes just return.
    if (_numChildrenWithCullingDisabled==num) return;

    // note, if _cullingActive is false then the
    // parents won't be affected by any changes to
    // _numChildrenWithCullingDisabled so no need to inform them.
    if (_cullingActive && !_parents.empty())
    {
    
        // need to pass on changes to parents.        
        int delta = 0;
        if (_numChildrenWithCullingDisabled>0) --delta;
        if (num>0) ++delta;
        if (delta!=0)
        {
            // the number of callbacks has changed, need to pass this
            // on to parents so they know whether app traversal is
            // required on this subgraph.
            for(ParentList::iterator itr =_parents.begin();
                itr != _parents.end();
                ++itr)
            {    
                (*itr)->setNumChildrenWithCullingDisabled(
                    (*itr)->getNumChildrenWithCullingDisabled()+delta
                    );
            }

        }
    }
    
    // finally update this objects value.
    _numChildrenWithCullingDisabled=num;
}


BoundingSphere Node::computeBound() const
{
    return BoundingSphere();
}


void Node::dirtyBound()
{
    if (_boundingSphereComputed)
    {
        _boundingSphereComputed = false;

        // dirty parent bounding sphere's to ensure that all are valid.
        for(ParentList::iterator itr=_parents.begin();
            itr!=_parents.end();
            ++itr)
        {
            (*itr)->dirtyBound();
        }

    }
}

void Node::setThreadSafeRefUnref(bool threadSafe)
{
    Object::setThreadSafeRefUnref(threadSafe);
    
    if (_stateset.valid()) _stateset->setThreadSafeRefUnref(threadSafe);
    for(unsigned i = 0u; i < _updateCallbacks.size(); i++)
    {
        _updateCallbacks[i]->setThreadSafeRefUnref(threadSafe);
    }
    for(unsigned i = 0u; i < _eventCallbacks.size(); i++)
    {
        _eventCallbacks[i]->setThreadSafeRefUnref(threadSafe);
    }
    for(unsigned i = 0u; i < _cullCallbacks.size(); i++)
    {
        _cullCallbacks[i]->setThreadSafeRefUnref(threadSafe);
    }
}

void Node::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_stateset.valid()) _stateset->resizeGLObjectBuffers(maxSize);

    for(unsigned i = 0u; i < _updateCallbacks.size(); i++)
    {
        _updateCallbacks[i]->resizeGLObjectBuffers(maxSize);
    }
    for(unsigned i = 0u; i < _eventCallbacks.size(); i++)
    {
        _eventCallbacks[i]->resizeGLObjectBuffers(maxSize);
    }
    for(unsigned i = 0u; i < _cullCallbacks.size(); i++)
    {
        _cullCallbacks[i]->resizeGLObjectBuffers(maxSize);
    }
}

void Node::releaseGLObjects(osg::State* state) const
{
    if (_stateset.valid()) _stateset->releaseGLObjects(state);

    for(unsigned i = 0u; i < _updateCallbacks.size(); i++)
    {
        _updateCallbacks[i]->releaseGLObjects(state);
    }
    for(unsigned i = 0u; i < _eventCallbacks.size(); i++)
    {
        _eventCallbacks[i]->releaseGLObjects(state);
    }
    for(unsigned i = 0u; i < _cullCallbacks.size(); i++)
    {
        _cullCallbacks[i]->releaseGLObjects(state);
    }
}



