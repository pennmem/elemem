#ifndef EEGCIRCULARDATA_H
#define EEGCIRCULARDATA_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class EEGCircularData {
    public:
    EEGCircularData(size_t sampling_rate, size_t duration_ms)
      : sampling_rate(sampling_rate), duration_ms(duration_ms),
      circular_data_len(duration_ms * sampling_rate / 1000),
      circular_data(sampling_rate, circular_data_len) {
    }

    // TODO: JPB: (refactor) Reuse member variables of EEGData
    size_t sampling_rate = 0;
    size_t duration_ms = 0;
    size_t circular_data_len = 1000;  // Set this in constructor

    EEGDataDouble circular_data;
    size_t circular_data_start = 0;
    size_t circular_data_end = 0;
    bool has_wrapped = false;

    RC::APtr<EEGDataDouble> GetRecentData(size_t amnt);

    RC::APtr<EEGDataDouble> GetData();
    RC::APtr<EEGDataDouble> GetData(size_t amnt);
    RC::APtr<EEGDataDouble> GetDataAll();
    RC::APtr<EEGDataDouble> GetDataAllAsTimeline();
    void PrintData();
    void PrintRawData();

    void Append(RC::APtr<const EEGDataDouble>& new_data);
    void Append(RC::APtr<const EEGDataDouble>& new_data, size_t start);
    void Append(RC::APtr<const EEGDataDouble>& new_data, size_t start, size_t amnt);
  };
}

#endif // EEGCIRCULARDATA_H

