#ifndef UTILS_H
#define UTILS_H

#include "RC/RStr.h"
#include "RC/Data1D.h"

namespace CML {
  RC::RStr GetHomeDir();
  RC::RStr GetDesktop();
  int CeilDiv(int dividend, int divisor);
  size_t CeilDiv(size_t dividend, size_t divisor);
  bool CheckStorage(RC::RStr path, size_t amount);
}

#endif // UTILS_H

