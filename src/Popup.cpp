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
    QMessageBox::information(nullptr, title.c_str(), message.c_str());
  }

  bool PopupManager::Confirm_Handler(const RStr& message, const RStr& title) {
    return (QMessageBox::question(nullptr, title.c_str(), message.c_str(),
            QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes)
            == QMessageBox::Yes);
  }

  void PopupManager::LogMsg_Handler(const RStr& message) {
    if (log_file.IsOpen()) {
      log_file.Put(Time::GetStr());
      log_file.Put(", ");
      log_file.Put(message);
      log_file.Put("\n");
      log_file.Flush();
    }
    else {
      // Fallback.  In case anyone is watching.
      std::cerr << message << std::endl;
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
    QMessageBox::warning(nullptr, title.c_str(), message.c_str());
  }

  void PopupManager::SetLogFile_Handler(const RStr& filename) {
    log_file.Open(filename, APPEND);
  }


  void DispatchError(RC::ErrorMsgFatal& err) {
    RC::RStr errormsg = RC::RStr("Fatal Error:  ")+err.what();
    ErrorWin(errormsg);
    exit(-1);
  }

  void DispatchError(RC::ErrorMsgNote& err) {
    RC::RStr errormsg = RC::RStr(err.GetError());
    ErrorWin(errormsg);
  }

  void DispatchError(RC::ErrorMsg& err) {
    RC::RStr errormsg = RC::RStr("Error:  ")+err.GetError();
    RC::RStr logmsg = RC::RStr("Error:  ")+err.what();
    ErrorWin(errormsg, "Error", logmsg);
  }

  #ifndef NO_HDF5
  void DispatchError(H5::Exception& ex) {
    RC::RStr errormsg = RC::RStr("HDF5 Error:  ")+ex.getCDetailMsg();
    ErrorWin(errormsg);
  }
  #endif // NO_HDF5

  void DispatchError(std::exception &ex) {
    RC::RStr errormsg = RC::RStr("Unhandled exception: ") + ex.what();
    ErrorWin(errormsg);
  }
}


