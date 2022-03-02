#ifndef UTILS_H
#define UTILS_H

#include "RC/RStr.h"

namespace CML {
  RC::RStr GetHomeDir();
  RC::RStr GetDesktop();
  int CeilDiv(int dividend, int divisor);
  size_t CeilDiv(size_t dividend, size_t divisor);
}

#endif // UTILS_H

