#include "TaskClassifierManager.h"
#include "RC/Macros.h"
#include "Classifier.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "JSONLines.h"

namespace CML {
  TaskClassifierManager::TaskClassifierManager(RC::Ptr<Handler> hndl,
    size_t sampling_rate, size_t duration_ms, size_t bin_frequency)
    : hndl(hndl), circular_data(sampling_rate, duration_ms), sampling_rate(sampling_rate) {
    callback_ID = RC::RStr("TaskClassifierManager_") + bin_frequency;
    task_classifier_settings.binned_sampling_rate = bin_frequency;

    hndl->eeg_acq.RegisterCallback(callback_ID, ClassifyData);
  }

  void TaskClassifierManager::StartClassification() {
	if (!callback.IsSet()) Throw_RC_Error("Start classification callback not set");

    stim_event_waiting = false;
    num_eeg_events_before_stim = 0;

    RC::APtr<const EEGData> data = circular_data.GetData().ExtractConst();
    // TODO: JPB: (need) Exception if binned_sampling_rate (as early as possible in setup) is less than 2.99X the max classification freq.
    RC::APtr<const EEGData> binned_data = EEGCircularData::BinData(data, task_classifier_settings.binned_sampling_rate).ExtractConst();

    callback(binned_data, task_classifier_settings);
  }

  void TaskClassifierManager::ClassifyData_Handler(RC::APtr<const EEGData>& data) {
    //RC_DEBOUT(RC::RStr("TaskClassifierManager_Handler\n"));
    auto& datar = data->data;

    if (stim_event_waiting) {
      RC_DEBOUT(num_eeg_events_before_stim);
      if (num_eeg_events_before_stim <= datar[0].size()) { // TODO: JPB: (Need) How to handle if first channel is empty
        circular_data.Append(data, 0, num_eeg_events_before_stim);
        StartClassification();
        circular_data.Append(data, num_eeg_events_before_stim);
      } else { // num_eeg_events_before_stim > datar.size()
        circular_data.Append(data);
        num_eeg_events_before_stim -= datar[0].size(); // TODO: JPB: (Need) How to handle if first channel is empty
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
    if (!stim_event_waiting) {
      stim_event_waiting = true;
      num_eeg_events_before_stim = duration_ms * sampling_rate / 1000;
      task_classifier_settings.cl_type = cl_type;
      task_classifier_settings.duration_ms = duration_ms;
      task_classifier_settings.classif_id = classif_id;
    } else {
      // TODO: JPB: (feature) Allow classifier to start gather EEGData at the same time as another gathering?  Requires id queue.
      hndl->event_log.Log("Skipping stim event, another stim event is already waiting (collecting EEGData)");
    }
  }

  void TaskClassifierManager::SetCallback_Handler(const TaskClassifierCallback& new_callback) {
    callback = new_callback;
  }
}
