#ifndef V_CUBE_CHANGED_OPERATION_H_61C1FBC8_B334_4F7F_99BB_3A5C72BF76B2_INCLUDE
#define V_CUBE_CHANGED_OPERATION_H_61C1FBC8_B334_4F7F_99BB_3A5C72BF76B2_INCLUDE

#include "SceneGraphOperationBase.h"

namespace vcm
{
    class IVirtualCube;
}

class FileReadInterceptor;

class VCubeChanged_Operation : public SceneGraphOperationBase
{
public:
    explicit VCubeChanged_Operation(unsigned nFragCount)
        : m_nFragmentCount(nFragCount)
    {
    }

public:
    void    addCube(const ID &idVCube, const vcm::IVirtualCube *pVCube);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    void doMerge(osg::Node *pVCubNode, const vcm::IVirtualCube *pVCube);

protected:
    std::map<ID, OpenSP::sp<const vcm::IVirtualCube> >    m_mapChangedCubes;

    const unsigned          m_nFragmentCount;
};



#endif
