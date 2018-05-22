#ifndef SELECTING_OPERATOR_H_48D3B5A2_A01B_4875_B253_5C977B186478_INCLUDE
#define SELECTING_OPERATOR_H_48D3B5A2_A01B_4875_B253_5C977B186478_INCLUDE

#include "SceneGraphOperationBase.h"
#include <OpenThreads/Block>
#include <osgViewer/View>

class Selecting_Operation : public SceneGraphOperationBase
{
public:
    explicit Selecting_Operation(void) : m_eSelectingMode(SM_Point){}
    virtual ~Selecting_Operation(void) {}

public:
    void    selectByScreenPoint(osgViewer::View *pView, const osg::Vec2s &point);
    void    selectByScreenRect(osgViewer::View *pView, const osg::Vec2s &ptLeftTop, const osg::Vec2s &ptRightBottom);

    void    waitForFinishing(IDList &listIDs, osg::Vec3d &minIntersectPoint);

protected:
    void    doScreenPointSelecting(osg::Node *pTargetNode);
    void    doScreenRectSelecting(osg::Node *pTargetNode);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    enum SelectingMode
    {
        SM_Point,
        SM_Rect,
    };
    SelectingMode           m_eSelectingMode;

	osg::Vec3d				m_minIntersectPoint;

    std::vector<osg::Vec2s>         m_vecScreenSelectingCoord;
    osg::ref_ptr<osgViewer::View>   m_pTargetView;

    IDList                  m_listResult;
    OpenThreads::Block      m_blockFinished;
};


#endif

