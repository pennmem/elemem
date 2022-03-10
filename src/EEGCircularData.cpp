#include "EEGCircularData.h"
#include "RC/Macros.h"
#include "RC/RStr.h"

namespace CML {
  RC::APtr<EEGData> EEGCircularData::GetData() {
    return GetData(circular_data_end);
  }

  RC::APtr<EEGData> EEGCircularData::GetData(size_t amnt) {
    if (amnt > circular_data.sample_len) {
      Throw_RC_Error(("The amount of data requested "
            "(" + RC::RStr(amnt) + ") " +
            "is greater than the number of samples in the ciruclar data "
            "(" + RC::RStr(circular_data.sample_len) + ")").c_str());
    }

    RC::APtr<EEGData> out_data = new EEGData(circular_data.sampling_rate, amnt);
    auto& circ_datar = circular_data.data;
    auto& out_datar = out_data->data;
    out_datar.Resize(circ_datar.size());

    RC_ForIndex(i, circ_datar) { // Iterate over channels
      auto& circ_events = circ_datar[i];
      auto& out_events = out_datar[i];

      if (circ_events.IsEmpty()) { continue; } // Skip empty channels
      out_data->EnableChan(i);

      size_t amnt_to_end = circular_data_len - circular_data_start;
      if (amnt <= amnt_to_end) {
        out_events.CopyAt(0, circ_events, circular_data_start, amnt);
      } else {
        out_events.CopyAt(0, circ_events, circular_data_start, amnt_to_end);
        size_t amnt_from_start = amnt - amnt_to_end;
        out_events.CopyAt(amnt_to_end, circ_events, 0, amnt_from_start);
      }
    }
    return out_data;
  }

  RC::APtr<EEGData> EEGCircularData::GetDataAll() {
    return GetData(circular_data_len);
  }

  /// This gets the data as a timeline, meaning that if the data isn't full yet
  /// then the 0s go before the data instead of after
  RC::APtr<EEGData> EEGCircularData::GetDataAllAsTimeline() {
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

  void EEGCircularData::PrintData() {
    RC_DEBOUT(RC::RStr("circular_data_start: ") + circular_data_start + "\n");
    RC_DEBOUT(RC::RStr("circular_data_end: ") + circular_data_end + "\n");
    GetData()->Print();
  }

  void EEGCircularData::PrintRawData() {
    RC_DEBOUT(RC::RStr("circular_data_start: ") + circular_data_start + "\n");
    RC_DEBOUT(RC::RStr("circular_data_end: ") + circular_data_end + "\n");
    circular_data.Print();
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

    // TODO: JPB: (refactor) Decide if this is how I should set the data size
    //                       (or should I pass all the info in the constructor)
    // Setup the circular EEGData to match the incoming EEGData
    if (circ_datar.IsEmpty()) {
      circ_datar.Resize(new_datar.size());
      RC_ForIndex(i, circ_datar) { // Iterate over channels
        auto& new_events = new_datar[i];
        auto& circ_events = circ_datar[i];
        if (new_events.IsEmpty()) { continue; } // Skip empty channels
        circ_events.Resize(circular_data_len);
        circ_events.Zero();
      }
    }

    if (new_data->sampling_rate != circular_data.sampling_rate)
      Throw_RC_Type(Bounds, (RC::RStr("The sampling_rate of new_data (") + new_data->sampling_rate + ") and circular_data (" + circular_data.sampling_rate + ") do not match").c_str());
    if (new_datar.size() != circ_datar.size())
      Throw_RC_Type(Bounds, (RC::RStr("The number of channels in new_data (") + new_datar.size() + ") and circular_data (" + circ_datar.size() + ") do not match").c_str());
    if (start >= new_datar.size())
      Throw_RC_Type(Bounds, (RC::RStr("The \"start\" value (") + start + ") is greater than or equal to the number of items that new_data contains (" + new_datar.size() + ")").c_str());
    if (start + amnt > new_data->sample_len)
      Throw_RC_Type(Bounds, (RC::RStr("The end value (") + (start + amnt) + ") is greater than the number of items that new_data contains (" + new_datar[0].size() + ")").c_str());
    // TODO: JPB: (feature) Log error message and write only the last buffer length of data
    if (amnt-start > circular_data_len)
      Throw_RC_Type(Bounds, (RC::RStr("Trying to write more values (") + (amnt - start) + ") into the circular_data than the circular_data contains (" + circular_data_len + ")").c_str());

    if (amnt ==  0) { return; } // Not writing any data, so skip

    size_t circ_remaining_events = circular_data_len - circular_data_end;
    size_t frst_amnt = std::min(circ_remaining_events, amnt);
    size_t scnd_amnt = std::max(int64_t(0),
        int64_t(amnt) - int64_t(frst_amnt));

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

    if (!has_wrapped && (circular_data_end + amnt >= circular_data_len)) {
      has_wrapped = true;
      if (scnd_amnt) {
        circular_data_start = (circular_data_start + scnd_amnt) % circular_data_len;
      }
    } else if (has_wrapped) {
      circular_data_start = (circular_data_start + amnt) % circular_data_len;
    }
    circular_data_end = (circular_data_end + amnt) % circular_data_len;
  }
}
