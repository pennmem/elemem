#ifndef QTFILEFUNCTIONS_H
#define QTFILEFUNCTIONS_H

#include "RC/RC.h"
#include <QIODevice>

namespace CML {

class MyQIODevice : public QIODevice {
  Q_OBJECT

  public:

  MyQIODevice(RC::FileRW frw) : frw(frw) { }

  qint64 readData(char* data, qint64 maxSize) {
    RC::Data1D<char> rcdata(maxSize, data);
    frw.Read(rcdata);
    return maxSize;
  }

  qint64 writeData(const char* data, qint64 maxSize) {
    RC::Data1D<const char> rcdata(maxSize, data);
    frw.Write(rcdata);
    return maxSize;
  }

  protected:
  RC::FileRW frw;
};

}

#endif // QTFILEFUNCTIONS_H

