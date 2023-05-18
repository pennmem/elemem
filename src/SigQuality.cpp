#include "SigQuality.h"
#include "EEGAcq.h"
#include "Popup.h"
#include "RCmath/Stats.h"
#include <algorithm>


namespace CML {
  SigQuality::SigQuality(RC::Ptr<EEGAcq> eeg_acq)
      : eeg_acq(eeg_acq) {
  }

  SigQuality::~SigQuality() {
  }

  void SigQuality::Start_Handler() {
    amnt_processed = 0;
    target_amnt = 0;
    target_20 = 0;
    target_25 = 0;
    sampling_rate = 0;
    unwrapped.Clear();
    wrapped20.Clear();
    wrapped25.Clear();
    linenoise_frac.Clear();
    measured_freq.clear();

    Abort();
    eeg_acq->RegisterEEGMonoCallback("SigQuality", Process);
  }

  void SigQuality::Process_Handler(RC::APtr<const EEGDataRaw>& data) {
    auto& datar = data->data;

    if (sampling_rate == 0) {
      sampling_rate = data->sampling_rate;
      if ((sampling_rate % 20) || (sampling_rate % 25)) {
        Throw_RC_Error("Sampling rate must be evenly divisible by 20 and 25 "
            "for 60Hz and 50Hz signal quality checks.");
      }
      target_amnt = sampling_rate*meas_seconds;
      target_20 = sampling_rate/20;
      target_25 = sampling_rate/25;
      unwrapped.Resize(datar.size());
      wrapped20.Resize(datar.size());
      wrapped25.Resize(datar.size());

      for (size_t c=0; c<datar.size(); c++) {
        unwrapped[c].Resize(target_amnt);
        wrapped20[c].Resize(target_20);
        wrapped25[c].Resize(target_25);
      }
    }
    else {
      if (sampling_rate != data->sampling_rate) {
        Throw_RC_Error("Sampling rate changed during signal quality check!");
      }
      if (unwrapped.size() != datar.size()) {
        Throw_RC_Error("Channel count changed during signal quality check!");
      }
    }

    // Needs to be reset before reuse.
    if (amnt_processed >= target_amnt) {
      return;
    }

    size_t amnt_needed = target_amnt - amnt_processed;
    size_t amnt_to_copy = data->sample_len;
    if (amnt_to_copy > amnt_needed) {
      amnt_to_copy = amnt_needed;
    }

    size_t check_amnt = 0;
    for (size_t c=0; c<datar.size(); c++) {
      size_t ch_amnt = std::min(datar[c].size(), amnt_to_copy);
      if (ch_amnt > 0) {
        if (check_amnt == 0) {
          check_amnt = ch_amnt;
        }
        else if (ch_amnt != check_amnt) {
          Throw_RC_Error("Inconsistent channel data lengths during signal quality check!");
        }

        for (size_t i=0; i<ch_amnt; i++) {
          size_t unwr_i = amnt_processed + i;
          size_t wr20_i = unwr_i % target_20;
          size_t wr25_i = unwr_i % target_25;

          unwrapped[c][unwr_i] = datar[c][i];
          wrapped20[c][wr20_i] += datar[c][i];
          wrapped25[c][wr25_i] += datar[c][i];
        }
      }
    }

    amnt_processed += amnt_to_copy;

    if (amnt_processed > target_amnt) {
      Throw_RC_Error("Logic failure in signal quality check.");
    }
    else if (amnt_processed == target_amnt) {
      double denom20 = target_amnt / target_20;
      double denom25 = target_amnt / target_25;
      for (size_t c=0; c<wrapped20.size(); c++) {
        for (size_t i=0; i<target_20; i++) {
          wrapped20[c][i] /= denom20;
        }
        for (size_t i=0; i<target_25; i++) {
          wrapped25[c][i] /= denom25;
        }
      }

      Finalize();
    }
  }


  void SigQuality::Finalize() {
    eeg_acq->RemoveEEGMonoCallback("SigQuality");

    RC::Data1D<double> linenoise_frac20(unwrapped.size());
    RC::Data1D<double> linenoise_frac25(unwrapped.size());
    double max20 = 0;
    double max25 = 0;
    for (size_t c=0; c<unwrapped.size(); c++) {
      double sd_unwrap = RCmath::Stats::SD(unwrapped[c]);
      double sd_wrap20 = RCmath::Stats::SD(wrapped20[c]);
      double sd_wrap25 = RCmath::Stats::SD(wrapped25[c]);
      linenoise_frac20[c] = sd_wrap20 / (sd_unwrap + 1e-30);
      linenoise_frac25[c] = sd_wrap25 / (sd_unwrap + 1e-30);
      max20 = RCmath::Max(max20, linenoise_frac20[c]);
      max25 = RCmath::Max(max25, linenoise_frac25[c]);
    }
    if (max25 > max20) {
      measured_freq = "50Hz";
      linenoise_frac = linenoise_frac25;
    }
    else {
      measured_freq = "60Hz";
      linenoise_frac = linenoise_frac20;
    }

    Evaluate();
  }


  void SigQuality::Evaluate() {
    bool success = true;
    RC::Data1D<RC::RStr> report_messages;

    RC::Data1D<size_t> bad_per_bank(linenoise_frac.size()/bank_size);
    for (size_t b=0; b<bad_per_bank.size(); b++) {
      size_t bad_bank_cnt = 0;
      for (size_t i=0; i<bank_size; i++) {
        if (linenoise_frac[b*bank_size+i] > bad_line_noise) {
          bad_bank_cnt++;
        }
      }
      if (bad_bank_cnt >= bad_per_bank_thresh) {
        success = false;
        report_messages += "Size " + RC::RStr(bank_size) + " bank " +
          RC::RStr(b+1) + " has excess line noise, and might be unreferenced!";
      }
      else {
        report_messages += "Size " + RC::RStr(bank_size) + " bank " +
          RC::RStr(b+1) + " acceptable.";
      }
    }

    size_t bad_cnt = 0;
    for (size_t i=0; i<linenoise_frac.size(); i++) {
      if (linenoise_frac[i] > bad_line_noise) {
        bad_cnt++;
      }
    }
    double frac_bad = bad_cnt / double(linenoise_frac.size());
    report_messages += "Fraction of channels with excess line noise: " +
      RC::RStr(frac_bad, RC::FIXED, 3) + ".";
    if (frac_bad >= bad_chan_frac) {
      success = false;
      report_messages += "Bad channel count exceeds line noise standards!";
    }

    SigQualityResults results{linenoise_frac, report_messages,
      measured_freq, success};
    // Call the appropriate callbacks.
    for (size_t cb=0; cb<callbacks.size(); cb++) {
      callbacks[cb].callback(results);
    }
    callbacks.Clear();
  }

  /// Handler that registers a callback for the signal quality results.
  /** @param A (preferably unique) tag/name for the callback
   *  @param The callback for the completion results.
   */
  void SigQuality::RegisterCallback_Handler(const RC::RStr& tag,
      const SigResultCallback& callback) {
    RemoveCallback_Handler(tag);
    callbacks += TaggedCallback{tag, callback};
  }

  /// Handler that removes a callback for the signal quality results.
  /** @param The tag to be removed from the list of callbacks
   */
  void SigQuality::RemoveCallback_Handler(const RC::RStr& tag) {
    for (size_t i=0; i<callbacks.size(); i++) {
      if (callbacks[i].tag == tag) {
        callbacks.Remove(i);
        i--;
      }
    }
  }
}

