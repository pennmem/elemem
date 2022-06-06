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
#include "StimInterface.h"


namespace CML {
  // TODO: JPB: (refactor) Remove CSMaxValues?
  class CSMaxValues {
    public:
    uint8_t voltage; // [7,15], (0.5 + 0.6*voltage) = value in volts.
    uint16_t amplitude; // Unit 1uA, granularity 100uA for macro.
    uint32_t phase_charge; // Unit pC.  amplitude_us * width_us.  (Not 100us)
    uint32_t frequency; // Unit Hz.
  };

  class CereStim : public StimInterface {
    public:
    CereStim();
    ~CereStim();

    // Rule of 3.
    CereStim(const CereStim& other) = delete;
    CereStim& operator=(const CereStim& other) = delete;
 
    void ConfigureStimulation(StimProfile profile) override { ConfigureStimulation_Handler(profile); }
    void OpenInterface() override { OpenInterface_Handler(); }
    void CloseInterface() override { CloseInterface_Handler(); }
    void Stimulate() override { Stimulate_Handler(); }
    uint32_t GetBurstSlowFreq() override { return GetBurstSlowFreq_Handler(); }
    uint32_t GetBurstDuration_us() override { return GetBurstDuration_us_Handler(); }

    CSMaxValues GetMaxValues();
    void SetMaxValues(CSMaxValues max_vals);

    void StopStimulation();

//    uint16_t ShannonCriteria(float area_mmsq);
//    uint16_t ShannonCriteria(const StimChannel& chan);
//    bool ShannonSafe(float area_mmsq, uint16_t amplitude_uA);
//    bool ShannonSafe(const StimChannel& chan);
//    void ShannonAssert(float area_mmsq, uint16_t amplitude_uA);
//    void ShannonAssert(const StimChannel& chan);
//
//    uint32_t GetBurstSlowFreq() const { return burst_slow_freq; }
//    uint32_t GetBurstDuration_us() const { return burst_duration_us; }

    protected:
    void ConfigureStimulation_Helper(const StimProfile& profile) override;
    void OpenInterface_Helper() override;  // Automatic at first use.
    void CloseInterface_Helper() override;
    void Stimulate_Helper() override;


    private:
    void BeOpen();
    void ErrorCheck(int err);

//    uint32_t burst_slow_freq = 0; // Unit Hz.  Slower envelope freq of bursts.
//    float burst_frac = 1; // Fraction of 1/burst_slow_freq to stimulate for.
//    uint32_t burst_duration_us = 0;

//    uint16_t stim_width_us = 300;
//    bool is_configured = false;
    bool was_active = false;
    bool is_open = false;
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
}

#endif

