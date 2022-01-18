#ifndef EEGCIRCULARDATA_H
#define EEGCIRCULARDATA_H

#include "EEGData.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace CML {
  class EEGCircularData {
    public:
    EEGCircularData(size_t sampling_rate) : circular_data(0), sampling_rate(sampling_rate) {}

    EEGData circular_data;
    size_t circular_data_start = 0;
    size_t sampling_rate = 1000;
    
    RC::APtr<EEGData> GetData();
    void PrintData();
    void PrintRawData();

    void Append(RC::APtr<const EEGData>& new_data);
    void Append(RC::APtr<const EEGData>& new_data, size_t start);
    void Append(RC::APtr<const EEGData>& new_data, size_t start, size_t amnt);

    static RC::APtr<EEGData> BinData(RC::APtr<const EEGData> in_data, size_t new_sampling_rate);
  };
}

#endif // EEGCIRCULARDATA_H

