#include "StimWorker.h"
#include "ConfigFile.h"
#include "EventLog.h"
#include "NetWorker.h"

namespace CML {
  StimWorker::StimWorker(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
  }


  void StimWorker::ConfigureStimulation_Handler(CSStimProfile& profile) {
    cur_profile = profile;
    cerestim.ConfigureStimulation(profile);
  }


  void StimWorker::Stimulate_Handler() {
    JSONFile event_base = NetWorker::MakeResp("STIM");
    for (size_t i=0; i<cur_profile.size(); i++) {
      JSONFile event = event_base;
      event.Set(uint32_t(cur_profile[i].electrode_pos), "data",
          "electrode_pos");
      event.Set(uint32_t(cur_profile[i].electrode_neg), "data",
          "electrode_neg");
      event.Set(cur_profile[i].amplitude*1e-3, "data", "amplitude");
      event.Set(cur_profile[i].frequency, "data", "frequency");
      event.Set(cur_profile[i].duration*1e-3, "data", "duration");

      hndl->event_log.Log(event.Line());
    }

    cerestim.Stimulate();
  }

  void StimWorker::CloseCereStim_Handler() {
    cerestim.Close();
  }
}

