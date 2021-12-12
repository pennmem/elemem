#include "ClassificationData.h"
#include "RC/Macros.h"
#include "Handler.h"
#include "EEGAcq.h"
#include "Classifier.h"
#include <unordered_map>

namespace CML {
  // TODO: JPB: Remove sampling_rate
  ClassificationData::ClassificationData(RC::Ptr<Handler> hndl, int sampling_rate)
      : hndl(hndl), circular_data(0) {
    callback_ID = RC::RStr("ClassificationData_") + sampling_rate;
    hndl->eeg_acq.RegisterCallback(callback_ID, ClassifyData);

    // Test Code
    //RC::APtr<EEGData> data = new EEGData(10);
    //data->data.Resize(1);
    //data->data[0].Append({0,1,2,3,4,5,6,7,8,9,10});
    //RC::APtr<const EEGData> binned_data = BinData(data.ExtractConst(), 3).ExtractConst();
    //auto& datar = binned_data->data;
    //RC::RStr deb_msg = RC::RStr("Data\nStart: ") + circular_data_start + "\n";
    //RC_ForIndex(c, datar)
    //  deb_msg += "Channel " + RC::RStr(c) + ": " + RC::RStr::Join(datar[c], ", ") + "\n"; 
    //deb_msg += "\n\n";
    //RC_DEBOUT(deb_msg);
  }

  void ClassificationData::UpdateCircularBuffer(RC::APtr<const EEGData>& new_data) {
    size_t start = 0;
    size_t amnt = new_data->data.size();
    UpdateCircularBuffer(new_data, start, amnt);
  }

  void ClassificationData::UpdateCircularBuffer(RC::APtr<const EEGData>& new_data, size_t start) {
    size_t amnt = new_data->data.size() - start;
    UpdateCircularBuffer(new_data, start, amnt);
  }

  void ClassificationData::UpdateCircularBuffer(RC::APtr<const EEGData>& new_data, size_t start, size_t amnt) {
    auto& new_datar = new_data->data;
    auto& circ_datar = circular_data.data;

    // TODO: JPB: Decide if this is how I should set the sampling_rate and data size (or should I pass all the info in)
    if (circular_data.sampling_rate == 0) { // Setup the circular EEGData to match the incoming EEGData
      circular_data.sampling_rate = new_data->sampling_rate;
      circ_datar.Resize(new_datar.size());
    }

    if (start > new_datar.size() - 1)
      Throw_RC_Type(Bounds, "The \"start\" value is larger than the number of items that new_data contains");
    if (start + amnt > new_datar.size())
      Throw_RC_Type(Bounds, "The end value is larger than the number of items that new_data contains");
    if (new_datar.size() > circ_datar.size()) // TODO: JPB: Log error message and write only the last buffer length of data
      Throw_RC_Type(Bounds, "Trying to write more values into the circular_data than the circular_data contains");

    if (amnt ==  0) { return; } // Not writing any data, so skip
    
    RC_ForIndex(i, circ_datar) { // Iterate over channels
      auto& circ_events = circ_datar[i];
      auto& new_events = new_datar[i];
      size_t circ_remaining_events = circ_events.size() - circular_data_start;
      // Copy the data up to the end of the Data1D (or all the data, if possible)
      size_t frst_amnt = std::min(circ_remaining_events, amnt);
      circ_events.CopyAt(circular_data_start, new_events, start, frst_amnt);
      circular_data_start += frst_amnt;
      if (circular_data_start == circ_events.size())
        circular_data_start = 0;
      // Copy the remaining data at the beginning of the Data1D
      int scnd_amnt = (int)amnt - (int)frst_amnt;
      if (scnd_amnt > 0) {
        circ_events.CopyAt(0, new_events, start+frst_amnt, scnd_amnt);
        circular_data_start += scnd_amnt;
      }
    }
    
    //PrintCircularBuffer();
  }

  void ClassificationData::PrintCircularBuffer() {
    auto data = GetCircularBufferData();
    auto& datar = data->data;    
    RC::RStr deb_msg = RC::RStr("Data\nStart: ") + circular_data_start + "\n";
    RC_ForIndex(c, datar)
      deb_msg += "Channel " + RC::RStr(c) + ": " + RC::RStr::Join(datar[c], ", ") + "\n"; 
    deb_msg += "\n\n";
    RC_DEBOUT(deb_msg);
  }

  RC::APtr<EEGData> ClassificationData::GetCircularBufferData() {
    RC::APtr<EEGData> out_data = new EEGData(circular_data.sampling_rate);
    auto& circ_datar = circular_data.data;
    auto& out_datar = out_data->data;
    out_datar.Resize(circ_datar.size());
    RC_ForIndex(i, circ_datar) { // Iterate over channels
      auto& circ_events = circ_datar[i];
      auto& out_events = out_datar[i];
      out_events.Resize(circ_events.size());
      size_t amnt = circ_events.size() - circular_data_start;
      out_events.CopyAt(0, circ_events, circular_data_start, amnt);
      out_events.CopyAt(amnt, circ_events, 0, circular_data_start);
    }
    return out_data;
  }

  RC::APtr<EEGData> ClassificationData::BinData(RC::APtr<const EEGData> in_data, size_t new_sampling_rate) {
    // TODO: JPB: Add ability to handle sampling ratios that aren't true multiples
    RC::APtr<EEGData> out_data = new EEGData(new_sampling_rate);
    size_t sampling_ratio = in_data->sampling_rate / new_sampling_rate;
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    out_datar.Resize(in_datar.size());
    
    auto accum_event = [](u32 sum, size_t val) { return std::move(sum) + val; };
    RC_ForIndex(i, out_datar) { // Iterate over channels
      auto& in_events = in_datar[i];
      auto& out_events = out_datar[i];
      size_t out_events_size = in_events.size() / sampling_ratio + 1;
      out_events.Resize(out_events_size);
      RC_ForIndex(j, out_events) {
        if (j < out_events_size - 1) { 
          size_t start = j * sampling_ratio;
          size_t end = (j+1) * sampling_ratio - 1;
          size_t items = sampling_ratio;
          out_events[j] = std::accumulate(&in_events[start], &in_events[end]+1, 0, accum_event) / items;
        } else { // Last block could have leftover samples
          size_t start = j * sampling_ratio;
          size_t end = in_events.size() - 1;
          size_t items = std::distance(&in_events[start], &in_events[end]+1);
          RC_DEBOUT(items);
          out_events[j] = std::accumulate(&in_events[start], &in_events[end]+1, 0, accum_event) / items;
        }
      }
    }
    
    return out_data;
  }

  void ClassificationData::StartClassification() {
    stim_event_waiting = false;
    num_eeg_events_before_stim = 0;
    // TODO: JPB: if (!(eventLoop has too many waiting data points)) // skip classification to catch up
    RC::APtr<const EEGData> data = GetCircularBufferData().ExtractConst();
    // TODO: JPB: Get the new_sampling_rate from the config file
    RC::APtr<const EEGData> binned_data = BinData(data, 100).ExtractConst();

    auto& datar = binned_data->data;
    RC::RStr deb_msg = RC::RStr("Data\nStart: ") + circular_data_start;
    RC_ForIndex(c, datar)
      deb_msg += "Channel " + RC::RStr(c) + ": " + RC::RStr::Join(datar[c], ", ") + "\n"; 
    deb_msg += "\n\n";
    RC_DEBOUT(deb_msg);

    callback(binned_data);
  }

  void ClassificationData::ClassifyData_Handler(RC::APtr<const EEGData>& data) {
    //RC_DEBOUT(RC::RStr("ClassificationData_Handler\n"));
    auto& datar = data->data;

    if (stim_event_waiting) {
      size_t num_eeg_events_before_stim = this->num_eeg_events_before_stim;
      if (num_eeg_events_before_stim <= datar.size()) {
        UpdateCircularBuffer(data, 0, num_eeg_events_before_stim);
        StartClassification();
        UpdateCircularBuffer(data, num_eeg_events_before_stim);
      } else { // num_eeg_events_before_stim > datar.size()
        UpdateCircularBuffer(data);
        num_eeg_events_before_stim -= datar.size();
      }
    } else {
      UpdateCircularBuffer(data);
    }
  }

  void ClassificationData::ProcessTaskClassifierEvent_Handler(const RC::RStr& event) {
    if (event == "CLSTIM") {
      if (!stim_event_waiting) {
        stim_event_waiting = true;
        num_eeg_events_before_stim = 10;
      } else {
        // TODO: JPB: Allow classifier to start gather EEGData on top of the other one
        hndl->event_log.Log("Skipping stim event, another stim event is already waiting (collecting EEGData)");
      }
    }
  }
  
  void ClassificationData::ClassifierDecision_Handler(const double& result) {
    //RC_DEBOUT(RC::RStr("ClassifierDecision_Handler\n\n"));
    bool stim = result > 0.5;

    hndl->event_log.Log(RC::RStr("Sham: ") + task_classifier_settings.sham);
    hndl->event_log.Log(RC::RStr("Stim: ") + stim);

    if (stim && !task_classifier_settings.sham) {
      //hndl->stim_worker.Stimulate();
    }
  }
  
  void ClassificationData::SetCallback_Handler(const EEGCallback& new_callback) {
    callback = new_callback;
  }
}
