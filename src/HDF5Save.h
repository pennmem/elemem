#ifndef HDF5SAVE_H
#define HDF5SAVE_H
#ifndef NO_HDF5

#include "EEGFileSave.h"
#include <H5Cpp.h>


namespace CML {
  class Handler;

  class HDF5Save : public EEGFileSave {
    public:
    HDF5Save(RC::Ptr<Handler> hndl, size_t sampling_rate)
      : EEGFileSave(hndl), sampling_rate(sampling_rate) {
      callback_ID = RC::RStr("HDF5Save_") + RC::RStr(sampling_rate);
    }

    RC::RStr GetExt() const { return "h5"; }

    protected:
    void StartFile_Handler(const RC::RStr& filename,
                           const FullConf& conf) override;
    // Thread ordering constraint:
    // Must call Stop after Start, before this destructor, and before
    // hndl->eeg_acq is deleted.
    void StopSaving_Handler() override;
    void SaveData_Handler(RC::APtr<const EEGData>& data) override;

    RC::APtr<H5::H5File> hdf_hdl;
    RC::APtr<H5::DataSet> hdf_data;
    RC::APtr<H5::DataSpace> hdf_dataspace;
    RC::Data1D<uint16_t> channels;
    RC::Data1D<int16_t> serialize;
    size_t sampling_rate;
  };
}

#endif // NO_HDF5
#endif // HDF5SAVE_H

