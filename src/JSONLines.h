#ifndef JSONLINES_H
#define JSONLINES_H

#include "RC/RStr.h"
#include "RC/RTime.h"
#include "ConfigFile.h"

namespace CML {
  JSONFile MakeResp(RC::RStr type, uint64_t id, const JSONFile& data);

  inline JSONFile MakeResp(RC::RStr type, uint64_t id=uint64_t(-1),
      RC::RStr data_str=R"({})") {
    JSONFile data;
    data.Parse(data_str);
    return MakeResp(type, id, data);
  }
}

#endif // JSONLINES_H

