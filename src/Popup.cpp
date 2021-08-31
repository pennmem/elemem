#include "Popup.h"
#include "RC/RC.h"
#include <QMessageBox>
#include <QMetaType>


using namespace std;
using namespace RC;

namespace CML {
  void PopupWin(const RStr& message, const RStr& title) {
    PopupManager::GetManager()->Info(message, title);
  }


  bool ConfirmWin(const RStr& message, const RStr& title) {
    return PopupManager::GetManager()->Confirm(message, title);
  }


  void ErrorWin(const RStr& message, const RStr& title,
      const RStr& log_message) {
    PopupManager::GetManager()->Error(message, title, log_message);
  }

  void DebugLog(const RStr& message) {
    PopupManager::GetManager()->LogMsg(message);
  }

  Ptr<PopupManager> PopupManager::GetManager() {
    static PopupManager singleton;
    return Ptr<PopupManager>(&singleton);
  }


  void PopupManager::Info_Handler(const RStr& message, const RStr& title) {
    QMessageBox::information(0, title.c_str(), message.c_str());
  }

  bool PopupManager::Confirm_Handler(const RStr& message, const RStr& title) {
    return (QMessageBox::question(0, title.c_str(), message.c_str(),
            QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes)
            == QMessageBox::Yes);
  }

  void PopupManager::LogMsg_Handler(const RStr& message) {
    if (log_file.IsOpen()) {
      log_file.Put(Time::GetStr() + ", " + message + "\n");
    }
  }

  void PopupManager::Error_Handler(const RStr& message, const RStr& title,
      const RStr& log_message) {
    if (log_message.empty()) {
      LogMsg_Handler(title + "\n" + message);
    }
    else {
      LogMsg_Handler(title + "\n" + log_message);
    }
    QMessageBox::warning(0, title.c_str(), message.c_str());
  }

  void PopupManager::SetLogFile_Handler(const RStr& filename) {
    log_file.Open(filename, APPEND);
  }
}


