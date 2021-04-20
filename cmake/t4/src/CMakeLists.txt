
INCLUDE_DIRECTORIES(/home/workspace/MyEarth/cmake/t3/pre_install/include/hello/)

LINK_DIRECTORIES(/home/workspace/MyEarth/cmake/t3/pre_install/lib)

ADD_EXECUTABLE(main main.c)

TARGET_LINK_LIBRARIES(main hello)