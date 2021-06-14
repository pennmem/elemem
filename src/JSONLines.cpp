#include "JSONLines.h"

namespace CML {
  JSONFile MakeResp(RC::RStr type, uint64_t id, const JSONFile& data) {
    JSONFile resp;
    resp.SetFilename("HostResponse");
    resp.Set(type.Raw(), "type");
    resp.Set(data, "data");
    if (id != uint64_t(-1)) {
      resp.Set(id, "id");
    }
    resp.Set(RC::Time::Get()*1e3, "time");  // ms from 1970-01-01 00:00:00 UTC
    return resp;
  }
}

