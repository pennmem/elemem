// 2022, Ryan A. Colyer
// Computational Memory Lab, University of Pennsylvania
//
// Provides thread synchronization for calls to the edflib library, which
// is not thread-safe for opening or closing files.
//
/////////////////////////////////////////////////////////////////////////////

#include "EDFSynch.h"
#include <type_traits>

namespace CML {
  EDFSynch& EDFSynch::Inst() {
    static EDFSynch inst;
    return inst;
  }


  int EDFSynch::OpenRead(const char* filename, edf_hdr_struct* edf_hdr,
      int annotations) {
    return Inst().OpenRead_Task(filename, edf_hdr, annotations);
  }

  int EDFSynch::OpenWrite(const char* filename, int filetype, int num_sigs) {
    return Inst().OpenWrite_Task(filename, filetype, num_sigs);
  }

  void EDFSynch::Close(int edf_hdl) {
    Inst().Close_Task(edf_hdl);
  }


  int EDFSynch::OpenRead_Handler(const char*& filename,
      edf_hdr_struct*& edf_hdr, const int& annotations) {
    return edfopen_file_readonly(filename, edf_hdr, annotations);
  }


  int EDFSynch::OpenWrite_Handler(const char*& filename,
      const int& filetype, const int& num_sigs) {
    return edfopen_file_writeonly(filename, filetype, num_sigs);
  }

  void EDFSynch::Close_Handler(const int& edf_hdl) {
    edfclose_file(edf_hdl);
  }

}

