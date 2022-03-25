// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// Blackrock Cerebus acquisition test program.
//
/////////////////////////////////////////////////////////////////////////////

// g++ -std=c++17 -rdynamic -O3 -funroll-loops -fdiagnostics-color=always -Wall -Wextra -Wno-unused -Wnull-dereference -Werror -o continuous continuous.cpp -I ../include/ -Wno-unused-parameter -L ../lib64/ -lcbsdk -Wl,-rpath,$ORIGIN -ldl -lrt -lpthread -lstdc++fs

#include "APITests.h"
#ifdef CEREBUS_HW
#include "Cerebus.h"
#endif
#include "CereStim.h"

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <unistd.h>
#include <QtCore>

#include "RC/APtr.h"


using namespace std;
using namespace CML;


namespace APITests {
  #ifndef WIN32
  #ifndef Sleep
  void Sleep(uint64_t x) {
    usleep(x * 1000);
  }
  #endif
  #endif

#ifdef CEREBUS_HW
  void PrintTrial(const cbSdkTrialCont& trial) {
    cout << "Channel count:  " << trial.count << endl;

    //for (size_t i=0; i<cbNUM_ANALOG_CHANS; i++) {
    for (size_t i=0; i<1; i++) {
      cout << "  channel:  " << trial.chan[i] << endl;
      cout << "  sample rate:  " << trial.sample_rates[i] << endl;
      cout << "  number of samples:  " << trial.num_samples[i] << endl;
    }

    cout << "Start time:  " << trial.time << endl;
  }
#endif

  void CereStimTest() {
    cout << "Stimulation Test:" << endl;

    RC::APtr<StimInterface> cerestim = new CereStim();

    cout << "Connected." << endl;

    cout << "Configuring." << endl;

    StimProfile prof;
    StimChannel chan;

    chan.electrode_pos = 1;
    chan.electrode_neg = 2;
    chan.amplitude = 1000; // Unit 1uA, granularity 100uA for macro.
    chan.frequency = 100; // Unit Hz.
    chan.duration = 1000000;  // Unit us.
    chan.area = 6.31;  // Unit mm^2

    prof += chan;

    chan.electrode_pos = 3;
    chan.electrode_neg = 4;
    chan.amplitude = 2000; // Unit 1uA, granularity 100uA for macro.
    chan.frequency = 50; // Unit Hz.
    chan.duration = 1000000;  // Unit us.
    chan.area = 6.31;  // Unit mm^2

    prof += chan;

    cerestim->ConfigureStimulation(prof);

    cout << "Stimulating." << endl;
    cerestim->Stimulate();

    sleep(2);

    cout << "Done." << endl;
  }

  void CereLinkTest() {
    #ifndef CEREBUS_HW
    throw std::runtime_error("Cerebus hardware support not built.");
    #else
    cout << "Continuous Acquisition Test:" << endl;

    Cerebus cereb;
    cereb.SetChannel(0);

    for (size_t i=0; i<100; i++) {
      Sleep(100);
      auto& data = cereb.GetData();
  //    for (size_t c=0; c<data.size(); c++) {
      for(size_t c=0; c<8; c++) {
        auto& chan = data[c].data;
        cout << "Channel " << (c+1) << endl;
        for (size_t d=0; d<chan.size(); d++) {
          cout << "  " << chan[d] << endl;
        }
      }
    }

    cout << endl;
    #endif
  }
}
