/**********************************************************************
 *
 *    FILE:            Node.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Node in binary format to disk.
 *
 *    CREATED BY:        Rune Schmidt Jensen
 *
 *    HISTORY:        Created 10.03.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "Node.h"
#include "MatrixTransform.h"
#include "Group.h"
#include "Object.h"
#include "StateSet.h"
#include "AnimationPathCallback.h"
#include "ClusterCullingCallback.h"

using namespace ive;


void Node::write(DataOutputStream* out){

    // Write node identification.
    out->writeInt(IVENODE);

    // Write out any inherited classes.
    osg::Object*  obj = dynamic_cast<osg::Object*>(this);
    if(obj){
        ((ive::Object*)(obj))->write(out);
    }
    else
        out_THROW_EXCEPTION("Node::write(): Could not cast this osg::Node to an osg::Object.");


    // Write osg::node properties.
    if ( out->getVersion() < VERSION_0012 )
    {
        // Write Name
        out->writeString(getName());
    }
    if(out->getVersion() == VERSION_DEU)
    {
        const ID &id = getID();
        out->writeCharArray((char *)(&id), sizeof(ID));
    }
    // Write culling active
    out->writeBool( getCullingActive());

    // Write Descriptions
    if(out->getVersion() != VERSION_DEU)
    {
        int nDesc =  0;//getDescriptions().size();
        out->writeInt(nDesc);
        //if(nDesc!=0){
        //    std::vector<std::string> desc =  getDescriptions();
        //    for(int i=0;i<nDesc;i++)
        //        out->writeString(desc[i]); 
        //}
    }

    // Write Stateset if any
    out->writeBool( getStateSet()!=0);
    if(getStateSet())
        out->writeStateSet(getStateSet());

    // Write UpdateCallback if any
    osg::AnimationPathCallback* nc = NULL;
    const unsigned nNumUpdateCallback = getNumUpdateCallback();
    for(unsigned n = 0u; n < nNumUpdateCallback; n++)
    {
        osg::NodeCallback *callback = getUpdateCallback(n);
        nc = dynamic_cast<osg::AnimationPathCallback *>(callback);
        if(nc)  break;
    }
    out->writeBool(nc!=0);
    if(nc)
    {
        ((ive::AnimationPathCallback*)(nc))->write(out);
    }
    
    if (out->getVersion() >= VERSION_0006)
    {
        osg::ClusterCullingCallback* ccc = 0;
        const unsigned nNumCullCallback = getNumCullCallback();
        for(unsigned n = 0u; n < nNumCullCallback; n++)
        {
            osg::NodeCallback *callback = getCullCallback(n);
            ccc = dynamic_cast<osg::ClusterCullingCallback*>(callback);
            if(ccc) break;
        }

        out->writeBool(ccc!=0);
        if(ccc)
        {
            ((ive::ClusterCullingCallback*)(ccc))->write(out);
        }
    }


    if (out->getVersion() >= VERSION_0039)
    {
        out->writeBool(false);
    }

    if (out->getVersion() >= VERSION_0010)
    {
        const osg::BoundingSphere bs;// = getInitialBound();
        out->writeBool(bs.valid());
        if (bs.valid())
        {
            out->writeVec3(bs.center());
            out->writeFloat(bs.radius());
        }
    }

    // Write NodeMask
    out->writeUInt(getNodeMask());
}


void Node::read(DataInputStream* in){
    // Peak on the identification id.
    int id = in->peekInt();

    if(id == IVENODE){
        id = in->readInt();
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if(obj){
            ((ive::Object*)(obj))->read(in);
        }
        else
            in_THROW_EXCEPTION("Node::read(): Could not cast this osg::Node to an osg::Object.");

        if ( in->getVersion() < VERSION_0012 )
        {
            // Read name
            setName(in->readString());
        }
        if(in->getVersion() == VERSION_DEU)
        {
            const ID id;
            in->readCharArray((char *)(&id), sizeof(ID));
            setID(id);
        }
        // Read Culling active
        setCullingActive(in->readBool());

        // Read descriptions
        if(in->getVersion() != VERSION_DEU)
        {
            int nDesc = in->readInt();
            if(nDesc!=0){
                for(int i=0;i<nDesc;i++)
                     in->readString();
            }
        }

        // Read StateSet if any
        if(in->readBool())
        {
            setStateSet(in->readStateSet());
        }

        // Read UpdateCallback if any
        if(in->readBool())
        {
            osg::AnimationPathCallback* nc = new osg::AnimationPathCallback();
            ((ive::AnimationPathCallback*)(nc))->read(in);
            addUpdateCallback(nc);
        }

        if (in->getVersion() >= VERSION_0006)
        {
            if(in->readBool())
            {
                osg::ClusterCullingCallback* ccc = new osg::ClusterCullingCallback();
                ((ive::ClusterCullingCallback*)(ccc))->read(in);
                addCullCallback(ccc);
            }
        }

        if (in->getVersion() >= VERSION_0039)
        {
            if(in->readBool())
            {
                //int pacID = in->peekInt();
                //if (pacID==IVEVOLUMEPROPERTYADJUSTMENTCALLBACK)
                //{
                //    //osgVolume::PropertyAdjustmentCallback* pac = new osgVolume::PropertyAdjustmentCallback();
                //    //((ive::VolumePropertyAdjustmentCallback*)(pac))->read(in);
                //    //setEventCallback(pac);
                //}
                //else
                //{
                //    in_THROW_EXCEPTION("Unknown event callback identification in Node::read()");
                //}
            }
        }

        if (in->getVersion() >= VERSION_0010)
        {
            if (in->readBool())
            {
                osg::BoundingSphere bs;
                bs.center() = in->readVec3();
                bs.radius() = in->readFloat();
            }
        }

        // Read NodeMask
        setNodeMask(in->readUInt());
    }
    else{
        in_THROW_EXCEPTION("Node::read(): Expected Node identification");
    }
}
