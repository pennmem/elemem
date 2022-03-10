#ifndef PYTHONINTERPRETER_H
#define PYTHONINTERPRETER_H

// NOTE: This include needs to be put before any other qt include or includes that then include qt

#include <pybind11/embed.h>
// https://stackoverflow.com/questions/15078060/embedding-python-in-qt-5
#undef B0 // This causes a conflict with Qt5
#include "RCqt/Worker.h"

namespace py = pybind11;

namespace CML {
  // There can only one python interpreter with pybind11, so a singleton is needed
  // Singleton design pattern: https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
  class PythonInterface : public RCqt::WorkerThread {

    public:
      // Singleton getter
      static PythonInterface& GetInstance()
      {
          static PythonInterface instance; // Lazy Initialization
          return instance;
      }

      // Singleton can't copy
      PythonInterface(PythonInterface const&) = delete;
      void operator=(PythonInterface const&) = delete;

      // Python functions
      RCqt::TaskGetter<double, const double&> Sqrt =
        TaskHandler(PythonInterface::Sqrt_Handler);


    private:
      // Imported modules (make sure to release these in destructor)
      // The purpose of this is to reduce the overhead of each import
      py::object py_sqrt;

      // Handlers
      double Sqrt_Handler(const double& x) { return py_sqrt(x).cast<double>(); }

      PythonInterface() {
        py::initialize_interpreter();
        py_sqrt = py::module::import("math").attr("sqrt");
      }

      // Make sure to call release() on any pybind11 objects BEFORE calling finalize_interpreter()
      ~PythonInterface() {
        py_sqrt.release();
        py::finalize_interpreter();
      }
  };
}

#endif // PYTHONINTERPRETER_H
