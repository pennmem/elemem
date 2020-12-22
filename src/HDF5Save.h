#ifndef HDF5SAVE_H
#define HDF5SAVE_H

#include "EEGData.h"
#include "RC/File.h"
#include "RC/Ptr.h"
#include "RCqt/Worker.h"

namespace H5{
  class H5File;
  class DataSet;
  class DataSpace;
}

namespace CML {
  class Handler;

  class HDF5Save : public RCqt::WorkerThread {
    public:
    HDF5Save(RC::Ptr<Handler> hndl);
    ~HDF5Save();

    // Rule of 3.
    HDF5Save(const HDF5Save&) = delete;
    HDF5Save& operator=(const HDF5Save&) = delete;

    RCqt::TaskCaller<const RC::RStr> StartFile =
      TaskHandler(HDF5Save::StartFile_Handler);
    RCqt::TaskCaller<> StopSaving =
      TaskHandler(HDF5Save::StopSaving_Handler);

    RCqt::TaskCaller<RC::APtr<const EEGData>> SaveData =
      TaskHandler(HDF5Save::SaveData_Handler);

    protected:
    void StartFile_Handler(const RC::RStr& filename);
    void StopSaving_Handler();
    void SaveData_Handler(RC::APtr<const EEGData>& data);

    template<class F, class P>
    void SetChanParam(F func, P p, RC::RStr error_msg);

    RC::APtr<H5::H5File> hdf_hdl;
    RC::APtr<H5::DataSet> hdf_data;
    RC::APtr<H5::DataSpace> hdf_dataspace;
    RC::Ptr<Handler> hndl;
    RC::Data1D<uint8_t> channels;
    RC::Data1D<int16_t> serialize;
    size_t sampling_rate = 1000;
  };
}

#endif // HDF5SAVE_H

