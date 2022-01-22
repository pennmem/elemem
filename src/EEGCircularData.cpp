#include "EEGCircularData.h"
#include "RC/Macros.h"
#include "RC/RStr.h"

namespace CML {
    RC::APtr<EEGData> EEGCircularData::GetData() {
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

    void EEGCircularData::PrintData() {
      PrintEEGData(*GetData());
    }

    void EEGCircularData::PrintRawData() {
      RC_DEBOUT(RC::RStr("circular_data_start: ") + circular_data_start + "\n");
      PrintEEGData(circular_data);
    }

    void EEGCircularData::Append(RC::APtr<const EEGData>& new_data) {
    size_t start = 0;
    size_t amnt = new_data->data[0].size();
    Append(new_data, start, amnt);
  }

  void EEGCircularData::Append(RC::APtr<const EEGData>& new_data, size_t start) {
    size_t amnt = new_data->data[0].size() - start;
    Append(new_data, start, amnt);
  }

  void EEGCircularData::Append(RC::APtr<const EEGData>& new_data, size_t start, size_t amnt) {
    auto& new_datar = new_data->data;
    auto& circ_datar = circular_data.data;

    // TODO: JPB: (refactor) Decide if this is how I should set the sampling_rate and data size in Append 
    //                       (or should I pass all the info in the constructor)
    // Setup the circular EEGData to match the incoming EEGData
    if (circular_data.sampling_rate == 0) {
      circular_data.sampling_rate = new_data->sampling_rate;
      circ_datar.Resize(new_datar.size());
      RC_ForIndex(i, circ_datar) { // Iterate over channels
        // TODO: JPB: (need) Make the circular buffer size configurable
        circ_datar[i].Resize(100);
      }
    }

    if (new_datar.size() > circ_datar.size())
      Throw_RC_Type(Bounds, "The number of channels in new_data and circular_data do not match");
    if (start > new_datar.size() - 1)
      Throw_RC_Type(Bounds, "The \"start\" value is larger than the number of items that new_data contains");
    if (start + amnt > new_datar[0].size())
      Throw_RC_Type(Bounds, "The end value is larger than the number of items that new_data contains");
    // TODO: JPB: (feature) Log error message and write only the last buffer length of data
    if (new_datar[0].size() > circ_datar[0].size())
      Throw_RC_Type(Bounds, "Trying to write more values into the circular_data than the circular_data contains");

    if (amnt ==  0) { return; } // Not writing any data, so skip

    RC_ForIndex(i, circ_datar) { // Iterate over channels
      auto& circ_events = circ_datar[i];
      auto& new_events = new_datar[i];
      size_t circ_remaining_events = circ_events.size() - circular_data_start;

      if (new_events.IsEmpty()) { continue; }

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
  }

  RC::APtr<EEGData> EEGCircularData::BinData(RC::APtr<const EEGData> in_data, size_t new_sampling_rate) {
    // TODO: JPB: (feature) Add ability to handle sampling ratios that aren't true multiples
    RC::APtr<EEGData> out_data = new EEGData(new_sampling_rate);
    size_t sampling_ratio = in_data->sampling_rate / new_sampling_rate;
    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    out_datar.Resize(in_datar.size());

    auto accum_event = [](u32 sum, size_t val) { return std::move(sum) + val; };
    RC_ForIndex(i, out_datar) { // Iterate over channels
      auto& in_events = in_datar[i];
      auto& out_events = out_datar[i];

      if (in_events.IsEmpty()) { continue; }
      size_t out_events_size = in_events.size() / sampling_ratio + 1;
      out_events.Resize(out_events_size);
      RC_ForIndex(j, out_events) {
        if (j < out_events_size - 1) {
          size_t start = j * sampling_ratio;
          size_t end = (j+1) * sampling_ratio - 1;
          size_t items = sampling_ratio;
          out_events[j] = std::accumulate(&in_events[start], &in_events[end]+1,
                            0, accum_event) / items;
        } else { // Last block could have leftover samples
          size_t start = j * sampling_ratio;
          size_t end = in_events.size() - 1;
          size_t items = std::distance(&in_events[start], &in_events[end]+1);
          RC_DEBOUT(items);
          out_events[j] = std::accumulate(&in_events[start], &in_events[end]+1,
                            0, accum_event) / items;
        }
      }
    }

    return out_data;
  }
}
