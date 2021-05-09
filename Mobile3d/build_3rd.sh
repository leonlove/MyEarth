#! /bin/bash
echo "build 3rd ......"
echo $ANDROID_NDK_HOME
pwd
dir_root=`pwd`

#x86 x86_64 armeabi-v7a arm64
APP_ABI=armeabi-v7a
# OpenSSL_ABI=arm
APP_LEVEL=android-21

dir_build=$dir_root/build
dir_3rd=$dir_root/3rd

#提前安装目录
dir_3rd_out=$dir_root/3rd/pre_install/$APP_ABI

build_type='Release'

#编译OpenSceneGraph-3.6.5
if [ -f $dir_3rd_out/lib/libosg.a ]; then
	echo "Use a compiled library:libosg.a"
else
	cd $dir_3rd/OpenSceneGraph-3.6.5
	if [ ! -d "build" ]; then
		mkdir build
	fi
	cd build
	rm -rf *
	cmake -DCMAKE_INSTALL_PREFIX=$dir_3rd_out \
		  -DANDROID_NDK=$ANDROID_NDK_HOME \
		  -DCMAKE_BUILD_TYPE=$build_type \
		  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
		  -DANDROID_TOOLCHAIN=clang \
		  -DANDROID_PLATFORM=$APP_LEVEL \
		  -DANDROID_ABI=$APP_ABI \
		  -DANDROID_STL=c++_static \
		  -DCMAKE_CXX_FLAGS="-std=c++11 -frtti -fexceptions -stdlib=libc++ -DUSE_ZLIB" \
		  -DOPENGL_PROFILE="GLES2" \
		  -DDYNAMIC_OPENTHREADS=OFF \
		  -DDYNAMIC_OPENSCENEGRAPH=OFF \
		  .. 
	make -j4 && make install
	if [ $? -eq 0 ]; then	
		echo " .........OpenSceneGraph-3.6.5 OK........"
	else
		echo "..........OpenSceneGraph-3.6.5 Error........."
		exit -1
	fi
fi