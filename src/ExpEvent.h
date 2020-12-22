#ifndef EXPEVENT_H
#define EXPEVENT_H

#include "RC/Caller.h"

namespace CML {
  class ExpEvent {
    public:
    RC::Caller<> pre_event;  // Parameters pre-bound with Bind.
    RC::Caller<> event;  // Parameters pre-bound with Bind.
    uint64_t active_ms;  // active duration
    uint64_t event_ms;  // total event time
  };
}

#endif // EXPEVENT_H

