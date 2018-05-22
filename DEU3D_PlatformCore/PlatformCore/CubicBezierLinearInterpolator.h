#ifndef COS_INTERPOLATOR_H_DFC19C49_33B6_480B_81E7_0A048C503311_INCLUDE
#define COS_INTERPOLATOR_H_DFC19C49_33B6_480B_81E7_0A048C503311_INCLUDE

#include <osgAnimation/Interpolator>
#include <osgAnimation/Sampler>
#include <osgAnimation/Channel>

namespace osgAnimation
{
    template <class TYPE, class KEY = TYPE>
    class TemplateCubicBezierLinearInterpolator : public osgAnimation::TemplateCubicBezierInterpolator<TYPE, KEY>
    {
    public:
        TemplateCubicBezierLinearInterpolator(void)
        {
        }

        void getValue(const osgAnimation::TemplateKeyframeContainer<KEY> &keyframes, double dblTime, TYPE& result) const 
        {
            if(dblTime >= keyframes.back().getTime())
            {
                result = keyframes.back().getValue().getPosition();
                return;
            }
            else if(dblTime <= keyframes.front().getTime())
            {
                result = keyframes.front().getValue().getPosition();
                return;
            }

            const int    i = getKeyIndexFromTime(keyframes, dblTime);
            const double t = (dblTime - keyframes[i].getTime()) / ( keyframes[i+1].getTime() -  keyframes[i].getTime());
            const double one_minus_t = 1.0 - t;
            const double one_minus_t2 = one_minus_t * one_minus_t;
            const double one_minus_t3 = one_minus_t2 * one_minus_t;
            const double t2 = t * t;
            const double t3 = t2 * t;

            const double v0 = 0.0 * one_minus_t3;
            const double v1 = keyframes[i].getValue().getControlPointIn() * (3.0 * t * one_minus_t2);
            const double v2 = keyframes[i].getValue().getControlPointOut() * (3.0 * t2 * one_minus_t);
            const double v3 = 1.0 * t3;
            const double dblBlend = v0 + v1 + v2 + v3;

            const TYPE &value1 = keyframes[i].getValue().getPosition();
            const TYPE &value2 = keyframes[i + 1].getValue().getPosition();
            result = value1 * (1.0 - dblBlend) + value2 * dblBlend;
        }
    };


    typedef TemplateCubicBezierLinearInterpolator<double, DoubleCubicBezier>    DoubleCubicBezierLinearInterpolator;
    typedef TemplateCubicBezierLinearInterpolator<float, FloatCubicBezier>      FloatCubicBezierLinearInterpolator;

    typedef TemplateSampler<DoubleCubicBezierLinearInterpolator>    DoubleCubicBezierLinearSampler;
    typedef TemplateSampler<FloatCubicBezierLinearInterpolator>     FloatCubicBezierLinearSampler;

    typedef TemplateChannel<DoubleCubicBezierLinearSampler>         DoubleCubicBezierLinearChannel;
    typedef TemplateChannel<FloatCubicBezierLinearSampler>          FloatCubicBezierLinearChannel;
}

#endif
