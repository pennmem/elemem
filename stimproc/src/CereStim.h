// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the CereStim API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CERESTIM_H
#define CERESTIM_H

#include <stdexcept>
#include <string>
#include <vector>
#include "CereStimDLL.h"
#include "ChannelConf.h"


namespace CML {
  class CSMaxValues {
    public:
    uint8_t voltage; // [7,15], (0.5 + 0.6*voltage) = value in volts.
    uint16_t amplitude; // Unit 1uA, granularity 100uA for macro.
    uint32_t phase_charge; // Unit pC.  amplitude_us * width_us.  (Not 100us)
    uint32_t frequency; // Unit Hz.
  };

  class CSStimProfile;

  class CereStim {
    public:
    CereStim();
    ~CereStim();

    // Rule of 3.
    CereStim(const CereStim& other) = delete;
    CereStim& operator=(const CereStim& other) = delete;

    void Open();  // Automatic at first use.
    void Close();

    CSMaxValues GetMaxValues();
    void SetMaxValues(CSMaxValues max_vals);

    void ConfigureStimulation(CSStimProfile profile);
    void Stimulate();
    void StopStimulation();

    uint16_t ShannonCriteria(float area_mmsq);
    uint16_t ShannonCriteria(const CSStimChannel& chan);
    bool ShannonSafe(float area_mmsq, uint16_t amplitude_uA);
    bool ShannonSafe(const CSStimChannel& chan);
    void ShannonAssert(float area_mmsq, uint16_t amplitude_uA);
    void ShannonAssert(const CSStimChannel& chan);

    uint32_t GetBurstSlowFreq() const { return burst_slow_freq; }
    uint32_t GetBurstDuration_us() const { return burst_duration_us; }

    private:
    void BeOpen();
    void ErrorCheck(int err);

    uint32_t burst_slow_freq = 0; // Unit Hz.  Slower envelope freq of bursts.
    float burst_frac = 1; // Fraction of 1/burst_slow_freq to stimulate for.
    uint32_t burst_duration_us = 0;

    uint16_t stim_width_us = 300;
    bool was_active = false;
    bool is_open = false;
    bool is_configured = false;
  };


  class CSException : public std::runtime_error {
    public:
    CSException(int error_code_)
      : std::runtime_error(std::string("CereStim Error ") +
          std::to_string(error_code_) + ", " + CodeToString(error_code_)),
        error_code(error_code_),
        error_message(CodeToString(error_code_)) {
    }
    virtual ~CSException(); // to specify translation unit for v-table.

    CS_Result GetEnum() { return (CS_Result)error_code; }
    int GetErrorCode() { return error_code; }
    std::string GetErrorMsg() { return error_message; }

    static std::string CodeToString(int err);

    private:
    int error_code;
    std::string error_message;
  };


  class CSStimProfile {
    public:
    CSStimProfile& operator+=(const CSStimChannel& chan);
    size_t size() const { return stim_profile.size(); }
    const CSStimChannel& operator[](size_t i) const {
      return stim_profile[i];
    }

    private:
    std::vector<CSStimChannel> stim_profile;
    friend class CereStim;
  };

}

#endif

