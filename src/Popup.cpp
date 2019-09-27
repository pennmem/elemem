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


  void ErrorWin(const RStr& message, const RStr& title) {
    PopupManager::GetManager()->Error(message, title);
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

  void PopupManager::Error_Handler(const RStr& message, const RStr& title) {
    QMessageBox::warning(0, "Error", message.c_str());
  }
}


