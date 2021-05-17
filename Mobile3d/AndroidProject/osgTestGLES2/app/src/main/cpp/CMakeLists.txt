# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.

project("osgtestgles2")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp
        OsgAndroidNotifyHandler.cpp
        OsgMainApp.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

SET(3rdpath "${CMAKE_SOURCE_DIR}/../../../../../../3rd/pre_install/")

INCLUDE_DIRECTORIES(${3rdpath}/armeabi-v7a/include)
#MESSAGE(FATAL_ERROR "3rdpath:${3rdpath}")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${3rdpath}/armeabi-v7a/lib -L${3rdpath}/armeabi-v7a/lib/osgPlugins-3.6.5")
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                    GLESv2
                    z
                    osgdb_dds
                    osgdb_openflight
                    osgdb_tga
                    osgdb_rgb
                    osgdb_osgterrain
                    osgdb_osg
                    osgdb_ive
                    osgdb_deprecated_osgviewer
                    osgdb_deprecated_osgvolume
                    osgdb_deprecated_osgtext
                    osgdb_deprecated_osgterrain
                    osgdb_deprecated_osgsim
                    osgdb_deprecated_osgshadow
                    osgdb_deprecated_osgparticle
                    osgdb_deprecated_osgfx
                    osgdb_deprecated_osganimation
                    osgdb_deprecated_osg
                    osgdb_serializers_osgvolume
                    osgdb_serializers_osgtext
                    osgdb_serializers_osgterrain
                    osgdb_serializers_osgsim
                    osgdb_serializers_osgshadow
                    osgdb_serializers_osgparticle
                    osgdb_serializers_osgmanipulator
                    osgdb_serializers_osgfx
                    osgdb_serializers_osganimation
                    osgdb_serializers_osg
                    osgViewer
                    osgVolume
                    osgTerrain
                    osgText
                    osgShadow
                    osgSim
                    osgParticle
                    osgManipulator
                    osgGA
                    osgFX
                    osgDB
                    osgAnimation
                    osgUtil
                    osg
                    OpenThreads
        )