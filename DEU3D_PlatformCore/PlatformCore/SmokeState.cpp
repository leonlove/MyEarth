#include "SmokeState.h"

#include "Registry.h"

#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osgParticle/PointPlacer>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/RadialShooter>
#include <osgDB/ReadFile>
#include <osg/CoordinateSystemNode>
#include <osg/MatrixTransform>

#include "AddOrRemove_Operation.h"

SmokeState::SmokeState(const std::string &strName) : StateBase(strName)
{
    m_dblSize = 10.0;
    m_strSmokeImageFile = cmm::genResourceFileDir() + "smoke.png";
    m_dblPitch = osg::PI;
    m_dblAzimuth = 0.0;
}

SmokeState::SmokeState(const std::string &strName, SceneGraphOperator *pSceneGraphOperator) : StateBase(strName)
{
    m_dblSize = 10.0;
    m_strSmokeImageFile =  cmm::genResourceFileDir() + "smoke.png";
    m_dblPitch = osg::PI;
    m_dblAzimuth = 0.0;
    m_pSceneGraphOperator = pSceneGraphOperator;
}


SmokeState::~SmokeState(void)
{
}

void SmokeState::setSmokeImageFile(const std::string &strSmokeFile)
{
    m_strSmokeImageFile = strSmokeFile;
}

void SmokeState::setSmokeSize(double dblSize)
{
    m_dblSize = dblSize;
}

const cmm::FloatColor SmokeState::getSmokeColor() const
{
    cmm::FloatColor clr;
    clr.m_fltR = m_color[0];
    clr.m_fltG = m_color[1];
    clr.m_fltB = m_color[2];
    clr.m_fltA = m_color[3];

    return clr;
}

void SmokeState::setSmokeColor(const cmm::FloatColor &clr)
{
    m_color.set(clr.m_fltR, clr.m_fltG, clr.m_fltB, clr.m_fltA);

    //m_pSmokeImage = osgDB::readImageFile(m_strSmokeImageFile);
    //unsigned char *pData = m_pSmokeImage->data();

    //for(int i = 0; i < m_pSmokeImage->s() * m_pSmokeImage->t(); i++)
    //{
    //    pData[i * 4 + 0] = m_color[0] * 255;
    //    pData[i * 4 + 1] = m_color[1] * 255;
    //    pData[i * 4 + 2] = m_color[2] * 255;
    //}
}

void SmokeState::setSmokeDirection(double dblAzimuth, double dblPitch)
{

}

osg::Node *SmokeState::createEffect(void)
{
    osg::ref_ptr<osgParticle::ParticleSystem> pParticleSystem1 = new osgParticle::ParticleSystem;
    pParticleSystem1->setFreezeOnCull(true);
    pParticleSystem1->setDefaultAttributes(m_strSmokeImageFile, true, false);
    pParticleSystem1->getStateSet()->setRenderBinDetails(1, "DepthSortedBin");

    osg::ref_ptr<osgParticle::ModularEmitter> pModularEmitter1 = new osgParticle::ModularEmitter;
    pModularEmitter1->setParticleSystem(pParticleSystem1);

    osgParticle::Particle Particle1;
    Particle1.setLifeTime(1.5);
    Particle1.setSizeRange(osgParticle::rangef(1.0f, 3.0f));
    Particle1.setColorRange(osgParticle::rangev4(osg::Vec4(1.0f, 0.5f, 0.0f, 1.0f), osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    pModularEmitter1->setParticleTemplate(Particle1);

    osg::ref_ptr<osgParticle::RandomRateCounter> pRandomRateCounter1 = new osgParticle::RandomRateCounter;
    pRandomRateCounter1->setRateRange(30.0f, 50.0f);
    pModularEmitter1->setCounter(pRandomRateCounter1);

    osg::ref_ptr<osgParticle::PointPlacer> pPointPlacer1 = new osgParticle::PointPlacer;
    pPointPlacer1->setCenter(0.0f, 0.0f, 0.0f);
    pModularEmitter1->setPlacer(pPointPlacer1);

    osg::ref_ptr<osgParticle::RadialShooter> pRadialShooter1 = new osgParticle::RadialShooter;
    pRadialShooter1->setThetaRange(-osg::PI_4, osg::PI_4);
    pRadialShooter1->setPhiRange(-osg::PI_4, osg::PI_4);
    pRadialShooter1->setInitialSpeedRange(5.0f, 7.5f);
    pModularEmitter1->setShooter(pRadialShooter1);

    osg::ref_ptr<osgParticle::ParticleSystem> pParticleSystem2 = new osgParticle::ParticleSystem;
    pParticleSystem2->setFreezeOnCull(true);
    pParticleSystem2->setDefaultAttributes(m_strSmokeImageFile, false, false);
    pParticleSystem2->getStateSet()->setRenderBinDetails(0, "DepthSortedBin");

    osg::ref_ptr<osgParticle::ModularEmitter> pModularEmitter2 = new osgParticle::ModularEmitter;
    pModularEmitter2->setParticleSystem(pParticleSystem2);

    osgParticle::Particle Particle2;
    Particle2.setLifeTime(3.0);
    Particle2.setSizeRange(osgParticle::rangef(1.0f, 12.0f));
    Particle2.setColorRange(osgParticle::rangev4(osg::Vec4(0.1f, 0.1f, 0.1f, 0.5f), osg::Vec4(1.0f, 1.0f, 1.0f, 1.5f)));
    pModularEmitter2->setParticleTemplate(Particle2);

    osg::ref_ptr<osgParticle::RandomRateCounter> pRandomRateCounter2 = new osgParticle::RandomRateCounter;
    pRandomRateCounter2->setRateRange(30.0f, 50.0f);
    pModularEmitter2->setCounter(pRandomRateCounter2);

    osg::ref_ptr<osgParticle::PointPlacer> pPointPlacer2 = new osgParticle::PointPlacer;
    pPointPlacer2->setCenter(0.0f, 0.0f, 1.0f);
    pModularEmitter2->setPlacer(pPointPlacer2);

    osg::ref_ptr<osgParticle::RadialShooter> pRadialShooter2 = new osgParticle::RadialShooter;
    pRadialShooter2->setThetaRange(-osg::PI_4, osg::PI_4);
    pRadialShooter2->setPhiRange(-osg::PI_4, osg::PI_4);
    pRadialShooter2->setInitialSpeedRange(10.0f, 15.0f);
    pModularEmitter2->setShooter(pRadialShooter2);

    osg::ref_ptr<osgParticle::ParticleSystemUpdater> pParticleSystemUpdater = new osgParticle::ParticleSystemUpdater;
    pParticleSystemUpdater->addParticleSystem(pParticleSystem1);
    pParticleSystemUpdater->addParticleSystem(pParticleSystem2);

    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
    pGeode->addDrawable(pParticleSystem1);
    pGeode->addDrawable(pParticleSystem2);

    osg::ref_ptr<osg::Group> pGroup1 = new osg::Group;
    pGroup1->addChild(pModularEmitter1);
    pGroup1->addChild(pModularEmitter2);

    osg::ref_ptr<osg::Group> pGroup2 = new osg::Group;
    pGroup2->addChild(pGroup1);
    pGroup2->addChild(pParticleSystemUpdater);
    pGroup2->addChild(pGeode);

    return pGroup2.release();
}

void SmokeState::getSmokeDirection(double &dblAzimuth, double &dblPitch)
{

}

bool SmokeState::applyState(osg::Node *pNode, bool bApply)
{
    if(pNode == NULL)
    {
        return false;
    }

    ID id = pNode->getID();

    osg::Group *pGroup = pNode->asGroup();
    if(pGroup == NULL)
    {
        return false;
    }

    //临时解决烟雾效果，以后会删除
    ID effect_id = id;
    effect_id.ObjectID.m_nType = 0xFF;
    //

    if(bApply)
    {
        osg::BoundingSphere bs = pGroup->getBound();
        osg::EllipsoidModel *pEllipsoidModel = osg::EllipsoidModel::instance();
        osg::Matrix matrix;
        pEllipsoidModel->computeLocalToWorldTransformFromXYZ(bs._center[0], bs._center[1], bs._center[2], matrix);
        matrix.preMultScale(osg::Vec3(m_dblSize / 50.0f, m_dblSize / 50.0f, m_dblSize / 50.0f));
        osg::ref_ptr<osg::MatrixTransform> pMatrixTransform = new osg::MatrixTransform(matrix);

        osg::ref_ptr<osg::Node> pEffect = createEffect();
        pMatrixTransform->addChild(pEffect);

        pMatrixTransform->setID(effect_id);
        OpenSP::sp<AddTempModelByNode_Operation> pAddOperation = new AddTempModelByNode_Operation(pMatrixTransform.get());
        m_pSceneGraphOperator->pushOperation(pAddOperation);
    }
    else
    {
        OpenSP::sp<RemoveTempModelByID_Operation> pRemoveOperation = new RemoveTempModelByID_Operation(effect_id);
        m_pSceneGraphOperator->pushOperation(pRemoveOperation);
    }

    return true;
}