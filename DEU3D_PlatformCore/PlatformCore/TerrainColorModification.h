#ifndef TERRAIN_COLOR_MODIFICATION_H_796261C6_6176_48F3_8F1A_0DEED59D69D1_INCLUDE
#define TERRAIN_COLOR_MODIFICATION_H_796261C6_6176_48F3_8F1A_0DEED59D69D1_INCLUDE

#include "ITerrainColorModification.h"
#include "TerrainModification.h"

class TerrainColorModification : public ITerrainColorModification, virtual public TerrainModification
{
public:
    explicit TerrainColorModification(const std::string &strType, ea::IEventAdapter *pEventAdapter);
    virtual ~TerrainColorModification(void);

protected:
    virtual void  setColor(const cmm::FloatColor &color)    {   if(isApply())   return; m_color = color;    }
    virtual const cmm::FloatColor &getColor(void) const     {   return m_color;     }

    virtual bool    modifyTerrainTile(osg::Node *pTerrainTile) const;

protected:
    cmm::FloatColor     m_color;
};


#endif
