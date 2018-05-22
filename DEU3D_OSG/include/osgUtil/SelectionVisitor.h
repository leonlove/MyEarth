#ifndef SELECTION_VISITOR_H
#define SELECTION_VISITOR_H 1

#include "IntersectionVisitor.h"

namespace cmmOSG
{


class SelectionVisitor : public IntersectionVisitor
{
public:
    explicit SelectionVisitor(void);
    virtual ~SelectionVisitor(void);
protected:
};

}
#endif
