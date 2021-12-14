#ifndef TASKCLASSIFIERSETTINGS_H
#define TASKCLASSIFIERSETTINGS_H

namespace CML {
  /// This is a simple enum class that lists the classification types.
  enum class ClassificationType{
    STIM,
    SHAM,
    NORMALIZE
  };

  /// This is a simple class that acts as a container for classifier settings.
  class TaskClassifierSettings {
    public:
    ClassificationType cl_type;
    size_t sampling_rate;
    size_t duration_ms;
    size_t binned_sampling_rate;
  };
}

#endif // TASKCLASSIFIERSETTINGS_H

