#ifndef POPUP_H
#define POPUP_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include <QObject>

namespace RC {
  RC_MAKE_ERROR_TYPE(Note);
}

namespace CML {
  void PopupWin(const RC::RStr& message, const RC::RStr& title="");
  bool ConfirmWin(const RC::RStr& message, const RC::RStr& title="");
  void ErrorWin(const RC::RStr& message, const RC::RStr& title="Error");

  class PopupManager : public RCqt::Worker {
    public:

    static RC::Ptr<PopupManager> GetManager();

    RCqt::TaskCaller<const RC::RStr, const RC::RStr> Info =
      TaskHandler(PopupManager::Info_Handler);
    RCqt::TaskGetter<bool, const RC::RStr, const RC::RStr> Confirm =
      TaskHandler(PopupManager::Confirm_Handler);
    RCqt::TaskCaller<const RC::RStr, const RC::RStr> Error =
      TaskHandler(PopupManager::Error_Handler);

    RCqt::TaskCaller<const RC::RStr> SetLogFile =
      TaskHandler(PopupManager::SetLogFile_Handler);

    protected:

    void Info_Handler(const RC::RStr& message, const RC::RStr& title);
    bool Confirm_Handler(const RC::RStr& message, const RC::RStr& title);
    void Error_Handler(const RC::RStr& message, const RC::RStr& title);

    void SetLogFile_Handler(const RC::RStr& filename);

    RC::FileWrite log_file;

    private: signals:
    void InfoSignal(RC::RStr message, RC::RStr title);
    void ConfirmSignal(RC::RStr message, RC::RStr title);
    void ErrorSignal(RC::RStr message, RC::RStr title);
    private slots:
    void InfoSlot(RC::RStr message, RC::RStr title);
    void ConfirmSlot(RC::RStr message, RC::RStr title);
    void ErrorSlot(RC::RStr message, RC::RStr title);
  };
}



#endif // POPUP_H

