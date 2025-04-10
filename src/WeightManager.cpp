#include "WeightManager.h"

namespace CML {

  WeightManager::WeightManager(RC::RStr classif_json, RC::APtr<const CSVFile> elec_config) {
    auto jf = JSONFile(classif_json);

    auto weights_mut = RC::MakeAPtr<FeatureWeights>();
    RC::Data1D<double> coef1d;
    RC::Data1D<RC::RStr> chanstr;
    RC::Data1D<RC::RStr> dims;

    jf.Get(weights_mut->intercept, "intercept_", 0);
    jf.Get(coef1d, "coef_", 0);
    jf.Get(weights_mut->freqs, "coords", "frequency");
    jf.Get(chanstr, "coords", "channel");
    jf.Get(dims, "dims");

    auto& chansr = weights_mut->chans;
    chansr.Resize(chanstr.size());

    // Look up bipolar pairs in montage and assign them in weights_mut.
    for (size_t i=0; i<weights_mut->chans.size(); i++) {
      auto pairs = chanstr[i].SplitAny("_-");
      if (pairs.size() != 2) {
        Throw_RC_Error(("Bipolar pairs must contain monopolar channel labels "
            "split by an underscore (or dash), and \"" + chanstr[i] +
            "\" does not.").c_str());
      }

      auto& datar = elec_config->data;

      if (datar.size2() == 0 || datar.size1() < 2) {
        Throw_RC_Type(File, "Electrode configuration does not contain label "
            "and channel number entries.");
      }


      int64_t pos = -1;
      int64_t neg = -1;
      auto CompAsgn = [](const auto& check, const RC::RStr& contact,
                          auto& pn) {
        if (check[0] == contact) {
          if (pn != -1) {
            Throw_RC_Type(File, ("Multiple entries for " + contact + " in "
                "montage file.").c_str());
          }
          check[1].Get(pn);
          if (pn <= 0) {
            Throw_RC_Type(File, ("Montage entry for " + contact + " must have "
                          "channel of 1 or greater.").c_str());
          }
          pn--;  // Becomes BipolarPair later, 0 indexed.
        }
      };
      for (size_t e=0; e<datar.size2(); e++) {
        CompAsgn(datar[e], pairs[0], pos);
        CompAsgn(datar[e], pairs[1], neg);
      }

      if (pos < 0 || neg < 0) {
        Throw_RC_Type(File, ("Could not find bipolar pairs for " + chanstr[i] +
              " in montage file.").c_str());
      }

      chansr[i].pos = pos;
      chansr[i].neg = neg;
    }

    // Convert coef1d to 2D coefficients;
    auto& coefr = weights_mut->coef;
    size_t chancnt = chansr.size();
    size_t freqcnt = weights_mut->freqs.size();
    coefr.Resize(chancnt, freqcnt);
    if (coef1d.size() != coefr.size1() * coefr.size2()) {
      Throw_RC_Type(File, ("1D features matrix in " + classif_json + " has "
            "incorrect length for the number of frequencies and "
            "channels.").c_str());
    }
    
    if (!dims[0].compare("frequency")) {
      for (size_t i=0; i<coef1d.size(); i++) {
        coefr[i/chancnt][i%chancnt] = coef1d[i];
      }
    } else if (!dims[0].compare("channel") || !dims[0].compare("pair")) {
      for (size_t i=0; i<coef1d.size(); i++) {
        coefr[i%freqcnt][i/freqcnt] = coef1d[i];
      }
    } else {
      Throw_RC_Error(("Unknown outer dimension for coefficients: " + dims[0]).c_str());
    }

    weights = weights_mut.ExtractConst();
  }
}

