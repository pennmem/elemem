#include "Popup.h"
#include "RC/RC.h"
#include <QMessageBox>

#define MYTIMESTAMP __TIMESTAMP__

using namespace RC;

namespace CML {
  const RC::RStr LICENSE_MIT {
    "<p>MIT License</p>"
    "<p>Copyright (c) 2013-2019 Niels Lohmann</p>"
    "<p>Permission is hereby granted, free of charge, to any person obtaining a copy"
    "of this software and associated documentation files (the \"Software\"), to deal"
    "in the Software without restriction, including without limitation the rights"
    "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell"
    "copies of the Software, and to permit persons to whom the Software is"
    "furnished to do so, subject to the following conditions:</p>"
    "<p>The above copyright notice and this permission notice shall be included in all"
    "copies or substantial portions of the Software.</p>"
    "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE"
    "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
    "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE"
    "SOFTWARE.</p>"
  };

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
    text += "<i>";
    text += "<hr/>";
    text += "<p>JSON for Modern C++ library component:</p>";
    text += LICENSE_MIT;
    text += "</i>";

    QMessageBox::about(0, title.c_str(), text.c_str());
  }
}


