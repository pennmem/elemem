// TODO add license to About
#include "edflib/edflib.h"
#include "EDFSave.h"
#include "EEGAcq.h"
#include "Handler.h"
#include "ConfigFile.h"

namespace CML {
  EDFSave::EDFSave(RC::Ptr<Handler> hndl)
    : hndl(hndl) {
  }

  EDFSave::~EDFSave() {
    StopSaving_Handler();
  }

  void EDFSave::StartFile_Handler(RC::RStr& filename) {
    if (hndl->elec_config.IsNull()) {
      Throw_RC_Error("Cannot save data with no channels set");
    }
    if (hndl->exp_config.IsNull()) {
      Throw_RC_Error("Cannot save data with no experiment config");
    }

    channels.Resize(hndl->elec_config->data.size2());
    for (size_t c=0; c<channels.size(); c++) {
      channels[c] = uint8_t(hndl->elec_config->data[c][1].Get_u32() - 1);
    }

    edf_hdl = edfopen_file_writeonly(filename.c_str(),
        EDFLIB_FILETYPE_EDFPLUS, int(channels.size()));

    if (edf_hdl < 0) {
      Throw_RC_Type(File,
          (RC::RStr("Could not open ")+filename+" for edf writing").c_str());
    }

    SetChanParam(edf_set_samplefrequency, int(sampling_rate),
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
    
    if (edf_set_label(edf_hdl, 0, RC::Time::GetDateTime().c_str())) {
      Throw_RC_Type(File, "Could not set edf label");
    }

    if (edf_set_equipment(edf_hdl, "Elemem using Blackrock NeuroPort")) {
      Throw_RC_Type(File, "Could not set edf equipment");
    }
    
    std::string sub_name;
    hndl->exp_config->Get(sub_name, "subject");
    if (edf_set_patientname(edf_hdl, sub_name.c_str())) {
      Throw_RC_Type(File, "Could not set edf subject name");
    }

    if (edfwrite_annotation_utf8(edf_hdl, 0LL, -1LL, "Recording starts")) {
      Throw_RC_Type(File, "Could not mark edf recording start");
    }

    amount_written = 0;
    amount_buffered = 0;
    buffer.Clear();
    hndl->eeg_acq.RegisterCallback("EDFSave", SaveData);
  }


  void EDFSave::StopSaving_Handler() {
    hndl->eeg_acq.RemoveCallback("EDFSave");

    if (edf_hdl >= 0) {
      // No error check, can be a destructor cleanup call.
      edfwrite_annotation_utf8(edf_hdl,
          static_cast<long long>(amount_written * 10000 / sampling_rate),
          -1LL, "Recording ends");

      edfclose_file(edf_hdl);
      edf_hdl = -1;
    }
  }


  void EDFSave::SaveData_Handler(RC::APtr<const EEGData>& data) {
    auto& datar = *data;
    if (edf_hdl < 0) {
      StopSaving_Handler();
      return;
    }

    if (buffer.size() < datar.size()) {
      buffer.Resize(datar.size());
    }

    size_t max_written = 0;
    for (size_t c=0; c<datar.size(); c++) {
      buffer[c] += datar[c];
      max_written = std::max(max_written, datar[c].size());
    }
    amount_buffered += max_written;

    // Must write sampling frequency at a time.
    while (amount_buffered > sampling_rate) {
      // Write data in the order of the montage CSV.
      for (size_t c=0; c<channels.size(); c++) {
        if (c >= buffer.size()) {
          StopSaving_Handler();
          Throw_RC_Type(File, ("EDF save, configured channel " +
                RC::RStr(c+1) + " out of bounds").c_str());
        }

        if (buffer[channels[c]].size() < sampling_rate) {
          StopSaving_Handler();
          Throw_RC_Type(File,
              ("Data missing on edf save, channel " + RC::RStr(c+1)).c_str());
        }

        if (edfwrite_digital_short_samples(edf_hdl,
              buffer[channels[c]].Raw())) {
          StopSaving_Handler();
          Throw_RC_Type(File, "Could not save data to edf file");
        }
      }

      // Move remaining data in buffer to beginning and efficiently shrink.
      for (size_t c=0; c<buffer.size(); c++) {
        if (buffer[c].size() < sampling_rate) {
          continue;
        }
        buffer[c].CopyData(0, sampling_rate);
        buffer[c].Resize(buffer[c].size()-sampling_rate);
      }
      amount_buffered -= sampling_rate;
      amount_written += sampling_rate;
    }
  }


  template<class F, class P>
  void EDFSave::SetChanParam(F func, P p, RC::RStr error_msg) {
    for (size_t i=0; i<channels.size(); i++) {
      if (func(edf_hdl, int(i), p)) {
        Throw_RC_Type(File, error_msg.c_str());
      }
    }
  }
}

