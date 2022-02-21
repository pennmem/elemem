#ifndef CPSSPECS_H
#define CPSSPECS_H

namespace CML {
  class CPSSpecs {
    public:
    CPSSpecs() { Clear(); }
    void Clear() {
      experiment_duration_secs = size_t(-1);
      intertrial_range_ms.Clear();
      sham_duration_ms = uint64_t(-1);
    }

    size_t experiment_duration_secs;
    RC::Data1D<uint64_t> intertrial_range_ms;
    uint64_t sham_duration_ms;
  };
}

#endif // CPSSPECS_H

