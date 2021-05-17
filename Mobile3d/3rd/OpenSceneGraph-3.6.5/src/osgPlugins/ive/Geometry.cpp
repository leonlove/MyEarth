/**********************************************************************
 *
 *    FILE:            Geometry.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Geometry in binary format to disk.
 *
 *    CREATED BY:        Auto generated by iveGenerated
 *                    and later modified by Rune Schmidt Jensen.
 *
 *    HISTORY:        Created 18.3.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Geometry.h"

#include "Exception.h"
#include "Drawable.h"
#include "DrawArrays.h"
#include "DrawArrayLengths.h"
#include "DrawElementsUByte.h"
#include "DrawElementsUShort.h"
#include "DrawElementsUInt.h"

using namespace ive;

void Geometry::write(DataOutputStream* out){
    // Write Geometry's identification.
    out->writeInt(IVEGEOMETRY);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::Drawable*  drawable = dynamic_cast<osg::Drawable*>(this);
    if(drawable){
        ((ive::Drawable*)(drawable))->write(out);
    }
    else
        out_THROW_EXCEPTION("Geometry::write(): Could not cast this osg::Geometry to an osg::Drawable.");


    // Write Geometry's properties.

    // Write primitiveset list.
    int size = getNumPrimitiveSets();
    out->writeInt(size);
    for(int i=0;i<size;i++){
        if(dynamic_cast<osg::DrawArrays*>(getPrimitiveSet(i)))
            ((ive::DrawArrays*)(getPrimitiveSet(i)))->write(out);
        else if(dynamic_cast<osg::DrawArrayLengths*>(getPrimitiveSet(i)))
            ((ive::DrawArrayLengths*)(getPrimitiveSet(i)))->write(out);
        else if(dynamic_cast<osg::DrawElementsUByte*>(getPrimitiveSet(i)))
            ((ive::DrawElementsUByte*)(getPrimitiveSet(i)))->write(out);
        else if(dynamic_cast<osg::DrawElementsUShort*>(getPrimitiveSet(i)))
            ((ive::DrawElementsUShort*)(getPrimitiveSet(i)))->write(out);
        else if(dynamic_cast<osg::DrawElementsUInt*>(getPrimitiveSet(i)))
            ((ive::DrawElementsUInt*)(getPrimitiveSet(i)))->write(out);
        else
            out_THROW_EXCEPTION("Unknown PrimitivSet in Geometry::write()");
    }

    // Write vertex array if any
    out->writeBool(getVertexArray()!=0);
    if (getVertexArray())
    {
        out->writeArray(getVertexArray());
    }
    // Write vertex indices if any
    out->writeBool(false);

    // Write normal array if any
    if ( out->getVersion() < VERSION_0013 )
    {
        osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(getNormalArray());
        out->writeBool(normals!=0);
        if (normals)
        {
            out->writeBinding(normals->getBinding());
            out->writeVec3Array(normals);
        }
    }
    else
    {
        out->writeBool(getNormalArray()!=0);
        if (getNormalArray()!=0)
        {
            out->writeBinding(getNormalArray()->getBinding());
            out->writeArray(getNormalArray());
        }
    }

    // Write normal indices if any
    out->writeBool(false);

    // Write color array if any.
    out->writeBool(getColorArray()!=0);
    if (getColorArray()){
        out->writeBinding(getColorArray()->getBinding());
        out->writeArray(getColorArray());
    }
    // Write color indices if any
    out->writeBool(false);

    // Write secondary color array if any
    out->writeBool(getSecondaryColorArray()!=0);
    if (getSecondaryColorArray()){
        out->writeBinding(getSecondaryColorArray()->getBinding());
        out->writeArray(getSecondaryColorArray());
    }
    // Write second color indices if any
    out->writeBool(false);

    // Write fog coord array if any
    out->writeBool(getFogCoordArray()!=0);
    if (getFogCoordArray()){
        out->writeBinding(getFogCoordArray()->getBinding());
        out->writeArray(getFogCoordArray());
    }
    // Write fog coord indices if any
    out->writeBool(false);

    // Write texture coord arrays
    Geometry::ArrayList& tcal = getTexCoordArrayList();
    out->writeInt(tcal.size());
    unsigned int j;
    for(j=0;j<tcal.size();j++)
    {
        // Write coords if valid
        out->writeBool(tcal[j].valid());
        if (tcal[j].valid()){
            out->writeArray(tcal[j].get());
        }

        // Write indices if valid
        out->writeBool(false);
    }

    // Write vertex attributes
    Geometry::ArrayList& vaal = getVertexAttribArrayList();
    out->writeInt(vaal.size());
    for(j=0;j<vaal.size();j++)
    {
        // Write coords if valid
        const osg::Array* array = vaal[j].get();
        if (array)
        {
            out->writeBinding(static_cast<osg::Array::Binding>(array->getBinding()));
            out->writeBool(array->getNormalize());
            out->writeBool(true);
            out->writeArray(array);

            // Write indices if valid
            out->writeBool(false);
        }
        else
        {
            out->writeBinding(osg::Array::BIND_OFF);
            out->writeBool(false);
            out->writeBool(false);
            out->writeBool(false);
        }
    }
}

void Geometry::read(DataInputStream* in)
{
    // Read Geometry's identification.
    int id = in->peekInt();
    if(id == IVEGEOMETRY){
        // Code to read Geometry's properties.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Drawable*  drawable = dynamic_cast<osg::Drawable*>(this);
        if(drawable){
            ((ive::Drawable*)(drawable))->read(in);
        }
        else
            in_THROW_EXCEPTION("Geometry::read(): Could not cast this osg::Geometry to an osg::Drawable.");


        // Read geometry properties

        // Read primitiveset list.
        int size = in->readInt();
        int i;
        for(i=0;i<size;i++){
            osg::PrimitiveSet* prim;
            int primID = in->peekInt();
            if(primID==IVEDRAWARRAYS){
                prim = new osg::DrawArrays();
                ((ive::DrawArrays*)(prim))->read(in);
                addPrimitiveSet(prim);
            }
            else if(primID==IVEDRAWARRAYLENGTHS){
                prim = new osg::DrawArrayLengths();
                ((ive::DrawArrayLengths*)(prim))->read(in);
                addPrimitiveSet(prim);
            }
            else if(primID==IVEDRAWELEMENTSUBYTE){
                prim = new osg::DrawElementsUByte();
                ((ive::DrawElementsUByte*)(prim))->read(in);
                addPrimitiveSet(prim);
            }
            else if(primID==IVEDRAWELEMENTSUSHORT){
                prim = new osg::DrawElementsUShort();
                ((ive::DrawElementsUShort*)(prim))->read(in);
                addPrimitiveSet(prim);
            }
            else if(primID==IVEDRAWELEMENTSUINT){
                prim = new osg::DrawElementsUInt();
                ((ive::DrawElementsUInt*)(prim))->read(in);
                addPrimitiveSet(prim);
            }
            else{
                in_THROW_EXCEPTION("Unknown PrimitiveSet in Geometry::read()");
            }
        }

        // Read vertex array if any
        bool va=in->readBool();
        if (va)
        {
            setVertexArray(in->readArray());
        }
        // Read vertex indices if any
        if (in->readBool())
        {
            osg::ref_ptr<osg::IndexArray> indices = (static_cast<osg::IndexArray*>(in->readArray()));
            if (indices.valid() && getVertexArray()) getVertexArray()->setUserData(indices.get());
        }

        // Read normal array if any
        if ( in->getVersion() < VERSION_0013 )
        {
            if(in->readBool())
            {
                osg::Array::Binding binding = in->readBinding();
                setNormalArray(in->readVec3Array(), binding);
            }
        }
        else
        {
            if(in->readBool()){
                osg::Array::Binding binding = in->readBinding();
                setNormalArray(in->readArray(), binding);
            }
        }

        // Read normal indices if any
        if (in->readBool())
        {
            osg::ref_ptr<osg::IndexArray> indices = static_cast<osg::IndexArray*>(in->readArray());
            if (indices.valid() && getNormalArray()) getNormalArray()->setUserData(indices.get());
        }

        // Read color array if any.
        if(in->readBool())
        {
            osg::Array::Binding binding = in->readBinding();
            setColorArray(in->readArray(), binding);
        }
        // Read color indices if any
        if(in->readBool())
        {
            osg::ref_ptr<osg::IndexArray> indices = (static_cast<osg::IndexArray*>(in->readArray()));
            if (indices.valid() && getColorArray()) getColorArray()->setUserData(indices.get());
        }

        // Read secondary color array if any
        if(in->readBool()){
            osg::Array::Binding binding = in->readBinding();
            setSecondaryColorArray(in->readArray(), binding);
        }
        // Read second color indices if any
        if(in->readBool())
        {
            osg::ref_ptr<osg::IndexArray> indices = (static_cast<osg::IndexArray*>(in->readArray()));
            if (indices.valid() && getSecondaryColorArray()) getSecondaryColorArray()->setUserData(indices.get());
        }

        // Read fog coord array if any
        if(in->readBool()){
            osg::Array::Binding binding = in->readBinding();
            setFogCoordArray(in->readArray(), binding);
        }
        // Read fog coord indices if any
        if(in->readBool())
        {
            osg::ref_ptr<osg::IndexArray> indices = (static_cast<osg::IndexArray*>(in->readArray()));
            if (indices && getFogCoordArray()) getFogCoordArray()->setUserData(indices.get());
        }

        // Read texture coord arrays
        size = in->readInt();
        for(i =0;i<size;i++)
        {
            // Read coords if valid
            bool coords_valid = in->readBool();
            if(coords_valid)
                setTexCoordArray(i, in->readArray());
            // Read Indices if valid
            if(in->readBool())
            {
                osg::ref_ptr<osg::IndexArray> indices = (static_cast<osg::IndexArray*>(in->readArray()));
                if (indices && getTexCoordArray(i)) getTexCoordArray(i)->setUserData(indices.get());
            }
        }

        // Read vertex attrib arrays
        size = in->readInt();
        for(i =0;i<size;i++)
        {
            osg::Array::Binding binding = in->readBinding();
            bool normalize = in->readBool();

            // Read coords if valid
            bool coords_valid = in->readBool();
            if(coords_valid) {
                setVertexAttribArray(i, in->readArray(), binding);
                setVertexAttribNormalize(i,normalize);
            }

            // Read Indices if valid
            if(in->readBool())
            {
                osg::ref_ptr<osg::IndexArray> indices = (static_cast<osg::IndexArray*>(in->readArray()));
                if (indices && getVertexAttribArray(i)) getVertexAttribArray(i)->setUserData(indices.get());
            }
        }

    }
    else{
        in_THROW_EXCEPTION("Geometry::read(): Expected Geometry identification.");
    }
}