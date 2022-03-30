#ifndef TASKCLASSIFIERSETTINGS_H
#define TASKCLASSIFIERSETTINGS_H

#include <cstdint>
#include "RC/RStr.h"

namespace CML {
  /// This is a simple enum class that lists the classification types.
  enum class ClassificationType {
    STIM,
    SHAM,
    NORMALIZE,
    NOSTIM  // for non-stim/non-sham (e.g. post-stim) classification events
  };

  //RC::RStr ClassificationTypeToRStr(const ClassificationType& cl_type) {
  //  switch (cl_type) {
  //    case ClassificationType::STIM: return "STIM";
  //    case ClassificationType::SHAM: return "SHAM";
  //    case ClassificationType::NORMALIZE: return "NORMALIZE";
  //    default: Throw_RC_Error("Invalid classification type received.");
  //  }   
  //}

  /// This is a simple class that acts as a container for classifier settings.
  class TaskClassifierSettings {
    public:
    ClassificationType cl_type;
    size_t duration_ms;
    uint64_t classif_id = uint64_t(-1);
  };
}

#endif // TASKCLASSIFIERSETTINGS_H

