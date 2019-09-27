#ifndef HANDLER_H
#define HANDLER_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include <QObject>


namespace CML {
  class MainWindow;
  
  class Handler : public RCqt::WorkerThread {
    public:

    Handler();

    void SetMainWindow(RC::Ptr<MainWindow> new_main);


    private:

    RC::Ptr<MainWindow> main_window;
  };
}



#endif // HANDLER_H

