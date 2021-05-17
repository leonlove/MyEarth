#include <string.h>
#include <jni.h>
#include <android/log.h>

#include <iostream>

#include "OsgMainApp.hpp"

OsgMainApp mainApp;

//extern "C" {
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_init(JNIEnv * env, jobject obj, jint width, jint height);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_step(JNIEnv * env, jobject obj);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_clearContents(JNIEnv * env, jobject obj);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_mouseButtonPressEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_mouseButtonReleaseEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_mouseMoveEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_keyboardDown(JNIEnv * env, jobject obj, jint key);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_keyboardUp(JNIEnv * env, jobject obj, jint key);
//JNIEXPORT jintArray JNICALL Java_com_example_osgtestgles2_osgNativeLib_getClearColor(JNIEnv * env, jobject obj);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_setClearColor(JNIEnv * env, jobject obj, jint red, jint green, jint blue);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_loadObject(JNIEnv * env, jobject obj, jstring address);
//JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_unLoadObject(JNIEnv * env, jobject obj, jint number);
//JNIEXPORT jobjectArray JNICALL Java_com_example_osgtestgles2_osgNativeLib_getObjectNames(JNIEnv * env, jobject obj);
//};

extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_init(JNIEnv * env, jobject obj, jint width, jint height){
    mainApp.initOsgWindow(0,0,width,height);
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_step(JNIEnv * env, jobject obj){
    mainApp.draw();
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_clearContents(JNIEnv * env, jobject obj){
    mainApp.clearScene();
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_mouseButtonPressEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button){
    mainApp.mouseButtonPressEvent(x,y,button);
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_mouseButtonReleaseEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint button){
    mainApp.mouseButtonReleaseEvent(x,y,button);
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_mouseMoveEvent(JNIEnv * env, jobject obj, jfloat x, jfloat y){
    mainApp.mouseMoveEvent(x,y);
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_keyboardDown(JNIEnv * env, jobject obj, jint key){
    mainApp.keyboardDown(key);
}
extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_keyboardUp(JNIEnv * env, jobject obj, jint key){
    mainApp.keyboardUp(key);
}

extern "C" JNIEXPORT jintArray JNICALL Java_com_example_osgtestgles2_osgNativeLib_getClearColor(JNIEnv * env, jobject obj){

    jintArray color;
    color = env->NewIntArray(3);
    if (color == NULL) {
        return NULL;
    }
    osg::Vec4 vTemp1 = mainApp.getClearColor();

    jint vTemp2[3];

    vTemp2[0] = (int) (vTemp1.r() * 255);
    vTemp2[1] = (int) (vTemp1.g() * 255);
    vTemp2[2] = (int) (vTemp1.b() * 255);

    std::cout<<vTemp2[0]<<" "<<vTemp2[1]<<" "<<vTemp2[2]<<" "<<std::endl;

    env->SetIntArrayRegion(color, 0, 3, vTemp2);

    return color;
}

extern "C"  JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_setClearColor(JNIEnv * env, jobject obj, jint red, jint green, jint blue){
    osg::Vec4 tVec((float) red / 255.0f, (float) green / 255.0f, (float) blue / 255.0f, 0.0f);
    mainApp.setClearColor(tVec);
}

extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_loadObject(JNIEnv * env, jobject obj, jstring address){
    //Import Strings from JNI
    const char *nativeAddress = env->GetStringUTFChars(address, JNI_FALSE);

    mainApp.loadObject(std::string(nativeAddress));

    //Release Strings to JNI
    env->ReleaseStringUTFChars(address, nativeAddress);
}

extern "C" JNIEXPORT void JNICALL Java_com_example_osgtestgles2_osgNativeLib_unLoadObject(JNIEnv * env, jobject obj, jint number){
    mainApp.unLoadObject(number);
}

extern "C" JNIEXPORT jobjectArray JNICALL Java_com_example_osgtestgles2_osgNativeLib_getObjectNames(JNIEnv * env, jobject obj){

    jobjectArray fileNames;
    unsigned int numModels = mainApp.getNumberObjects();
    fileNames = (jobjectArray)env->NewObjectArray(numModels,env->FindClass("java/lang/String"),env->NewStringUTF(""));

    for(unsigned int i=0;i < numModels;i++){
        std::string name = mainApp.getObjectName(i);
        env->SetObjectArrayElement(fileNames,i,env->NewStringUTF(name.c_str()));
    }

    return fileNames;
}