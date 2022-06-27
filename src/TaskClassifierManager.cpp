#include "TaskClassifierManager.h"
#include "RC/Macros.h"
#include "Classifier.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "JSONLines.h"

namespace CML {
  TaskClassifierManager::TaskClassifierManager(RC::Ptr<Handler> hndl,
    size_t sampling_rate, size_t circ_buf_duration_ms)
    : hndl(hndl), circular_data(sampling_rate, circ_buf_duration_ms),
      sampling_rate(sampling_rate) {
    callback_ID = RC::RStr("TaskClassifierManager_") + sampling_rate;
    hndl->eeg_acq.RegisterEEGCallback(callback_ID, ClassifyData);
  }

  TaskClassifierManager::~TaskClassifierManager() {
    Shutdown_Handler();
  }

  void TaskClassifierManager::Shutdown_Handler() {
    if (callback_ID.size() > 0) {
      hndl->eeg_acq.RemoveEEGCallback(callback_ID);
      callback_ID = "";
    }
  }

  void TaskClassifierManager::StartClassification() {
    if (!callback.IsSet()) {
      Throw_RC_Error("Start classification callback not set");
    }

    stim_event_waiting = false;
    num_eeg_events_before_stim = 0;

    size_t num_samples = task_classifier_settings.duration_ms *
      sampling_rate / 1000;
    RC::APtr<const EEGDataDouble> data =
      circular_data.GetRecentData(num_samples).ExtractConst();

    if (ShouldAbort()) { return; }
    callback(data, task_classifier_settings);
  }

  void TaskClassifierManager::ClassifyData_Handler(
      RC::APtr<const EEGDataDouble>& data) {

    if (stim_event_waiting) {
      if (num_eeg_events_before_stim <= data->sample_len) {
        circular_data.Append(data, 0, num_eeg_events_before_stim);
        StartClassification();
        circular_data.Append(data, num_eeg_events_before_stim);
      } else { // num_eeg_events_before_stim > datar.size()
        circular_data.Append(data);
        num_eeg_events_before_stim -= data->sample_len;
      }
    } else {
      // TODO: JPB: (feature) This can likely be removed to reduce overhead
      //            If there is no stim event waiting, then don't update data
      circular_data.Append(data);
    }
  }

  void TaskClassifierManager::ProcessClassifierEvent_Handler(
        const ClassificationType& cl_type, const uint64_t& duration_ms,
        const uint64_t& classif_id) {
    if (duration_ms > circular_data.duration_ms) {
      Throw_RC_Error(("Classification duration (" + RC::RStr(duration_ms) +
            ") is greater than the circular buffer duration (" +
            RC::RStr(circular_data.duration_ms) + ")").c_str());
    }

    if (!stim_event_waiting) {
      stim_event_waiting = true;
      num_eeg_events_before_stim = duration_ms * sampling_rate / 1000;
      task_classifier_settings.cl_type = cl_type;
      task_classifier_settings.duration_ms = duration_ms;
      task_classifier_settings.classif_id = classif_id;
    } else {
      // TODO: JPB: (feature) Allow classifier to start gather EEGData at the same time as another gathering?  Requires id queue.
      hndl->event_log.Log("Skipping stim event, another stim event is "
          "already waiting (collecting EEGData)");
    }
  }

  void TaskClassifierManager::SetCallback_Handler(
      const TaskClassifierCallback& new_callback) {
    callback = new_callback;
  }
}
