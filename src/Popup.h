#ifndef POPUP_H
#define POPUP_H

#include "RC/RC.h"
#include "RCqt/Worker.h"
#include <QObject>
#include <sstream>

namespace RC {
  RC_MAKE_ERROR_TYPE(Note);
}

#define DEBLOG_OUT_HELP(v) + ", " + #v + " = " + RC::RStr([&](){std::stringstream ss; ss << v; return ss.str();}())
#define DEBLOG_OUT(...) CML::DebugLog(RC::RStr(__FILE__) + ":" + RC::RStr(__LINE__) RC_ARGS_EACH(DEBLOG_OUT_HELP,__VA_ARGS__));

#define CatchErrorsReturn() \
  catch(std::exception& ex) { CML::DispatchError(ex); return true; } \
  catch (...) { \
    RStr errormsg = "Unknown exception type"; \
    CML::ErrorWin(errormsg); \
    return true; \
  }

#define CatchErrors() \
  catch(std::exception& ex) { CML::DispatchError(ex); } \
  catch (...) { \
    RStr errormsg = "Unknown exception type"; \
    CML::ErrorWin(errormsg); \
  }

namespace CML {
  void PopupWin(const RC::RStr& message, const RC::RStr& title="");
  bool ConfirmWin(const RC::RStr& message, const RC::RStr& title="");
  void ErrorWin(const RC::RStr& message, const RC::RStr& title="Error",
      const RC::RStr& log_message="");
  void DebugLog(const RC::RStr& message);

  class PopupManager : public RCqt::Worker {
    public:

    static RC::Ptr<PopupManager> GetManager();

    RCqt::TaskCaller<const RC::RStr, const RC::RStr> Info =
      TaskHandler(PopupManager::Info_Handler);
    RCqt::TaskGetter<bool, const RC::RStr, const RC::RStr> Confirm =
      TaskHandler(PopupManager::Confirm_Handler);
    // No pop-up for this one.  Debugging only.
    RCqt::TaskCaller<const RC::RStr> LogMsg =
      TaskHandler(PopupManager::LogMsg_Handler);
    RCqt::TaskCaller<const RC::RStr, const RC::RStr, const RC::RStr> Error =
      TaskHandler(PopupManager::Error_Handler);

    RCqt::TaskCaller<const RC::RStr> SetLogFile =
      TaskHandler(PopupManager::SetLogFile_Handler);

    protected:

    void Info_Handler(const RC::RStr& message, const RC::RStr& title);
    bool Confirm_Handler(const RC::RStr& message, const RC::RStr& title);
    void LogMsg_Handler(const RC::RStr& message);
    void Error_Handler(const RC::RStr& message, const RC::RStr& title,
        const RC::RStr& log_message);

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

  void DispatchError(RC::ErrorMsgFatal& err);
  void DispatchError(RC::ErrorMsgNote& err);
  void DispatchError(RC::ErrorMsg& err);
  #ifndef NO_HDF5
  void DispatchError(H5::Exception& ex);
  #endif // NO_HDF5
  void DispatchError(std::exception &ex);
}



#endif // POPUP_H

