#include "TaskStimManager.h"
#include "RC/Macros.h"
#include "Classifier.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "JSONLines.h"

namespace CML {
  TaskStimManager::TaskStimManager(RC::Ptr<Handler> hndl) : hndl(hndl) {
    callback_ID = RC::RStr("TaskStimManager");
  }

  void TaskStimManager::StimDecision_Handler(const double& result,
    const TaskClassifierSettings& task_classifier_settings) {
    RC_DEBOUT(RC::RStr("ClassifierDecision_Handler\n\n"));

    bool stim = result < 0.5;
    bool stim_type =
      (task_classifier_settings.cl_type == ClassificationType::STIM);

    JSONFile data;
    data.Set(result, "result");
    data.Set(stim, "decision");

    const RC::RStr type = [&] {
        switch (task_classifier_settings.cl_type) {
          case ClassificationType::STIM: return "STIM_DECISION";
          case ClassificationType::SHAM: return "SHAM_DECISION";
          default: Throw_RC_Error("Invalid classification type received.");
        }
    }();

    auto resp = MakeResp(type, task_classifier_settings.classif_id, data);
    hndl->event_log.Log(resp.Line());
    RC_DEBOUT(resp);

    if (stim_type && stim) {
      hndl->stim_worker.Stimulate();
      if (callback.IsSet()) { callback(true, task_classifier_settings); }
    } else {
      if (callback.IsSet()) { callback(false, task_classifier_settings); }
    }
  }

  void TaskStimManager::SetCallback_Handler(const TaskStimCallback& new_callback) {
    callback = new_callback;
  }
}
