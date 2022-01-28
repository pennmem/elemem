#ifndef EDFSYNCH_H
#define EDFSYNCH_H

#include "RC/RStr.h"
#include "RC/File.h"
#include "RCqt/Worker.h"
#include "edflib/edflib.h"

namespace CML {
  class EDFSynch : public RCqt::WorkerThread {
    public:
    static int OpenRead(const char* filename, edf_hdr_struct* edf_hdr,
        int annotations);
    static int OpenWrite(const char* filename, int filetype, int num_sigs);
    static void Close(int edf_hdl);

    /// Initialize this after Qt initializes but before workers do work.
    static EDFSynch& Inst();

    protected:
    EDFSynch() { }

    RCqt::TaskGetter<int, const char*, edf_hdr_struct*, const int>
      OpenRead_Task =
      TaskHandler(EDFSynch::OpenRead_Handler);
    RCqt::TaskGetter<int, const char*, const int, const int> OpenWrite_Task =
      TaskHandler(EDFSynch::OpenWrite_Handler);
    RCqt::TaskBlocker<const int> Close_Task =
      TaskHandler(EDFSynch::Close_Handler);

    int OpenRead_Handler(const char*& filename, edf_hdr_struct*& edf_hdr,
        const int& annotations);
    int OpenWrite_Handler(const char*& filename,
        const int& filetype, const int& num_sigs);
    void Close_Handler(const int& edf_hdl);
  };
}

#endif // EDFSYNCH_H

