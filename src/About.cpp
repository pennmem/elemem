#include "Popup.h"
#include "RC/RC.h"
#include <QMessageBox>

#define MYTIMESTAMP __TIMESTAMP__

using namespace RC;

namespace CML {
  void AboutWin() {
    RStr year = RStr(MYTIMESTAMP).SplitWords().Last();

    RStr title = "Elemem";
    RStr copyright = RStr("&copy; 2019");
    if (year != "2019") {
      copyright += "-" + year;
    }
    copyright += RStr(", Computational Memory Lab, Universiy of Pennsylvania");
    RStr version = RStr("Build:  ") + MYTIMESTAMP;

    RStr text = RStr("<p style=\"font-size:x-large\">") + title + "</p>"
              + "<p>" + copyright + "<p/>"
              + "<p><i>" + version + "</i><p/>"
              + "<p><a href=\"https://memory.psych.upenn.edu\">https://memory.psych.upenn.edu</a></p>";

    QMessageBox::about(0, title.c_str(), text.c_str());
  }
}


