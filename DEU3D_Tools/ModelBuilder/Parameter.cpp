#include "Parameter.h"
#include "md2.h"

Parameter::Parameter(void)
{
}


Parameter::~Parameter(void)
{
}

void Parameter::saveSymbolAsShortcut(bool is_shortcut)
{
    _symbol_as_shortcut = is_shortcut;
}

bool Parameter::isSymbolAShortcut()
{
    return _symbol_as_shortcut;
}

