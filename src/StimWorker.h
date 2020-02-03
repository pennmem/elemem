#ifndef STIMWORKER_H
#define STIMWORKER_H

#include "CereStim.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class Handler;

  class StimWorker : public RCqt::WorkerThread {
    public:

    StimWorker(RC::Ptr<Handler> hndl);

    RCqt::TaskCaller<CSStimProfile> ConfigureStimulation =
      TaskHandler(StimWorker::ConfigureStimulation_Handler);

    RCqt::TaskCaller<> Stimulate =
      TaskHandler(StimWorker::Stimulate_Handler);

    RCqt::TaskBlocker<> CloseCereStim =
      TaskHandler(StimWorker::CloseCereStim_Handler);

    protected:

    void ConfigureStimulation_Handler(CSStimProfile& profile);
    void Stimulate_Handler();

    void CloseCereStim_Handler();

    RC::Ptr<Handler> hndl;

    CereStim cerestim;
    CSStimProfile cur_profile;
  };
}

#endif // STIMWORKER_H

