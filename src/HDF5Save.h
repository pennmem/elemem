#ifndef HDF5SAVE_H
#define HDF5SAVE_H

#include "EEGFileSave.h"
#include <H5Cpp.h>


namespace CML {
  class Handler;

  class HDF5Save : public EEGFileSave {
    public:
    HDF5Save(RC::Ptr<Handler> hndl) : EEGFileSave(hndl) {}

    protected:
    void StartFile_Handler(const RC::RStr& filename) override;
    void StopSaving_Handler() override;
    void SaveData_Handler(RC::APtr<const EEGData>& data) override;

    RC::APtr<H5::H5File> hdf_hdl;
    RC::APtr<H5::DataSet> hdf_data;
    RC::APtr<H5::DataSpace> hdf_dataspace;
    RC::Data1D<uint8_t> channels;
    RC::Data1D<int16_t> serialize;
    size_t sampling_rate = 1000;
  };
}

#endif // HDF5SAVE_H

