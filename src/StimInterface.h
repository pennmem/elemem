// 2022, James Bruska
// Computational Memory Lab, University of Pennsylvania
//
// This file provides the interface for Stim
//
/////////////////////////////////////////////////////////////////////////////

#ifndef STIMINTERFACE_H
#define STIMINTERFACE_H

#include <vector>
#include <RC/Macros.h>
#include "RCqt/Worker.h"
#include "ChannelConf.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>

namespace CML {
  class StimMaxValues {
    public:
    uint8_t voltage = 0; // [7,15], (0.5 + 0.6*voltage) = value in volts.
    uint16_t amplitude = 0; // Unit 1uA, granularity 100uA for macro.
    uint32_t phase_charge = 0; // Unit pC.  amplitude_us * width_us.  (Not 100us)
    uint32_t frequency = 0; // Unit Hz.
  };

  class StimProfile {
    public:
    StimProfile& operator+=(const StimChannel& chan) {
      stim_profile.push_back(chan);
      return *this;
    }
    size_t size() const {
      return stim_profile.size();
    }
    const StimChannel& operator[](size_t i) const {
      return stim_profile[i];
    }

    protected:
    // TODO: JPB: (refactor) Change vector to Data1D
    std::vector<StimChannel> stim_profile;
  };

  class StimInterface {
    public:
    StimInterface() = default;
    virtual ~StimInterface() = default;

    // NOTE: All public functions of this interface must be PURE VIRTUAL !!
    //       These can then be defined in the derived class and use the protected functions.
    //       This is NEEDED because a threading architecture problem would occur if the 
    //       derived class inherits from RCqt::WorkerThread and then another WorkerThread class 
    //       viewed or edited a variable in this base class from the other classes WorkerThread.
    //
    //       Make sure to use a TaskHandler within all definitions of these virtual functions 
    //       for all Derived classes that use RCqt::WorkerThread !!
    //
    //       Also, all arguments MUST be pass by value
    virtual void ConfigureStimulation(StimProfile profile) = 0;
    virtual void OpenInterface() = 0;
    virtual void CloseInterface() = 0;
    virtual void Stimulate() = 0;

    virtual uint32_t GetBurstSlowFreq() = 0;
    virtual uint32_t GetBurstDuration_us() = 0;


    protected:
    virtual void ConfigureStimulation_Helper(const StimProfile& profile) = 0;
    virtual void OpenInterface_Helper() = 0;
    virtual void CloseInterface_Helper() = 0;
    virtual void Stimulate_Helper() = 0;

    void ConfigureStimulation_Handler(const StimProfile& profile);
    void OpenInterface_Handler();
    void CloseInterface_Handler();
    void Stimulate_Handler();
    uint32_t GetBurstSlowFreq_Handler();
    uint32_t GetBurstDuration_us_Handler();

    uint16_t ShannonCriteria(float area_mmsq);
    uint16_t ShannonCriteria(const StimChannel& chan);
    bool ShannonSafe(float area_mmsq, uint16_t amplitude_uA);
    bool ShannonSafe(const StimChannel& chan);
    void ShannonAssert(float area_mmsq, uint16_t amplitude_uA);
    void ShannonAssert(const StimChannel& chan);

    bool is_configured = false;

    uint32_t burst_slow_freq = 0; // Unit Hz.  Slower envelope freq of bursts.
    float burst_frac = 1; // Fraction of 1/burst_slow_freq to stimulate for.
    uint32_t burst_duration_us = 0;

    uint16_t stim_width_us = 300;

    const StimMaxValues stim_max_vals = {
      .voltage=15, // (0.5 + 0.6*15)V = 9.5V
      .amplitude=3500, // 35*100uA.
      .phase_charge=20000000, // Won't go above 1.05e6 with 300us
      .frequency=1000}; // Hz
  };
}

#endif

