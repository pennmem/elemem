#include "Popup.h"
#include "RC/RC.h"
#include <QMessageBox>

#define MYTIMESTAMP __TIMESTAMP__

using namespace RC;

namespace CML {
  // For the Lohmann JSON library:
  const RC::RStr LICENSE_JSON {
    "<p>JSON for Modern C++ library component:</p>"
    "<p>MIT License</p>"
    "<p>Copyright (c) 2013-2019 Niels Lohmann</p>"
    "<p>Permission is hereby granted, free of charge, to any person obtaining "
    "a copy of this software and associated documentation files "
    "(the \"Software\"), to deal in the Software without restriction, "
    "including without limitation the rights to use, copy, modify, merge, "
    "publish, distribute, sublicense, and/or sell copies of the Software, and "
    "to permit persons to whom the Software is furnished to do so, subject to "
    "the following conditions:</p>"
    "<p>The above copyright notice and this permission notice shall be "
    "included in all copies or substantial portions of the Software.</p>"
    "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
    "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
    "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. "
    "IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY "
    "CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, "
    "TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE "
    "SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
  };

	// For the EDFLib library
  const RC::RStr LICENSE_EDFLIB {
    "<p>EDFLib library component:</p>"
    "<p>BSD 3-Clause License</p>"
    "<p>Copyright (c) 2009 - 2019 Teunis van Beelen<br/>"
    "All rights reserved.</p>"
    "<p>Redistribution and use in source and binary forms, with or without"
    "modification, are permitted provided that the following conditions are"
    "met:<p>"
    "<ul><li>Redistributions of source code must retain the above copyright"
    "notice, this list of conditions and the following disclaimer.</li>"
    "<li>Redistributions in binary form must reproduce the above copyright"
    "notice, this list of conditions and the following disclaimer in the"
    "documentation and/or other materials provided with the"
    "distribution.</li>"
    "<li>Neither the name of the copyright holder nor the names of its"
    "contributors may be used to endorse or promote products derived from"
    "this software without specific prior written permission.</li></ul>"
    "<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS"
    "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT"
    "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A"
    "PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT"
    "HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,"
    "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT"
    "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,"
    "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY"
    "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT"
    "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE"
    "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH"
    "DAMAGE.</p>"
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
    text += LICENSE_JSON;
		text += LICENSE_EDFLIB;
    text += "</i>";

    QMessageBox::about(0, title.c_str(), text.c_str());
  }
}


