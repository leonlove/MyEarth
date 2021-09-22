#pragma once

#include    "CELLMath.hpp"
namespace   CELL
{
    class   CELLResourceMgr;
    class   CELLOpenGL;
    class   CELLContext
    {
    public:
        CELLOpenGL*         _device;
        /// 资源管理者指针
        CELLResourceMgr*    _resMgr;
        /// 鼠标的当前位置X
        int         _mouseX;
        /// 鼠标的当前位置Y
        int         _mouseY;

        /// 窗口的大小
        int         _width;
        int         _height;
        /// 2d 屏幕投影矩阵
        matrix4r    _screenPrj;
    };
}
