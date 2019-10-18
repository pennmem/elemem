#include "StimWorker.h"

namespace CML {
  StimWorker::StimWorker() {
  }


  void StimWorker::ConfigureStimulation_Handler(CSStimProfile& profile) {
    // TODO - log event
    cerestim.ConfigureStimulation(profile);
  }


  void StimWorker::Stimulate_Handler() {
    // TODO - log event
    cerestim.Stimulate();
  }

  void StimWorker::CloseCereStim_Handler() {
    cerestim.Close();
  }
}

