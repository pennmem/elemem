#ifndef RCQAPPLICATION_H
#define RCQAPPLICATION_H

#include <QApplication>

// Processes exceptions in events.
class RCQApplication : public QApplication {
  public:

  RCQApplication(int& argc, char** argv) : QApplication(argc, argv) { }

  bool notify(QObject *receiver, QEvent *e); 
};

#endif // RCQAPPLICATION_H

