#include "StimWorker.h"
#include "ConfigFile.h"
#include "EventLog.h"
#include "Handler.h"
#include "JSONLines.h"
#include "NetWorker.h"
#include "StatusPanel.h"

namespace CML {
  StimWorker::StimWorker(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
  }

  StimulatorType StimWorker::GetStimulatorType() const {
#ifdef CERESTIM_SIMULATOR
      return StimulatorType::Simulator;
#else
      return StimulatorType::CereStim;
#endif

  }

  void StimWorker::ConfigureStimulation_Handler(const CSStimProfile& profile) {
    cur_profile = profile;
    cerestim.ConfigureStimulation(profile);

    max_duration = 0;
    for (size_t i=0; i<profile.size(); i++) {
      max_duration = std::max(max_duration, profile[i].duration);
    }
  }


  void StimWorker::Stimulate_Handler() {
    size_t num_bursts = 1;
    f64 burst_period = 0;
    if (cerestim.GetBurstSlowFreq() != 0) {
      num_bursts = std::round(1e-6*cerestim.GetBurstDuration_us() *
          cerestim.GetBurstSlowFreq());
      burst_period = 1.0 / cerestim.GetBurstSlowFreq();
    }

    RC::Time timer;
    cerestim.Stimulate();
    status_panel->SetStimming(max_duration);

    JSONFile event_base = MakeResp("STIMMING");
    for (size_t i=0; i<cur_profile.size(); i++) {
      JSONFile event = event_base;
      event.Set(uint32_t(cur_profile[i].electrode_pos), "data",
          "electrode_pos");
      event.Set(uint32_t(cur_profile[i].electrode_neg), "data",
          "electrode_neg");
      event.Set(cur_profile[i].amplitude*1e-3, "data", "amplitude");
      event.Set(cur_profile[i].frequency, "data", "frequency");
      event.Set(cur_profile[i].duration*1e-3, "data", "duration");

      // Log burst data if it's a burst.
      if (cur_profile[i].burst_frac != 1) {
        event.Set(cur_profile[i].burst_slow_freq,
            "data", "burst_slow_frequency");
        event.Set(cur_profile[i].burst_frac, "data", "burst_fraction");
      }

      hndl->event_log.Log(event.Line());
    }

    for (size_t b=1; b<num_bursts; b++) {
      f64 burst_time_left = burst_period*b - timer.SinceStart();
      if (burst_time_left > 0) {
        RC::Time::Sleep(burst_time_left);
      }

      if (ShouldAbort()) {
        return;
      }

      cerestim.Stimulate();
    }
  }

  void StimWorker::CloseCereStim_Handler() {
    cerestim.Close();
  }
}

