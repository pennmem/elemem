#include "TaskClassifierManager.h"
#include "RC/Macros.h"
#include "Classifier.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "JSONLines.h"

namespace CML {
  TaskClassifierManager::TaskClassifierManager(RC::Ptr<Handler> hndl,
    size_t sampling_rate, size_t circular_buffer_len, size_t bin_frequency)
    : hndl(hndl), circular_data(sampling_rate, circular_buffer_len), sampling_rate(sampling_rate) {
    callback_ID = RC::RStr("TaskClassifierManager_") + bin_frequency;
    task_classifier_settings.binned_sampling_rate = bin_frequency;

    hndl->eeg_acq.RegisterCallback(callback_ID, ClassifyData);
  }

  void TaskClassifierManager::StartClassification() {
    stim_event_waiting = false;
    num_eeg_events_before_stim = 0;

    RC::APtr<const EEGData> data = circular_data.GetData().ExtractConst();
    RC::APtr<const EEGData> binned_data = EEGCircularData::BinData(data, task_classifier_settings.binned_sampling_rate).ExtractConst();

    callback(binned_data, task_classifier_settings);
  }

  void TaskClassifierManager::ClassifyData_Handler(RC::APtr<const EEGData>& data) {
    //RC_DEBOUT(RC::RStr("TaskClassifierManager_Handler\n"));
    auto& datar = data->data;

    if (stim_event_waiting) {
      RC_DEBOUT(RC::RStr("TaskClassifierManager_Handler stim_event_waiting\n"));
      if (num_eeg_events_before_stim <= datar.size()) {
        circular_data.Append(data, 0, num_eeg_events_before_stim);
        StartClassification();
        circular_data.Append(data, num_eeg_events_before_stim);
      } else { // num_eeg_events_before_stim > datar.size()
        circular_data.Append(data);
        num_eeg_events_before_stim -= datar.size();
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

  void TaskClassifierManager::ClassifierDecision_Handler(const double& result,
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
          case ClassificationType::STIM: return "STIM_DECISON";
          case ClassificationType::SHAM: return "SHAM_DECISON";
          default: Throw_RC_Error("Invalid classification type received.");
        }
    }();

    auto resp = MakeResp(type, task_classifier_settings.classif_id, data);
    hndl->event_log.Log(resp.Line());

    if (stim_type && stim) {
      // TODO: JPB: (need) Temporarily remove call to stimulate
      //hndl->stim_worker.Stimulate();
    }
  }

  void TaskClassifierManager::SetCallback_Handler(const TaskClassifierCallback& new_callback) {
    callback = new_callback;
  }
}
