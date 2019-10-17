// 2019, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// This file provides C++ RAII wrappers for the Cerebus cbsdk API by Blackrock
// Microsystems
//
/////////////////////////////////////////////////////////////////////////////

#include "Cerebus.h"

namespace CML {
  std::string CSException::CodeToString(cbSdkResult err) {
    int err_i = int(err)
    switch(err_i) {
      case 3:
        return "If file conversion is needed";
      case 2:
        return "Library is already closed";
      case 1:
        return "Library is already opened";
      case 0:
        return "Successful operation";
      case -1:
        return "Not implemented";
      case -2:
        return "Unknown error";
      case -3:
        return "Invalid parameter";
      case -4:
        return "Interface is closed cannot do this operation";
      case -5:
        return "Interface is open cannot do this operation";
      case -6:
        return "Null pointer";
      case -7:
        return "Unable to open Central interface";
      case -8:
        return "Unable to open UDP interface (might happen if default)";
      case -9:
        return "Unable to open UDP port";
      case -10:
        return "Unable to allocate RAM for trial cache data";
      case -11:
        return "Unable to open UDP timer thread";
      case -12:
        return "Unable to open Central communication thread";
      case -13:
        return "Invalid channel number";
      case -14:
        return "Comment too long or invalid";
      case -15:
        return "Filename too long or invalid";
      case -16:
        return "Invalid callback type";
      case -17:
        return "Callback register/unregister failed";
      case -18:
        return "Trying to run an unconfigured method";
      case -19:
        return "Invalid trackable id, or trackable not present";
      case -20:
        return "Invalid video source id, or video source not present";
      case -21:
        return "Cannot open file";
      case -22:
        return "Wrong file format";
      case -23:
        return "Socket option error (possibly permission issue)";
      case -24:
        return "Socket memory assignment error";
      case -25:
        return "Invalid range or instrument address";
      case -26:
        return "library memory allocation error";
      case -27:
        return "Library initialization error";
      case -28:
        return "Conection timeout error";
      case -29:
        return "Resource is busy";
      case -30:
        return "Instrument is offline";
      case -31:
        return "The instrument runs an outdated protocol version";
      case -32:
        return "The library is outdated";
      default:
        return "Unrecognized error code";

    }
  }

}

