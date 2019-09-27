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


namespace CML {
  class CSMaxValues {
    public:
    uint8_t voltage; // [7,15], (0.5 + 0.6*voltage) = value in volts.
    uint16_t amplitude; // Unit 1us, granularity 100uA for macro.
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

    CSMaxValues GetMaxValues();
    void SetMaxValues(CSMaxValues max_vals);

    void ConfigureStimulation(CSStimProfile profile);
    void Stimulate();
    void StopStimulation();
      
    private:
    void ErrorCheck(int err);

    uint16_t stim_width_us = 300;
    bool was_active = false;
  };


  class CSException : public std::runtime_error {
    public:
    CSException(int error_code_)
      : std::runtime_error(std::string("CereStim Error ") +
          std::to_string(error_code_) + ", " + CodeToString(error_code_)),
        error_code(error_code_),
        error_message(CodeToString(error_code_)) {
    }

    CS_Result GetEnum() { return (CS_Result)error_code; }
    int GetErrorCode() { return error_code; }
    std::string GetErrorMsg() { return error_message; }

    static std::string CodeToString(int err);

    private:
    int error_code;
    std::string error_message;
  };


  class CSStimChannel {
    public:
    uint8_t electrode_pos;  // Goes positive first / anodic
    uint8_t electrode_neg;  // Goes negative first / cathodic
    uint16_t amplitude; // Unit 1uA, granularity 100uA for macro.
    uint32_t frequency; // Unit Hz.
    uint32_t duration;  // Unit us.
  };

  class CSStimProfile {
    public:
    CSStimProfile& operator+=(const CSStimChannel& chan);

    private:
    std::vector<CSStimChannel> stim_profile;
    std::vector<size_t> pattern_index;
    friend class CereStim;
  };

}

#endif

