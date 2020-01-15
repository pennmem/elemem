// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the Cerebus cbsdk API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CEREBUS_H
#define CEREBUS_H

#include "cbsdk.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef WIN32
#include <unistd.h>
#ifndef Sleep
#define Sleep(x) usleep((x) * 1000)
#endif
#endif

namespace CML {
  class CBException : public std::runtime_error {
    public:
    CBException(cbSdkResult error_code_, std::string attempted="",
        uint32_t instance=uint32_t(-1))
      : std::runtime_error(std::string("Cerebus ") + attempted + " Error " +
          std::to_string(int(error_code_)) + ", " +
          ((instance!=uint32_t(-1)) ? std::string("instance ") +
            std::to_string(instance) + ", " : std::string("")) +
          CodeToString(error_code_)),
        error_code(error_code_),
        error_message(CodeToString(error_code_)) {
    }
    virtual ~CBException(); // to specify translation unit for v-table.

    cbSdkResult GetErrorCode() { return error_code; }
    std::string GetErrorMsg() { return error_message; }

    static std::string CodeToString(cbSdkResult err);

    private:
    cbSdkResult error_code;
    std::string error_message;
  };

  struct TrialData {
    uint16_t chan;
    std::vector<int16_t> data =
        std::vector<int16_t>(cbSdk_CONTINUOUS_DATA_SAMPLES);
  };

  // All channel numbers are zero-based.  For user-interfacing use one-based.
  class Cerebus {
    public:
    Cerebus(uint32_t instance_=0);
    ~Cerebus();

    // Rule of 5.
    Cerebus(const Cerebus& other) = delete;
    Cerebus& operator=(const Cerebus& other) = delete;
    Cerebus(Cerebus&& other) = delete;
    Cerebus& operator=(Cerebus&& other);


    void Open();  // Automatic at first use.
    void Close();

    void SetInstance(uint32_t instance);

    void SetChannel(uint16_t channel);
    void SetChannelRange(uint16_t first_channel, uint16_t last_channel);
    void SetChannels(std::vector<uint16_t> channel_list);

    const std::vector<TrialData>& GetData();


    protected:

    void ClearChannels();
    void ConfigureChannel(uint16_t channel);
    void SetTrialConfig();

    void BeOpen();

    uint32_t instance;
    uint16_t first_chan=uint16_t(-1);  // unset
    uint16_t last_chan=0;

    std::vector<TrialData> channel_data =
        std::vector<TrialData>(cbNUM_ANALOG_CHANS);

    cbSdkTrialCont trial{};

    bool is_open = false;
  };
}

#endif

