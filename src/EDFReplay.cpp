// 2021, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// Reloads edf files and provides them as if they were an EEG source.
//
/////////////////////////////////////////////////////////////////////////////

#include "EDFReplay.h"
#include "Popup.h"
#include "RC/RTime.h"
#include "RC/Errors.h"


namespace CML {
  EDFReplay::EDFReplay(RC::RStr edf_filename)
      : filename(edf_filename) {
  }


  EDFReplay::~EDFReplay() {
    Close();
  }


  void EDFReplay::Open(bool clear_buffer) {
    static bool first_run = true;
    if (!first_run) {
      Close();
    }

    int res = edfopen_file_readonly(filename.c_str(), &edf_hdr,
        EDFLIB_DO_NOT_READ_ANNOTATIONS);
    if (res < 0) {
      Throw_RC_Type(File, (RC::RStr("Could not open edf file: ") +
          filename).c_str());
    }

    edf_hdl = res;

    if (clear_buffer) {
      // Invariant match linked to GetData function.
      file_bufs.Resize(edf_hdr.edfsignals);
      channel_data.resize(file_bufs.size());

      amnt_buffered = 0;
      max_requested = 1024;
      Prebuffer();

      TimeSinceLast_samples();
    }

    if (first_run) {
      PopupWin("EDFReplay activated", "Warning");
      first_run = false;
    }
  }


  void EDFReplay::Close() {
    if (edf_hdl >= 0) {
      edfclose_file(edf_hdl);
    }
    edf_hdl = -1;
  }


  void EDFReplay::InitializeChannels(size_t sampling_rate_Hz) {
    sampling_rate = sampling_rate_Hz;
    Open();
  }


  void EDFReplay::StartingExperiment() {
    Open();
  }


  void EDFReplay::Prebuffer() {
    if (file_bufs.size() == 0) { return; }

    size_t buf_target = 2*max_requested;
    size_t smpdr = edf_hdr.signalparam[0].smp_in_datarecord;
    u32 attempt_num = 0;
    while (amnt_buffered < buf_target && (attempt_num++)<4) {
      size_t dr_cnt = (buf_target - amnt_buffered)/smpdr;

      // No space for sampling rate data records.
      if (dr_cnt < 1) {
        // Because full of them.
        if (buf_target / smpdr > 2) {
          break;
        }
        // Because buffer too small.
        else {
          max_requested *= 2;
          buf_target *= 2;
          attempt_num--;
          continue;
        }
      }


      int amnt_read_final = 0;

      for (size_t s=0; s<file_bufs.size(); s++) {
        // Down-shift existing buffer contents.
        size_t first_filled = file_bufs[s].size() - amnt_buffered;
        if (amnt_buffered > 0 && first_filled > 0) {
          file_bufs[s].CopyData(0, first_filled);
        }

        // Fill up the rest
        file_bufs[s].Resize(buf_target);
        int amnt_read = edfread_digital_samples(edf_hdl, s, smpdr,
            file_bufs[s].Raw() + amnt_buffered);

        if (amnt_read < 0) {
          Throw_RC_Type(File, "EDFReplay file read error");
        }

        file_bufs[s].Resize(amnt_buffered + amnt_read);

        if (s==0) {
          amnt_read_final = amnt_read;
        }
        else {
          if (amnt_read_final != amnt_read) {
            Throw_RC_Type(File, "Uneven amounts of data in edf channels.");
          }
        }
      }

      amnt_buffered += amnt_read_final;
      if (file_bufs.size() > 0 && file_bufs[0].size() < amnt_buffered) {
        Throw_RC_Type(File, "Reported reading more than capacity!  "
            "This should never happen.");
      }

      // Reopen the file and grab some more from the beginning if not full.
      if (size_t(amnt_read_final) != smpdr) {
        Open(false);
      }
      else {
        break;
      }
    }
  }


  int16_t EDFReplay::ClampInt(int val) {
    if (val < -32768) { return -32768; }
    if (val > 32767) { return 32767; }
    return val;
  }


  uint64_t EDFReplay::TimeSinceLast_samples() {
    static uint64_t last = uint64_t(RC::Time::Get()*sampling_rate);
    uint64_t cur = uint64_t(RC::Time::Get()*sampling_rate);
    uint64_t diff = cur - last;
    last = cur;
    return diff;
  }


  const std::vector<TrialData>& EDFReplay::GetData() {
    if (sampling_rate == 0) {
      throw std::runtime_error("Initialize channels before getting data");
    }

    if (edf_hdl < 0) {
      Throw_RC_Type(File,
          ("EDF file " + RC::RStr(filename) + " not opened.").c_str());
    }

    size_t data_len = TimeSinceLast_samples();
    max_requested = std::max(max_requested, data_len);
    if (amnt_buffered < data_len) {
      Prebuffer();
    }

    if (file_bufs.size() < 1) {
      Throw_RC_Type(File, "No channels in edf file.");
    }

    if (amnt_buffered < data_len) {
      Throw_RC_Type(File, ("Could not obtain requested " + RC::RStr(data_len)
          + " samples from " + filename).c_str());
    }

    size_t offset = file_bufs[0].size() - amnt_buffered;
    for (size_t c=0; c<channel_data.size(); c++) {
      // TODO - Set this from montage, using edf_hdr's edf_param_struct label?
      // Note, might make usage more difficult.  Consider a fallback mode.
      channel_data[c].chan = c;
      channel_data[c].data.resize(data_len);
      for (size_t d=0; d<data_len; d++) {
        channel_data[c].data[d] = ClampInt(file_bufs[c][offset + d]);
      }
    }

    amnt_buffered -= data_len;

    return channel_data;
  }
}

