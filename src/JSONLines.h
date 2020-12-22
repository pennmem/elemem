#ifndef JSONLINES_H
#define JSONLINES_H

#include "RC/RStr.h"
#include "RC/RTime.h"
#include "ConfigFile.h"

namespace CML {
  JSONFile MakeResp(RC::RStr type, uint64_t id=uint64_t(-1));
}

#endif // JSONLINES_H

