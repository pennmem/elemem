#include "StimWorker.h"
#include "ConfigFile.h"
#include "EventLog.h"
#include "Handler.h"
#include "JSONLines.h"
#include "StatusPanel.h"

namespace CML {
  StimWorker::StimWorker(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
    // first stim event should always pass lockout constraint
    prev_stim_offset_time_sec = -stim_lockout_sec;
  }

  StimulatorType StimWorker::GetStimulatorType() const {
#ifdef CERESTIM_SIMULATOR
      return StimulatorType::Simulator;
#else
      return StimulatorType::CereStim;
#endif

  }

  void StimWorker::SetStimInterface_Handler(RC::APtr<StimInterface>& new_interface) {
    stim_interface = new_interface;
  }

  void StimWorker::Open_Handler() {
    if (stim_interface.IsNull()) {
      Throw_RC_Error("The stim_interface in StimWorker is null on Configure");
    }

    stim_interface->CloseInterface();
    stim_interface->OpenInterface();
  }

  void StimWorker::ConfigureStimulation_Handler(const StimProfile& profile) {
    if (stim_interface.IsNull()) {
      Throw_RC_Error("The stim_interface in StimWorker is null on Configure");
    }

    cur_profile = profile;
    stim_interface->ConfigureStimulation(profile);

    max_duration = 0;
    for (size_t i=0; i<profile.size(); i++) {
      max_duration = std::max(max_duration, profile[i].duration);
    }
  }


  void StimWorker::Stimulate_Handler() {
    if (stim_interface.IsNull()) {
      Throw_RC_Error("The stim_interface in StimWorker is null on Stimulate");
    }

    // Setup theta-burst values
    size_t num_bursts = 1;
    f64 burst_period = 0;
    if (stim_interface->GetBurstSlowFreq() > 0) {
      num_bursts = std::round(1e-6 * stim_interface->GetBurstDuration_us() *
          stim_interface->GetBurstSlowFreq());
      burst_period = 1.0 / stim_interface->GetBurstSlowFreq();
    }

    // Safety check to stop repeated stimulation
    f64 cur_stim_onset_time_sec = RC::Time::Get();
    if (cur_stim_onset_time_sec - prev_stim_offset_time_sec < stim_lockout_sec) {
      Abort();
      hndl->Abort();
      Throw_RC_Error((string("Stimulation requested before ") +
                      to_string(stim_lockout_sec) +
                      string(" seconds after start of previous stimulation event. Aborting experiment for safety.")).c_str());
    }

    // Stimulate
    RC::Time timer;
    stim_interface->Stimulate();
    status_panel->SetStimming(max_duration);

    // Log Stimulation
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

    // Theta-burst stimulation loop
    for (size_t b=1; (b<num_bursts) && KeepGoing(); b++) {
      f64 burst_time_left = burst_period*b - timer.SinceStart();
      if (burst_time_left > 0) {
        RC::Time::Sleep(burst_time_left);
      }

      if (ShouldAbort()) {
        return;
      }

      stim_interface->Stimulate();
    }

    // Safety check stim offset
    prev_stim_offset_time_sec = RC::Time::Get();
  }

  void StimWorker::CloseStim_Handler() {
    if (stim_interface.IsSet()) {
      stim_interface->CloseInterface();
    }
  }
}

