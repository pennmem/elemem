#ifndef OPSSPECS_H
#define OPSSPECS_H

namespace CML {
  class OPSSpecs {
    public:
    OPSSpecs() { Clear(); }
    void Clear() {
      num_stim_trials = size_t(-1);
      num_sham_trials = size_t(-1);
      intertrial_range_ms.Clear();
      sham_duration_ms = uint64_t(-1);
    }

    size_t num_stim_trials;
    size_t num_sham_trials;
    RC::Data1D<uint64_t> intertrial_range_ms;
    uint64_t sham_duration_ms;
  };
}

#endif // OPSSPECS_H

