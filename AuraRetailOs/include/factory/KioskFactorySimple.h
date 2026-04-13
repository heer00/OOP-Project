#ifndef KIOSKFACTORYSIMPLE_H
#define KIOSKFACTORYSIMPLE_H
  
#include <string>
#include "factory/KioskFactory.h"
  
class KioskFactorySimple {
public:
    static KioskFactory* createFactory(const std::string& type);
};
  
#endif