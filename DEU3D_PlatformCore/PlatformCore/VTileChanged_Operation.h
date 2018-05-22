#ifndef V_TILE_CHANGED_OPERATION_H_61C1FBC8_B334_4F7F_99BB_3A5C72BF76B2_INCLUDE
#define V_TILE_CHANGED_OPERATION_H_61C1FBC8_B334_4F7F_99BB_3A5C72BF76B2_INCLUDE

#include "SceneGraphOperationBase.h"
#include "DEUProxyNode.h"

namespace vtm
{
    class IVirtualTile;
}

class FileReadInterceptor;

class VTileChanged_Operation : public SceneGraphOperationBase
{
public:
    explicit VTileChanged_Operation(unsigned nFragCount)
        : m_nFragmentCount(nFragCount)
    {
    }

public:
    void    addTile(const ID &idVTile, const vtm::IVirtualTile *pVTile);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    void doMerge(osg::Node *pVTileNode, const vtm::IVirtualTile *pVTile);

protected:
    std::map<ID, OpenSP::sp<const vtm::IVirtualTile> >    m_mapChangedTiles;

    const unsigned          m_nFragmentCount;
};



#endif
