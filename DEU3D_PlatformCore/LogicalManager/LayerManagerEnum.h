#ifndef LAYER_MANAGER_ENUM_H_2419B677_51EF_4F00_9660_EB1BE1FFEA11_INCLUDE
#define LAYER_MANAGER_ENUM_H_2419B677_51EF_4F00_9660_EB1BE1FFEA11_INCLUDE


namespace logical
{

typedef enum tagObjectSourceType
{
    OST_UNKNOWN,
    OST_REMOTE,
    OST_LOCAL
}ObjectSourceType;

typedef enum tagObjectType
{
    OT_UNKNOWN  = 0,
    OT_INSTANCE,
    OT_LAYER
}ObjectType;

}

#endif