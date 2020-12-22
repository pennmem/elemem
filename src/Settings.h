#ifndef SETTINGS_H
#define SETTINGS_H

#include "RC/APtr.h"
#include "RC/Data1D.h"
#include "RC/RStr.h"
#include "ChannelConf.h"
#include "OPSSpecs.h"

namespace CML {
  class JSONFile;
  class CSVFile;
  class EEGChan;


  struct FullConf {
    RC::APtr<const JSONFile> exp_config;
    RC::APtr<const CSVFile> elec_config;
  };

  class StimSettings {
    public:
    CSStimChannel params;
    RC::RStr label;
    bool approved;
  };


  class Settings {
    public:

    Settings();

    void Clear();
    RC::Data1D<EEGChan> LoadElecConfig(RC::RStr dir);
    void LoadStimParamGrid();
    void LoadChannelSettings();

    void UpdateConfFR(JSONFile& current_config);
    void UpdateConfOPS(JSONFile& current_config);

    size_t GridSize() const;

    RC::APtr<const JSONFile> exp_config;
    RC::APtr<const CSVFile> elec_config;

    RC::Data1D<StimSettings> stimconf;
    RC::Data1D<StimSettings> min_stimconf;
    RC::Data1D<StimSettings> max_stimconf;

    RC::Data1D<uint16_t> stimgrid_amp_uA;
    RC::Data1D<uint32_t> stimgrid_freq_Hz;
    RC::Data1D<uint32_t> stimgrid_dur_us;

    RC::Data1D<bool> stimgrid_chan_on;
    RC::Data1D<bool> stimgrid_amp_on;
    RC::Data1D<bool> stimgrid_freq_on;
    RC::Data1D<bool> stimgrid_dur_on;

    OPSSpecs ops_specs;

    size_t stimloctest_chanind;
    size_t stimloctest_amp;
    size_t stimloctest_freq;
    size_t stimloctest_dur;

    RC::RStr exper;
    RC::RStr sub;

    bool grid_exper;
    bool task_driven;
  };
}

#endif // SETTINGS_H
