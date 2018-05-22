#ifndef STATE_MANAGER_H_ED61AA2A_E455_4825_B3F5_B662C3D32589_INCLUDE
#define STATE_MANAGER_H_ED61AA2A_E455_4825_B3F5_B662C3D32589_INCLUDE

#include "IStateManager.h"
#include <map>
#include <OpenSP/sp.h>
#include <OpenThreads/Mutex>
//��Ϊ��ʱ��ӽ�������ڴ�����ģ�����ʽ�������Ҫɾ��
#include "SceneGraphOperator.h"
//

class StateManager : public IStateManager
{
public:
    StateManager(void);
    //��Ϊ��ʱ��ӽ�������ڴ�����ģ�����ʽ�������Ҫɾ��
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

    //��Ϊ��ʱ��ӽ�������ڴ�����ģ�����ʽ�������Ҫɾ��
    SceneGraphOperator *m_pSceneGraphOperator;
    //
};

#endif
