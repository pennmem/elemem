#include "EEGDisplay.h"
#include "RC/RC.h"
#include <algorithm>

using namespace RC;

namespace CML {
  EEGDisplay::EEGDisplay(int new_width, int new_height) {
    width = new_width;
    height = new_height;
  }


  EEGDisplay::~EEGDisplay() {
  }


  void EEGDisplay::DrawBackground() {
  }


  void EEGDisplay::DrawOnTop() {
    /*size_t x;
    float max, xscale, yscale;
    */

    if (data.IsEmpty()) {
      return;
    }

    /*

    max = -1e100;
    for (x = 0; x < data.size(); x++) {
      max = std::max(max, data[x]);
    }
    yscale = height / max;
    xscale = (float(width-1)) / (data.size()-1);


    SetPen(palette.NormToARGB(0.0, 0.4, 1.0, 1.0));
    QPointF last(0.0, height - (data[0]*yscale));
    for (x = 1; x < data.size(); x++) {
      QPointF current(x * xscale, height - (data[x] * yscale));
      painter.drawLine(last, current);
      last = current;
    }

    CornerText(RStr("max: ") + max);
    */
  }
}

