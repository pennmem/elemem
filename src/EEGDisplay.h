#ifndef EEGDISPLAY_H
#define EEGDISPLAY_H

#include "CImage.h"
#include "RC/Data1D.h"
#include <vector>


namespace CML {
  typedef RC::Data1D<RC::Data1D<float>> EEGData_t;
  typedef std::vector<std::vector<float>> EEGDataVect_t;

  class EEGLabels {
    public:
    uint32_t channel;
    RC::RStr label;
  };

  class EEGDisplay : public CImage {
    public:

    EEGDisplay(int width, int height);
    virtual ~EEGDisplay();

    RCqt::TaskCaller<EEGData_t> UpdateData =
      TaskHandler(EEGDisplay::UpdateData_Handler);

    RCqt::TaskCaller<EEGDataVect_t> UpdateDataVect =
      TaskHandler(EEGDisplay::UpdateDataVect_Handler);

    protected:

    template <class C>
    void UpdateData_Template(C& new_data) {
      // TODO
      /*data = new_data;*/
    }
    void UpdateData_Handler(EEGData_t& new_data) {
      UpdateData_Template(new_data);
    }
    void UpdateDataVect_Handler(EEGDataVect_t& new_data) {
      UpdateData_Template(new_data);
    }

    virtual void DrawBackground();
    virtual void DrawOnTop();

    EEGData_t data;
    size_t data_offset = 0;
    
  };
}

#endif // EEGDISPLAY_H

