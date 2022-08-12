#ifndef CPSSPECS_H
#define CPSSPECS_H

namespace CML {
  class CPSSpecs {
    public:
    CPSSpecs() { Clear(); }
    void Clear() {
      intertrial_range_ms.Clear();
      sham_duration_ms = uint64_t(0);
      n_normalize_events = 0;
      classify_ms = 0;
      poststim_biomarker_lockout_ms = 0;
      obsNoise = 0;
      exp_bias = 0;
      n_init_samples = 0;
      kern_lengthscale_lb = 0;
      kern_lengthscale_ub = 0;
      kern_var_lb = 0;
      kern_var_ub = 0;
      kern_white_lb = 0;
      kern_white_ub = 0;
    }

    // timing/normalization specs
    RC::Data1D<uint64_t> intertrial_range_ms;
    uint64_t sham_duration_ms;
    // number of events for normalizing EEG features
    size_t n_normalize_events;
    // classification interval duration
    uint64_t classify_ms;
    // lockout period between stim offset and post-stim biomarker/classification interval onset
    uint64_t poststim_biomarker_lockout_ms;

    // Bayesian optimization specs
    double obsNoise;
    double exp_bias;
    size_t n_init_samples;
    // kernel parameter upper (ub) and lower (lb) bounds
    double kern_lengthscale_lb;  // fitting kernel lengthscale
    double kern_lengthscale_ub;
    double kern_var_lb;  // fitting kernel variance
    double kern_var_ub;
    double kern_white_lb;  // white noise variance
    double kern_white_ub;
  };
}

#endif // CPSSPECS_H

