#ifndef HANDLER_H
#define HANDLER_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include "APITests.h"
#include "EEGAcq.h"
#include "StimWorker.h"
#include <QObject>


namespace CML {
  class MainWindow;
  
  class Handler : public RCqt::WorkerThread {
    public:

    Handler();

    void SetMainWindow(RC::Ptr<MainWindow> new_main);

    RCqt::TaskCaller<> CerebusTest =
      TaskHandler(Handler::CerebusTest_Handler);

    RCqt::TaskCaller<> CereStimTest =
      TaskHandler(Handler::CereStimTest_Handler);

    EEGAcq ecg_acq;
    StimWorker stim_worker;

    private:

    RC::Ptr<MainWindow> main_window;

    void CerebusTest_Handler() { APITests::CereLinkTest(); }
    void CereStimTest_Handler() { APITests::CereStimTest(); }
  };
}



#endif // HANDLER_H

