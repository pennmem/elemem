// TODO add license to About
#include "edflib/edflib.h"
#include "EDFSave.h"
#include "EDFSynch.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "ConfigFile.h"
#include "Popup.h"

namespace CML {
  template<class F, class P>
  void EDFSave::SetChanParam(F func, P p, RC::RStr error_msg) {
    for (size_t i=0; i<channels.size(); i++) {
      if (func(edf_hdl, int(i), p)) {
        Throw_RC_Type(File, error_msg.c_str());
      }
    }
  }


  void EDFSave::StartFile_Handler(const RC::RStr& filename,
                                  const FullConf& conf) {
    if (conf.elec_config.IsNull()) {
      Throw_RC_Error("Cannot save data with no channels set");
    }
    if (conf.exp_config.IsNull()) {
      Throw_RC_Error("Cannot save data with no experiment config");
    }

    channels.Resize(conf.elec_config->data.size2());
    for (size_t c=0; c<channels.size(); c++) {
      channels[c] = uint16_t(conf.elec_config->data[c][1].Get_u32() - 1); // Subtract 1 to convert to 0-indexing
    }

    StopSaving_Handler();
    edf_hdl = EDFSynch::OpenWrite(filename.c_str(),
        EDFLIB_FILETYPE_EDFPLUS, int(channels.size()));

    if (edf_hdl < 0) {
      Throw_RC_Type(File,
          (RC::RStr("Could not open ")+filename+" for edf writing").c_str());
    }

    int datarecord_scaleby = 1;
    if (sampling_rate <= 10000) {
      datarecord_len = sampling_rate;
    }
    else {
      datarecord_scaleby = 10;
      datarecord_len = sampling_rate / datarecord_scaleby;
    }

    // Set only when needed.
    if (datarecord_scaleby != 1) {
      // duration input has units of 100us.
      if (edf_set_datarecord_duration(edf_hdl,
                                      int(100000 / datarecord_scaleby))) {
        Throw_RC_Type(File, "Could not set edf data record duration");
      }
    }

    if (edfwrite_annotation_utf8(edf_hdl, 0LL, -1LL,
        (RC::RStr("Sampling rate: ")+RC::RStr(sampling_rate)).c_str())) {
      Throw_RC_Type(File, "Could not mark edf recording start");
    }

    SetChanParam(edf_set_samplefrequency, int(datarecord_len),
        "Could not set edf sample frequency");
    SetChanParam(edf_set_digital_maximum, 32767,
        "Could not set edf digital maximum");
    SetChanParam(edf_set_digital_minimum, -32768,
        "Could not set edf digital minimum");
    SetChanParam(edf_set_physical_maximum, 32767,
        "Could not set edf physical maximum");
    SetChanParam(edf_set_physical_minimum, -32768,
        "Could not set edf physical minimum");
    SetChanParam(edf_set_physical_dimension, "250nV",
        "Could not set units");

    for (size_t c=0; c<channels.size(); c++) {
      if (edf_set_label(edf_hdl, int(c),
                        conf.elec_config->data[c][0].c_str())) {
        Throw_RC_Type(File, "Could not set edf label");
      }
    }

    if (edf_set_equipment(edf_hdl, "Elemem using Blackrock NeuroPort")) {
      Throw_RC_Type(File, "Could not set edf equipment");
    }

    std::string sub_name;

    conf.exp_config->Get(sub_name, "subject");
    if (edf_set_patientname(edf_hdl, sub_name.c_str())) {
      Throw_RC_Type(File, "Could not set edf subject name");
    }

    if (edfwrite_annotation_utf8(edf_hdl, 0LL, -1LL, "Recording starts")) {
      Throw_RC_Type(File, "Could not mark edf recording start");
    }

    amount_written = 0;
    amount_buffered = 0;
    buffer.data.Clear();
    hndl->eeg_acq.RegisterEEGMonoCallback(callback_ID, SaveData);
  }


  void EDFSave::StopSaving_Handler() {
    hndl->eeg_acq.RemoveEEGMonoCallback(callback_ID);

    if (edf_hdl >= 0) {
      // No error check, can be a destructor cleanup call.
      edfwrite_annotation_utf8(edf_hdl,
          static_cast<long long>(amount_written * 10000 / sampling_rate),
          -1LL, "Recording ends");

      EDFSynch::Close(edf_hdl);
      edf_hdl = -1;
    }
  }


  void EDFSave::SaveData_Handler(RC::APtr<const EEGData>& data) {
    auto& datar = data->data;
    if (edf_hdl < 0) {
      StopSaving_Handler();
      return;
    }

    if (buffer.data.size() < datar.size()) {
      buffer.data.Resize(datar.size());
    }

    size_t max_written = 0;
    for (size_t c=0; c<datar.size(); c++) {
      buffer.data[c] += datar[c];
      max_written = std::max(max_written, datar[c].size());
    }
    amount_buffered += max_written;

    // Must write data_record_duration at a time.  (sampling_rate by default)
    while (amount_buffered > datarecord_len) {
      // Write data in the order of the montage CSV.
      for (size_t c=0; c<channels.size(); c++) {
        if (c >= buffer.data.size()) {
          StopSaving_Handler();
          Throw_RC_Type(File, ("EDF save, configured channel " +
                RC::RStr(c+1) + " out of bounds").c_str());
        }

        if (buffer.data[channels[c]].size() < datarecord_len) {
          StopSaving_Handler();

          RC::RStr deb_msg("Data missing details\n");
          deb_msg += "sampling_rate = " + RC::RStr(sampling_rate) + ", ";
          deb_msg += "data_record_duration = " + RC::RStr(datarecord_len) + ", ";
          deb_msg += "amount_buffered = " + RC::RStr(amount_buffered) + "\n";
          for (size_t dc=0; dc<channels.size(); dc++) {
            deb_msg += RC::RStr(buffer.data[channels[dc]].size()) + " elements:  ";
            deb_msg += RC::RStr::Join(buffer.data[channels[dc]], ", ");
            deb_msg += "\n";
          }
          DebugLog(deb_msg);

          Throw_RC_Type(File,
              ("Data missing on edf save, channel " + RC::RStr(channels[c]+1)).c_str());
        }

        int write_err = edfwrite_digital_short_samples(edf_hdl,
            buffer.data[channels[c]].Raw());
        if (write_err) {
          StopSaving_Handler();
          Throw_RC_Type(File, ("Could not save data to edf file, error code" +
                        RC::RStr(write_err)).c_str());
        }
      }

      // Move remaining data in buffer to beginning and efficiently shrink.
      for (size_t c=0; c<buffer.data.size(); c++) {
        if (buffer.data[c].size() < datarecord_len) {
          continue;
        }
        if (buffer.data[c].size() > datarecord_len) {
          // Not else, can't copy if == data_record_duration.
          buffer.data[c].CopyData(0, datarecord_len);
        }
        buffer.data[c].Resize(buffer.data[c].size()-datarecord_len);
      }
      amount_buffered -= datarecord_len;
      amount_written += datarecord_len;
    }

    buffer.sample_len = amount_buffered; // Keep the buffer sample_len in line with the manual editing
  }
}

