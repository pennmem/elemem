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
    // TODO: JPB: what is the intention of these debug statements?
    //            Is this supposed to be the function driving the callback or the callback itself?
    RC_DEBOUT(RC::RStr("TaskStimManager::ClassifierDecision_Handler\n"));

    bool stim = result < 0.5;
    RC_DEBOUT(RC::RStr(string("TaskStimManager::ClassifierDecision_Handler ") + to_string(result) + string(" ") + to_string(stim) + string("\n")));
    bool stim_type =
      (task_classifier_settings.cl_type == ClassificationType::STIM);

    JSONFile data;
    data.Set(result, "result");
    data.Set(stim, "decision");

    const RC::RStr type = [&] {
        switch (task_classifier_settings.cl_type) {
          case ClassificationType::STIM: return "STIM_DECISION";
          case ClassificationType::SHAM: return "SHAM_DECISION";
          case ClassificationType::NOSTIM: return "NOSTIM_DECISION";
          case ClassificationType::NORMALIZE: return "NORMALIZE_NOSTIM_DECISION";
          default: Throw_RC_Error("Invalid classification type received.");
        }
    }();

    auto resp = MakeResp(type, task_classifier_settings.classif_id, data);
    hndl->event_log.Log(resp.Line());
    // RC_DEBOUT(resp);

    f64 stim_time_from_start_sec = RC::Time::Get();
    // JAMES: WHAT WAS PREVIOUSLY HERE WAS NOT CORRECT. SEE COMMENT BELOW. FOR SHAM EVENTS, WE STILL NEED TO KNOW WHETHER THE EVENT WAS DETECTED AS BEING IN A BAD/GOOD MEM STATE, NOT JUST WHETHER A STIM EVENT ACTUALLY OCCURRED
    // return whether or not a bad memory state was detected with <stim>, 
    // not whether a stim event actually occurred (that can be determined with task_classifier_settings).
    if (stim_type && stim) {
      hndl->stim_worker.Stimulate();
    }
    callback(stim, task_classifier_settings, stim_time_from_start_sec);
  }

  void TaskStimManager::SetCallback_Handler(const TaskStimCallback& new_callback) {
    callback = new_callback;
  }
  
  /// Handler that registers a callback on the stim results
  /** @param A (preferably unique) tag/name for the callback
   *  @param The callback on the classifier results
   */
  // void TaskStimManager::RegisterCallback_Handler(const RC::RStr& tag,
  //                                           const TaskStimCallback& callback) {
  //   RemoveCallback_Handler(tag);
  //   data_callbacks += TaggedCallback{tag, callback};
  // }

  // /// Handler that removes a callback on the stim results.
  // * @param The tag to be removed from the list of callbacks
  //  *  Note: All tags of the same name will be removed (even if there is more than one)
   
  // void TaskStimManager::RemoveCallback_Handler(const RC::RStr& tag) {
  //   for (size_t i=0; i<data_callbacks.size(); i++) {
  //     if (data_callbacks[i].tag == tag) {
  //       data_callbacks.Remove(i);
  //       i--;
  //     }
  //   }
  // }

}
