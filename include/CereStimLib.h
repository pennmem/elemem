#ifndef CERESTIMLIB_H
#define CERESTIMLIB_H

#include <stdint.h>


// 1 to 1 mapping with Blackrock BResult values from BStimulator.h
enum class CS_Result {
  // Sofware Errors:
  BRETURN              =    1, // Early returned warning
  BSUCCESS             =    0, // Successful operation
  BNOTIMPLEMENTED      =   -1, // Not implemented
  BUNKNOWN             =   -2, // Unknown error
  BINVALIDHANDLE       =   -3, // Invalid handle
  BNULLPTR             =   -4, // Null pointer
  BINVALIDINTERFACE    =   -5, // Invalid intrface specified or interface not supported
  BINTERFACETIMEOUT    =   -6, // Timeout in creating the interface
  BDEVICEREGISTERED    =   -7, // Device with that address already connected.
  BINVALIDPARAMS       =   -8, // Invalid parameters
  BDISCONNECTED        =   -9, // Stim is disconnected, invalid operation
  BCONNECTED           =  -10, // Stim is connected, invalid operation
  BSTIMATTACHED        =  -11, // Stim is attached, invalid operation
  BSTIMDETACHED        =  -12, // Stim is detached, invalid operation
  BDEVICENOTIFY        =  -13, // Cannot register for device change notification
  BINVALIDCOMMAND      =  -14, // Invalid command
  BINTERFACEWRITE      =  -15, // Cannot open interface for write
  BINTERFACEREAD       =  -16, // Cannot open interface for read
  BWRITEERR            =  -17, // Cannot write command to the interface
  BREADERR             =  -18, // Cannot read command from the interface
  BINVALIDMODULENUM    =  -19, // Invalid module number specified
  BINVALIDCALLBACKTYPE =  -20, // Invalid callback type
  BCALLBACKREGFAILED   =  -21, // Callback register/unregister failed
  BLIBRARYFIRMWARE     =  -22, // CereStim Firmware version not supported by SDK Library Version
  BFREQPERIODZERO      =  -23, // Frequency or Period is zero and unable to be converted
  BNODEVICESELECTED    =  -24, // No physical device has been set. See setDevice() for help.

  // Hardware Errors:
  BNOK                 = -100, // Comamnd result not OK
  BSEQUENCEERROR       = -102, // Sequence Error
  BINVALIDTRIGGER      = -103, // Invalid Trigger
  BINVALIDCHANNEL      = -104, // Invalid Channel
  BINVALIDCONFIG       = -105, // Invalid Configuration
  BINVALIDNUMBER       = -106, // Invalid Number
  BINVALIDRWR          = -107, // Invalid Read/Write
  BINVALIDVOLTAGE      = -108, // Invalid Voltage
  BINVALIDAMPLITUDE    = -109, // Invalid Amplitude
  BINVALIDAFCF         = -110, // Invalid AF/CF
  BINVALIDPULSES       = -111, // Invalid Pulses
  BINVALIDWIDTH        = -112, // Invalid Width
  BINVALIDINTERPULSE   = -113, // Invalid Interpulse
  BINVALIDINTERPHASE   = -114, // Invalid Interphase
  BINVALIDFASTDISCH    = -115, // Invalid Fast Discharge
  BINVALIDMODULE       = -116, // Invalid Module
  BSTIMULIMODULES      = -117, // More Stimuli than Modules
  BMODULEUNAVAILABLE   = -118, // Module not Available
  BCHANNELUSEDINGROUP  = -119, // Channel already used in Group
  BCONFIGNOTACTIVE     = -120, // Configuration not Active
  BEMPTYCONFIG         = -121, // Empty Config
  BPHASENOTBALANCED    = -122, // Phases not Balanced
  BPHASEGREATMAX       = -123, // Phase Charge Greater than Max
  BAMPGREATMAX         = -124, // Amplitude Greater than Max
  BWIDTHGREATMAX       = -125, // Width Greater than Max
  BVOLTGREATMAX        = -126, // Voltage Greater than Max
  BMODULEDISABLED      = -127, // Module already disabled can't disable it
  BMODULEENABLED       = -128, // Module already enabled can't reenable it
  BINVALIDFREQUENCY    = -129, // Invalid Frequency
  BFREQUENCYGREATMAX   = -130, // The frequency is greater than the max value allowed
  BDEVICELOCKED        = -131, // Device locked due to hardware mismatch or not being configured
  BECHOERROR           = -132  // Command returned was not the same command sent
};


#ifdef DLL_BUILD
#define CS_EXPORT extern "C" __declspec(dllexport)
#else
#define CS_EXPORT extern "C"
#endif

// NOTE:  All int return values should be cast to CS_Result enum.

CS_EXPORT int CS_Connect();
CS_EXPORT int CS_Disconnect();

CS_EXPORT int CS_ScanForDevices(uint64_t* max_devices, uint32_t* array_of_serial_nums);
CS_EXPORT int CS_SetDevice(uint32_t dev);

// voltage allowed values 7 through 15, meaning (0.5 + 0.6*voltage)V
//   4.7V, 5.3V, 5.9V, 6.5V, 7.1V, 7.7V, 8.3V, 8.9V, 9.5V
CS_EXPORT int CS_GetMaxValues(uint8_t* voltage, uint16_t* amplitude,
    uint32_t* phase_charge, uint32_t* frequency);

// voltage allowed values 7 through 15, meaning (0.5 + 0.6*voltage)V
//   4.7V, 5.3V, 5.9V, 6.5V, 7.1V, 7.7V, 8.3V, 8.9V, 9.5V
CS_EXPORT int CS_SetMaxValues(uint8_t voltage, uint16_t amplitude, uint32_t
    phase_charge, uint32_t frequency);

// waveform 1 through 15.
// cathodic_first=1 is cathodic/negative first, 0 is anodic/positive first.
CS_EXPORT int CS_ConfigureStimulusPattern(uint16_t waveform, uint8_t
    cathodic_first, uint8_t pulses, uint16_t amp1, uint16_t amp2, uint16_t
    width1, uint16_t width2, uint32_t frequency, uint16_t interphase);

// Up to 128 AutoStimulus and Wait commands permitted with a sequence.
CS_EXPORT int CS_BeginningOfSequence();
// Required for simultaneous stimulation
CS_EXPORT int CS_BeginningOfGroup();
// waveform 1 through 15.
CS_EXPORT int CS_AutoStimulus(uint8_t electrode, uint16_t waveform);
CS_EXPORT int CS_Wait(uint16_t milliseconds);
CS_EXPORT int CS_EndOfGroup();
CS_EXPORT int CS_EndOfSequence();

CS_EXPORT int CS_Play(uint16_t times);
CS_EXPORT int CS_Pause();
CS_EXPORT int CS_Stop();

#endif // CERESTIMLIB_H

