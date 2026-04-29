#ifndef DIAGNOSISSERVICE_H
#define DIAGNOSISSERVICE_H

#include <string>
#include "core/KioskInterface.h"

class DiagnosisReport {
public:
    std::string fullReport;
};

class DiagnosisService {
public:
    DiagnosisReport runFullCheck(KioskInterface* kiosk);
};

#endif


