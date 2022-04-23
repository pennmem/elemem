#ifndef STIMLOOP_H
#define STIMLOOP_H

#include "NetClient.h"
#include "CereStim.h"
#include "Config.h"
#include <string>
#include <vector>

namespace SP {
  class StimLoop {
    public:
    StimLoop(Config conf, Sock soc, std::string version)
      : conf(conf), soc(soc), version(version) { }
    void Run();

    private:
    bool StimInitialize();
    void StimStart();
    void StimConfig(const std::vector<std::string>& cmd, bool thetaburst);

    ChannelLimits FindLimit(uint8_t pos, uint8_t neg);

    CML::CereStim cerestim;
    Config conf;
    Sock soc;
    std::string version;
    bool stim_configured = false;
  };
}

#endif

