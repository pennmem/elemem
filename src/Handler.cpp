#include "ConfigFile.h"
#include "Handler.h"
#include "MainWindow.h"
#include "Popup.h"
#include "RC/RC.h"

using namespace std;
using namespace RC;


namespace CML {
  Handler::Handler() : edf_save(this) {
  }

  // Defaulting this here after ConfigFile is included ensures
  // proper deletion of the forward delcared APtr elements.
  Handler::~Handler() = default;

  void Handler::SetMainWindow(Ptr<MainWindow> new_main) {
    main_window = new_main;
  }

  void Handler::CerebusTest_Handler() {
    eeg_acq.CloseCerebus();

    APITests::CereLinkTest();
  }

  void Handler::CereStimTest_Handler() {
    if (stim_test_warning) {
      if (!ConfirmWin("WARNING!  CereStim Test should not be run with a participant connected.  Continue?")) {
        return;
      }

      stim_test_warning = false;
    }

    stim_worker.CloseCereStim();

    APITests::CereStimTest();
  }

  void Handler::SetStimSettings_Handler(const StimSettings& updated_settings) {
    // TODO - implement
  }

  void Handler::OpenConfig_Handler(RC::FileRead& fr) {
    APtr<JSONFile> conf = new JSONFile();
    conf->Load(fr);
    exp_config = conf.ExtractConst();

    std::string elecfilename;
    exp_config->Get(elecfilename, "electrode_config_file");

    APtr<CSVFile> elecs = new CSVFile();
    elecs->Load(elecfilename);
    elec_config = elecs.ExtractConst();

    auto stim_channels = exp_config->Node("experiment", "stim_channels");
    stim_settings.Resize(stim_channels.size());
    min_stim_settings.Resize(stim_channels.size());
    max_stim_settings.Resize(stim_channels.size());
    for (size_t c=0; c<stim_channels.size(); c++) {
      std::string label;
      std::vector<u8> elecs;
      try {
        stim_channels[c].Get(label);
        auto split = RStr(label).Split('_');
        if (split.size() != 2 || split[0] == split[1]) {
          Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
              " string must be in LA1_LA2 bipolar format").c_str());
        }
        elecs.resize(2);
        size_t found = 0;
        for (size_t i=0; i<elec_config->data.size2(); i++) {
          if (elec_config->data[i][0] == split[0]) {
            elecs[0] = u8(elec_config->data[i][1].Get_u32());
            found++;
          }
          if (elec_config->data[i][0] == split[1]) {
            elecs[1] = u8(elec_config->data[i][1].Get_u32());
            found++;
          }
        }
        if (found != 2) {
          Throw_RC_Type(File, (RStr("Expected 2 channels in CSV for ")
                + label + ", but found " + RStr(found)).c_str());
        }
      }
      catch (ErrorMsg& e) {
        try {
          stim_channels[c].Get(elecs);
          if (elecs.size() != 2) {
            Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
                  " specifies wrong number of electrodes").c_str());
          }
        }
        catch (ErrorMsg&) {
          throw (e);  // From outer catch
        }
      }

      stim_settings[c].label = label;
      stim_settings[c].params.electrode_pos = elecs[0];
      stim_settings[c].params.electrode_neg = elecs[1];
      float f;
      stim_channels[c].Get(f, "amplitude_mA");
      stim_settings[c].params.amplitude = uint16_t(f*1000+0.5f);
      stim_channels[c].Get(stim_settings[c].params.frequency, "frequency_Hz");
      stim_channels[c].Get(stim_settings[c].params.duration, "duration_ms");
      stim_settings[c].params.duration *= 1000;
      stim_settings[c].approved = false;
      
      min_stim_settings = stim_settings;
      max_stim_settings = stim_settings;

      std::vector<float> vf;
      std::vector<uint32_t> vi;

      stim_channels[c].Get(vf, "amplitude_range_mA");
      if (vf.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " amplitude_range_mA needs 2 values").c_str());
      }
      min_stim_settings[c].params.amplitude = uint16_t(vf[0]*1000+0.5f);
      max_stim_settings[c].params.amplitude = uint16_t(vf[1]*1000+0.5f);

      stim_channels[c].Get(vi, "frequency_range_Hz");
      if (vi.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " frequency_range_Hz needs 2 values").c_str());
      }
      min_stim_settings[c].params.frequency = vi[0];
      max_stim_settings[c].params.frequency = vi[1];

      stim_channels[c].Get(vi, "duration_range_ms");
      if (vi.size() != 2) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " duration_range_ms needs 2 values").c_str());
      }
      min_stim_settings[c].params.duration = vi[0] * 1000;
      max_stim_settings[c].params.duration = vi[1] * 1000;

      if ( ! (
        (min_stim_settings[c].params.amplitude <=
         Betw(stim_settings[c].params.amplitude) <=
         max_stim_settings[c].params.amplitude) &&
        (min_stim_settings[c].params.frequency <=
         Betw(stim_settings[c].params.frequency) <=
         max_stim_settings[c].params.frequency) &&
        (min_stim_settings[c].params.duration <=
         Betw(stim_settings[c].params.duration) <=
         max_stim_settings[c].params.duration))) {
        Throw_RC_Type(File, ("Stim channel index "+RStr(c)+
            " default stim value outside of allowed range").c_str());
      }

      if (c < main_window->StimConfigCount()) {
        main_window->GetStimConfigBox(c).SetChannel(min_stim_settings[c].params,
            max_stim_settings[c].params, stim_settings[c].label);
        main_window->GetStimConfigBox(c).SetParameters(
            stim_settings[c].params);
      }
    }
  }
}

