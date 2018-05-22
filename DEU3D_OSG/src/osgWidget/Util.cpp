// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <stdlib.h>

#include <osg/io_utils>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Util>
#include <osgWidget/ViewerEventHandlers>
#include <osgWidget/WindowManager>

namespace osgWidget {

std::string getFilePath(const std::string& filename) {
    osgDB::FilePathList path;

    char* fp = getenv("OSGWIDGET_FILE_PATH");
    
    osgDB::convertStringPathIntoFilePathList(fp ? fp : ".", path);

    return osgDB::findFileInPath(filename, path);
}

std::string generateRandomName(const std::string& base) {
    static unsigned int count = 0;
    
    std::stringstream ss;

    ss << base << "_" << count;
    
    count++;

    return ss.str();
}

osg::Camera* createOrthoCamera(matrix_type width, matrix_type height) {
    osg::Camera* camera = new osg::Camera();

    camera->getOrCreateStateSet()->setMode(
        GL_LIGHTING,
        osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
    );

    camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width, 0.0f, height));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    
    return camera;
}

bool writeWindowManagerNode(WindowManager* wm) {
    osgDB::writeNodeFile(*wm->getParent(0), "osgWidget.osg");

    return true;
}

}
