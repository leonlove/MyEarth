#ifndef STATE_CHANGED_OPERATION_H_68CD0001_B14F_4E45_ABD7_B4EF833FF12D_INCLUDE
#define STATE_CHANGED_OPERATION_H_68CD0001_B14F_4E45_ABD7_B4EF833FF12D_INCLUDE

#include "SceneGraphOperationBase.h"
#include <string>
#include <set>
#include "StateManager.h"

class StateChanged_Operation : public SceneGraphOperationBase
{
public:
    explicit StateChanged_Operation(void);
    virtual ~StateChanged_Operation(void);

public:
    void    setState(StateManager *pStateManager, const std::string &strState, bool bState);
    void    setChangedIDs(const IDList &listIDs);

protected:
    void applyVisibleState(osg::Node *pCultureRootNode);
    void applyNonVisibleState(osg::Node *pCultureRootNode);

protected:
    virtual bool doAction(SceneGraphOperator *pOperator);

protected:
    std::string     m_strState;
    bool            m_bState;
    std::set<ID>    m_listIDs;
    OpenSP::sp<StateManager>    m_pStateManager;
};


#endif
