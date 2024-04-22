#ifndef SETTINGS_H
#define SETTINGS_H

#include "RC/APtr.h"
#include "RC/Data1D.h"
#include "RC/RStr.h"
#include "ChannelConf.h"
#include "OPSSpecs.h"
#include "CPSSpecs.h"
#include "WeightManager.h"

namespace CML {
  class JSONFile;
  class CSVFile;
  class EEGChan;


  struct FullConf {
    RC::APtr<const JSONFile> exp_config;
    RC::APtr<const CSVFile> elec_config;
    RC::APtr<const CSVFile> bipolar_config;
  };

  class StimSettings {
    public:
    StimChannel params;
    RC::RStr label;
    RC::RStr stimtag;
    bool approved;
  };


  class Settings {
    public:

    Settings();

    void LoadSystemConfig();

    void Clear();
    RC::Data1D<EEGChan> LoadElecConfig(RC::RStr dir);
    bool BipolarElecConfigUsed();
    RC::Data1D<EEGChan> LoadBipolarElecConfig(RC::RStr dir, RC::Data1D<EEGChan> mono_chans);
    void LoadStimParamGrid();
    void LoadStimParamsCPS();
    void LoadChannelSettings();

    void UpdateConfFR(JSONFile& current_config);
    void UpdateConfOPS(JSONFile& current_config);
    RC::Data1D<RC::RStr> UpdateConfCPS(JSONFile& current_config, const RC::RStr& elemem_dir,
                       const RC::RStr& session_dir, const RC::RStr& current_elec_config_filename);

    size_t GridSize() const;

    RC::APtr<const JSONFile> sys_config;

    RC::APtr<const JSONFile> exp_config;
    RC::APtr<const CSVFile> elec_config;
    RC::APtr<const CSVFile> bipolar_config;

    RC::Data1D<StimSettings> stimconf;
    RC::Data1D<StimSettings> min_stimconf;
    RC::Data1D<StimSettings> max_stimconf;
    // min and max ranges for parameter search (min/max_stimconf give ranges for stim parameter configuration)
    RC::Data1D<StimSettings> min_stimconf_range;
    RC::Data1D<StimSettings> max_stimconf_range;

    RC::Data1D<uint16_t> stimgrid_amp_uA;
    RC::Data1D<uint32_t> stimgrid_freq_Hz;
    RC::Data1D<uint32_t> stimgrid_dur_us;

    RC::Data1D<bool> stimgrid_chan_on;
    RC::Data1D<bool> stimgrid_amp_on;
    RC::Data1D<bool> stimgrid_freq_on;
    RC::Data1D<bool> stimgrid_dur_on;

    RC::Data2D<double> butter_freq_bands;
    RC::APtr<WeightManager> weight_manager;
    OPSSpecs ops_specs;
    CPSSpecs cps_specs;

    size_t stimloctest_chanind;
    size_t stimloctest_amp;
    size_t stimloctest_freq;
    size_t stimloctest_dur;

    size_t macro_sampling_rate;
    size_t micro_sampling_rate;
    size_t sampling_rate;
    size_t binned_sampling_rate;

    RC::RStr exper;
    RC::RStr sub;

    bool grid_exper;
    bool task_driven;
  };
}

#endif // SETTINGS_H
