#ifndef STIMWORKER_H
#define STIMWORKER_H

#include "CereStim.h"
#include "RCqt/Worker.h"

namespace CML {
  class StimWorker : public RCqt::WorkerThread {
    public:

    StimWorker();

    RCqt::TaskCaller<CSStimProfile> ConfigureStimulation =
      TaskHandler(StimWorker::ConfigureStimulation_Handler);

    RCqt::TaskCaller<> Stimulate =
      TaskHandler(StimWorker::Stimulate_Handler);

    protected:

    void ConfigureStimulation_Handler(CSStimProfile& profile);
    void Stimulate_Handler();

    CereStim cerestim;
  };
}

#endif // STIMWORKER_H

