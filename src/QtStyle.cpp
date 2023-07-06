#include "QtStyle.h"

namespace Resource {
  const RC::RStr QtStyle_css {
    "QGroupBox {\n"
    "    border: 1px solid black;\n"
    "    border-radius: 4px;\n"
    "    margin: 0.5em 0.15em 0 0;\n"
    "}\n"
    "\n"
    "QGroupBox::title {\n"
    "    subcontrol-origin: margin;\n"
    "    left: 10px;\n"
    "    padding: 0 0 2 0;\n"
    "}\n"
    "QWidget {\n"
    "  font-size: 12pt;\n"
    "}\n"
  };
}
