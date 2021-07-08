package com.example.osgtest1;

public class osgNativeLib
{
    static
    {
        System.loadLibrary("native-lib");
    }

    public static native void init(int width, int height);
    // Rendering.
    public static native void step();
    // Resources.
    public static native void loadModel(String modelPath);
    // Input.
    public static native void moveMouse(float x, float y);
    public static native void pressMouse(boolean down, float x, float y);
}

