#include "CereStimDLL.h"
#include <iostream>
using namespace std;

CS_EXPORT int CS_Connect() {
  cout << "CS_Connect()\n";
  return 0;
}

CS_EXPORT int CS_Disconnect() {
  cout << "CS_Disconnect()\n";
  return 0;
}

CS_EXPORT int CS_ScanForDevices(uint64_t* max_devices, uint32_t* array_of_serial_nums) {
  if (max_devices == NULL || array_of_serial_nums == NULL || *max_devices == 0) {
    cout << "CS_ScanForDevices(INVALID)\n";
    return (int)CS_Result::BINVALIDPARAMS;
  }
  cout << "CS_ScanForDevices(&" << *max_devices << ", ...)\n";
  return 0;
}

CS_EXPORT int CS_SetDevice(uint32_t dev) {
  cout << "CS_SetDevice(" << dev << ")\n";
  return 0;
}

// voltage allowed values 7 through 15, meaning (0.5 + 0.6*voltage)V
//   4.7V, 5.3V, 5.9V, 6.5V, 7.1V, 7.7V, 8.3V, 8.9V, 9.5V
CS_EXPORT int CS_GetMaxValues(uint8_t* voltage, uint16_t* amplitude,
    uint32_t* phase_charge, uint32_t* frequency) {
  if (voltage == nullptr || amplitude == nullptr || phase_charge == nullptr ||
      frequency == nullptr) {
    cout << "CS_GetMaxValues(NULL)\n";
    return (int)CS_Result::BNULLPTR;
  }
  cout << "CS_GetMaxValues(...)\n";
  return 0;
}

// voltage allowed values 7 through 15, meaning (0.5 + 0.6*voltage)V
//   4.7V, 5.3V, 5.9V, 6.5V, 7.1V, 7.7V, 8.3V, 8.9V, 9.5V
CS_EXPORT int CS_SetMaxValues(uint8_t voltage, uint16_t amplitude, uint32_t
    phase_charge, uint32_t frequency) {
  cout << "CS_SetMaxValues(" << voltage << ", " << amplitude << ", " << phase_charge << ", " << frequency << ")\n";
  return 0;
}

// waveform 1 through 15.
// cathodic_first=1 is cathodic/negative first, 0 is anodic/positive first.
CS_EXPORT int CS_ConfigureStimulusPattern(uint16_t waveform, uint8_t
    cathodic_first, uint8_t pulses, uint16_t amp1, uint16_t amp2, uint16_t
    width1, uint16_t width2, uint32_t frequency, uint16_t interphase) {
  cout << "CS_ConfigureStimulusPattern(" << waveform << ", " << cathodic_first << ", " << pulses << ", amp=[" << amp1 << ", " << amp2 << "], width=[" << width1 << ", " << width2 << "], " << frequency << ", " << interphase << ")\n";
  if (waveform == 0 || waveform > 15 ||
      cathodic_first > 1 ||
      amp1 == 0 || amp1 > 10000 ||
      amp2 == 0 || amp2 > 10000 ||
      frequency < 4 || frequency > 5000 ||
      interphase < 53) {
      cout << "CS_ConfigureStimulusPattern was invalid.\n";
    return (int)CS_Result::BINVALIDPARAMS;
  }
  return 0;
}

// Up to 128 AutoStimulus and Wait commands permitted with a sequence.
CS_EXPORT int CS_BeginningOfSequence() {
  cout << "CS_BeginningOfSequence()\n";
  return 0;
}

// Required for simultaneous stimulation
CS_EXPORT int CS_BeginningOfGroup() {
  cout << "CS_BeginningOfGroup()\n";
  return 0;
}

// waveform 1 through 15.
CS_EXPORT int CS_AutoStimulus(uint8_t electrode, uint16_t waveform) {
  cout << "CS_AutoStimulus(" << electrode << ", " << waveform << ")\n";
  if (waveform == 0 || waveform > 15) {
    cout << "CS_AutoStimulus invalid.\n";
    return (int)CS_Result::BINVALIDPARAMS;
  }
  return 0;
}

CS_EXPORT int CS_Wait(uint16_t milliseconds) {
  cout << "CS_Wait(" << milliseconds << ")\n";
  return 0;
}

CS_EXPORT int CS_EndOfGroup() {
  cout << "CS_EndOfGroup()\n";
  return 0;
}

CS_EXPORT int CS_EndOfSequence() {
  cout << "CS_EndOfSequence()\n";
  return 0;
}

CS_EXPORT int CS_Play(uint16_t times) {
  cout << "CS_Play(" << times << ")\n";
  return 0;
}

CS_EXPORT int CS_Pause() {
  cout << "CS_Pause()\n";
  return 0;
}

CS_EXPORT int CS_Stop() {
  cout << "CS_Stop()\n";
  return 0;
}
