#ifndef I_DATA_SET_SEGMENT_COLLECTION_H_3C004182_6B15_427F_8312_E7B9A0A42489_INCLUDE
#define I_DATA_SET_SEGMENT_COLLECTION_H_3C004182_6B15_427F_8312_E7B9A0A42489_INCLUDE

#include <OpenSP/Ref.h>
#include "export.h"

namespace dk
{

class IDataSetObj;
class IDataSetSegment;
class IDataSetSegmentCollection : public OpenSP::Ref
{
public:
    virtual bool             addDataSetSegment(IDataSetSegment *pDataSetSeg) = 0;
    virtual unsigned         getSegmentCount(void)                           = 0;
    virtual bool             deleteDataSetSegment(unsigned nIndex)           = 0;
    virtual IDataSetSegment* getDataSetSegment(unsigned nIndex)              = 0;
    virtual void             setParent(IDataSetObj* pParent)                 = 0;
    virtual IDataSetObj*     getParent()                                     = 0;
};

}

#endif

