// TODO add license to About
// https://portal.hdfgroup.org/pages/viewpage.action?pageId=50073884

#ifndef NO_HDF5

#include "HDF5Save.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "ConfigFile.h"


namespace CML {
  void HDF5Save::StartFile_Handler(const RC::RStr& filename) {
    auto conf = hndl->GetConfig();

    if (conf.elec_config.IsNull()) {
      Throw_RC_Error("Cannot save data with no channels set");
    }
    if (conf.exp_config.IsNull()) {
      Throw_RC_Error("Cannot save data with no experiment config");
    }

    channels.Resize(conf.elec_config->data.size2());
    for (size_t c=0; c<channels.size(); c++) {
      channels[c] = uint16_t(conf.elec_config->data[c][1].Get_u32() - 1); // Subtract 1 to convert to 0-indexin
    }

    StopSaving_Handler();

    hdf_hdl = new H5::H5File(filename.c_str(), H5F_ACC_TRUNC);

    int fill_value = 0;
    H5::DSetCreatPropList prop_list;
    prop_list.setFillValue(H5::PredType::NATIVE_INT16, &fill_value);

    const size_t num_dims = 2;
    hsize_t cur_dims[num_dims] = {channels.size(), 0};
    hsize_t max_dims[num_dims] = {channels.size(), H5S_UNLIMITED};

    hdf_dataspace = new H5::DataSpace(num_dims, cur_dims, max_dims);
    hdf_data = new H5::DataSet(hdf_hdl->createDataSet("data",
          H5::PredType::NATIVE_INT16, *hdf_dataspace, prop_list));
    H5::Attribute sr_attr = hdf_data->createAttribute("samplerate",
        H5::PredType::NATIVE_DOUBLE, H5::DataSpace(H5S_SCALAR));
    f64 sr_f64 = sampling_rate;
    sr_attr.write(H5::PredType::NATIVE_DOUBLE, &sr_f64);

    hndl->eeg_acq.RegisterCallback(callback_ID, SaveData);
  }


  void HDF5Save::StopSaving_Handler() {
    hndl->eeg_acq.RemoveCallback(callback_ID);

    if (hdf_hdl.IsSet()) {
      hdf_data.Delete();
      hdf_dataspace.Delete();
      hdf_hdl.Delete();
    }
  }


  void HDF5Save::SaveData_Handler(RC::APtr<const EEGData>& data) {
    auto& datar = data->data;
    if (hdf_hdl < 0) {
      StopSaving_Handler();
      return;
    }

    size_t amnt_avail = 0;
    for (size_t c=0; c<channels.size(); c++) {
      if (amnt_avail == 0) {
        amnt_avail = datar[channels[c]].size();
      }
      else {
        if (amnt_avail != datar[channels[c]].size()) {
          Throw_RC_Type(File,
              ("Data missing on hdf save, channel " + RC::RStr(c+1)).c_str());
        }
      }
    }

    if (amnt_avail == 0) {
      return;
    }

    serialize.Resize(amnt_avail*channels.size());
    size_t si=0;
    // Write data in the order of the montage CSV.
    for (size_t c=0; c<channels.size(); c++) {
      for (size_t i=0; i<amnt_avail; i++) {
        serialize[si++] = datar[channels[c]][i];
      }
    }

    const size_t num_dims = 2;
    hsize_t cur_dims[num_dims] = {channels.size(), amnt_avail};
    hsize_t max_dims[num_dims] = {channels.size(), amnt_avail};

    H5::DataSpace mem_space(num_dims, cur_dims, max_dims);
    hdf_data->write(serialize.Raw(), H5::PredType::NATIVE_INT16, mem_space,
        *hdf_dataspace);
  }
}

#endif // NO_HDF5

