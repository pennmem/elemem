#include "EEGCircularData.h"
#include "RC/Macros.h"
#include "RC/RStr.h"

namespace CML {
    RC::APtr<EEGData> EEGCircularData::GetData() {
      RC::APtr<EEGData> out_data = new EEGData(circular_data.sampling_rate, circular_data.sample_len);
      auto& circ_datar = circular_data.data;
      auto& out_datar = out_data->data;
      out_datar.Resize(circ_datar.size());
      RC_ForIndex(i, circ_datar) { // Iterate over channels
        auto& circ_events = circ_datar[i];
        auto& out_events = out_datar[i];

        if (circ_events.IsEmpty()) { continue; } // Skip empty channels
        out_data->EnableChan(i);

        size_t amnt = circ_events.size() - circular_data_end;
        out_events.CopyAt(0, circ_events, circular_data_end, amnt);
        out_events.CopyAt(amnt, circ_events, 0, circular_data_end);
      }   
      return out_data;
    }

    // TODO: JPB: (feature) Implement EEGCircularData::GetData(size_t amnt)
    //            Should be very similar to above
    //            Refactor above to use this too
    RC::APtr<EEGData> EEGCircularData::GetData(size_t amnt) {
      RC::APtr<EEGData> out_data = new EEGData(circular_data.sampling_rate, circular_data.sample_len);
      return out_data;
    }

    void EEGCircularData::PrintData() {
      RC_DEBOUT(RC::RStr("circular_data_start: ") + circular_data_start + "\n");
      RC_DEBOUT(RC::RStr("circular_data_end: ") + circular_data_end + "\n");
      PrintEEGData(*GetData());
    }

    void EEGCircularData::PrintRawData() {
      RC_DEBOUT(RC::RStr("circular_data_start: ") + circular_data_start + "\n");
      RC_DEBOUT(RC::RStr("circular_data_end: ") + circular_data_end + "\n");
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

  /// Appends data to the circular_buffer
  /** @param new_data New data to add data from
    * @param start The start location in the new_data
    * @param amnt The amount of data from new_data to be added
    */
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
        auto& new_events = new_datar[i];
        auto& circ_events = circ_datar[i];
        if (new_events.IsEmpty()) { continue; } // Skip empty channels
        circ_events.Resize(circular_data_len);
        circ_events.Zero();
      }
    }

    if (new_datar.size() > circ_datar.size())
      Throw_RC_Type(Bounds, (RC::RStr("The number of channels in new_data (") + new_datar.size() + ") and circular_data (" + circ_datar.size() + ") do not match").c_str());
    if (start > new_datar.size() - 1)
      Throw_RC_Type(Bounds, (RC::RStr("The \"start\" value (") + start + ") is larger than the number of items that new_data contains (" + (new_datar.size()-1) + ")").c_str());
    if (start + amnt > new_datar[0].size()) // TODO: JPB: (Need) How to handle if first channel is empty
      Throw_RC_Type(Bounds, (RC::RStr("The end value (") + (start+amnt) + ") is larger than the number of items that new_data contains (" + new_datar[0].size() + ")").c_str());
    // TODO: JPB: (feature) Log error message and write only the last buffer length of data
    if (new_datar[0].size() > circular_data_len)
      Throw_RC_Type(Bounds, (RC::RStr("Trying to write more values (") + new_datar[0].size() + ") into the circular_data than the circular_data contains (" + circular_data_len + ")").c_str());

    if (amnt ==  0) { return; } // Not writing any data, so skip

    size_t circ_remaining_events = circular_data_len - circular_data_end;
    size_t frst_amnt = std::min(circ_remaining_events, amnt);
    size_t scnd_amnt = std::max(0, (int)amnt - (int)frst_amnt);
    
    RC_ForIndex(i, circ_datar) { // Iterate over channels
      auto& new_events = new_datar[i];
      auto& circ_events = circ_datar[i];

      if (new_events.IsEmpty()) { continue; } // Skip empty channels

      // Copy the data up to the end of the Data1D (or all the data, if possible)
      circ_events.CopyAt(circular_data_end, new_events, start, frst_amnt);

      // Copy the remaining data at the beginning of the Data1D
      if (scnd_amnt)
        circ_events.CopyAt(0, new_events, start+frst_amnt, scnd_amnt);
    }
    
    if (!has_wrapped && (circular_data_end + amnt >= circular_data_len))
      has_wrapped = true;
    if (has_wrapped)
      circular_data_start = (circular_data_start + amnt) % circular_data_len;
    circular_data_end = (circular_data_end + amnt) % circular_data_len;
  }

  RC::APtr<EEGData> EEGCircularData::BinData(RC::APtr<const EEGData> in_data, size_t new_sampling_rate) {
    // TODO: JPB: (feature) Add ability to handle sampling ratios that aren't true multiples
    size_t sampling_ratio = in_data->sampling_rate / new_sampling_rate;
    // This is integer division that returns the ceiling
    size_t new_sample_len = in_data->sample_len / sampling_ratio + (in_data->sample_len % sampling_ratio != 0);
    RC::APtr<EEGData> out_data = new EEGData(new_sampling_rate, new_sample_len);

    auto& in_datar = in_data->data;
    auto& out_datar = out_data->data;
    out_datar.Resize(in_datar.size());

    if (new_sampling_rate == 0)
      Throw_RC_Type(Bounds, "New binned sampling rate cannot be 0");

    auto accum_event = [](u32 sum, size_t val) { return std::move(sum) + val; };
    RC_ForIndex(i, out_datar) { // Iterate over channels
      auto& in_events = in_datar[i];
      auto& out_events = out_datar[i];

      if (in_events.IsEmpty()) { continue; }
      out_data->EnableChan(i);

      RC_ForIndex(j, out_events) {
        if (j < in_events.size() - 1) {
          size_t start = j * sampling_ratio;
          size_t end = (j+1) * sampling_ratio - 1;
          size_t items = sampling_ratio;
          out_events[j] = std::accumulate(&in_events[start], &in_events[end]+1,
                            0, accum_event) / items;
        } else { // Last block could have leftover samples
          size_t start = j * sampling_ratio;
          size_t end = in_events.size() - 1;
          size_t items = std::distance(&in_events[start], &in_events[end]+1);
          out_events[j] = std::accumulate(&in_events[start], &in_events[end]+1,
                            0, accum_event) / items;
        }
      }
    }

    return out_data;
  }
}
