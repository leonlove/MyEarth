#ifndef STATE_MANAGER_H_ED61AA2A_E455_4825_B3F5_B662C3D32589_INCLUDE
#define STATE_MANAGER_H_ED61AA2A_E455_4825_B3F5_B662C3D32589_INCLUDE

#include "IStateManager.h"
#include <map>
#include <OpenSP/sp.h>
#include <OpenThreads/Mutex>
//此为零时添加解决烟雾内存问题的，待正式解决后需要删除
#include "SceneGraphOperator.h"
//

class StateManager : public IStateManager
{
public:
    StateManager(void);
    //此为零时添加解决烟雾内存问题的，待正式解决后需要删除
    StateManager(SceneGraphOperator *pSceneGraphOperator);
    //
    ~StateManager(void);

public:
    virtual IState               *createRenderState(const std::string &strTypeName, const std::string &strStateName);
    virtual IState               *getRenderState(const std::string &strStateName);
    virtual std::vector<IState *> getRenderStateList(const std::string &strTypeName);

protected:
    std::map<std::string, OpenSP::sp<IState> > m_mapStates;
    OpenThreads::Mutex      m_mtxStates;

    //此为零时添加解决烟雾内存问题的，待正式解决后需要删除
    SceneGraphOperator *m_pSceneGraphOperator;
    //
};

#endif
