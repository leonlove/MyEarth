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
#include <stdlib.h>
#include <string.h>

#include <osg/CullSettings>

#include <osg/Notify>

using namespace osg;

CullSettings::CullSettings(const CullSettings& cs)
{
    setCullSettings(cs);
}

void CullSettings::setDefaults()
{
    _inheritanceMask = ALL_VARIABLES;
    _inheritanceMaskActionOnAttributeSetting = DISABLE_ASSOCIATED_INHERITANCE_MASK_BIT;
    _cullingMode = DEFAULT_CULLING;
    _smallFeatureCullingPixelSize = 2.0f;

    _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
    _nearFarRatio = 0.0005;
    _impostorActive = true;
    _depthSortImpostorSprites = false;
    _impostorPixelErrorThreshold = 4.0f;
    _numFramesToKeepImpostorSprites = 10;
    _cullMask = 0xffffffff;
    _cullMaskLeft = 0xffffffff;
    _cullMaskRight = 0xffffffff;
}

void CullSettings::setCullSettings(const CullSettings& rhs)
{
    _inheritanceMask = rhs._inheritanceMask;
    _inheritanceMaskActionOnAttributeSetting = rhs._inheritanceMaskActionOnAttributeSetting;

    _computeNearFar = rhs._computeNearFar;
    _cullingMode = rhs._cullingMode;
    _smallFeatureCullingPixelSize = rhs._smallFeatureCullingPixelSize;

    _clampProjectionMatrixCallback = rhs._clampProjectionMatrixCallback;
    _nearFarRatio = rhs._nearFarRatio;
    _impostorActive = rhs._impostorActive;
    _depthSortImpostorSprites = rhs._depthSortImpostorSprites;
    _impostorPixelErrorThreshold = rhs._impostorPixelErrorThreshold;
    _numFramesToKeepImpostorSprites = rhs._numFramesToKeepImpostorSprites;    

    _cullMask = rhs._cullMask;
    _cullMaskLeft = rhs._cullMaskLeft;
    _cullMaskRight =  rhs._cullMaskRight;
}


void CullSettings::inheritCullSettings(const CullSettings& settings, unsigned int inheritanceMask)
{
    if (inheritanceMask & COMPUTE_NEAR_FAR_MODE) _computeNearFar = settings._computeNearFar;
    if (inheritanceMask & NEAR_FAR_RATIO) _nearFarRatio = settings._nearFarRatio;
    if (inheritanceMask & IMPOSTOR_ACTIVE) _impostorActive = settings._impostorActive;
    if (inheritanceMask & DEPTH_SORT_IMPOSTOR_SPRITES) _depthSortImpostorSprites = settings._depthSortImpostorSprites;
    if (inheritanceMask & IMPOSTOR_PIXEL_ERROR_THRESHOLD) _impostorPixelErrorThreshold = settings._impostorPixelErrorThreshold;
    if (inheritanceMask & NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES) _numFramesToKeepImpostorSprites = settings._numFramesToKeepImpostorSprites;
    if (inheritanceMask & CULL_MASK) _cullMask = settings._cullMask;
    if (inheritanceMask & CULL_MASK_LEFT) _cullMaskLeft = settings._cullMaskLeft;
    if (inheritanceMask & CULL_MASK_RIGHT) _cullMaskRight = settings._cullMaskRight;
    if (inheritanceMask & CULLING_MODE) _cullingMode = settings._cullingMode;
    if (inheritanceMask & SMALL_FEATURE_CULLING_PIXEL_SIZE) _smallFeatureCullingPixelSize = settings._smallFeatureCullingPixelSize;
    if (inheritanceMask & CLAMP_PROJECTION_MATRIX_CALLBACK) _clampProjectionMatrixCallback = settings._clampProjectionMatrixCallback;
}


