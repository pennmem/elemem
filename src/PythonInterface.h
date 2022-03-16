#ifndef PYTHONINTERPRETER_H
#define PYTHONINTERPRETER_H

// NOTE: This include needs to be put before any other qt include or includes that then include qt

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
// https://stackoverflow.com/questions/15078060/embedding-python-in-qt-5
#undef B0 // This causes a conflict with Qt5
#include "RCqt/Worker.h"
#include <vector>

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

      //RCqt::TaskGetter<std::vector<short>> Buttfilt =
      //  TaskHandler(PythonInterface::Buttfilt_Handler);
      RCqt::TaskBlocker<> Buttfilt =
        TaskHandler(PythonInterface::Buttfilt_Handler);


    private:
      // Imported modules (make sure to release these in destructor)
      // The purpose of this is to reduce the overhead of each import
      py::object py_sqrt;
      py::object py_buttfilt;

      // Handlers
      double Sqrt_Handler(const double& x) { return py_sqrt(x).cast<double>(); }
      //double Buttfilt_Handler(data_np_array, freqs_list, sampling_rate_size_t, filt_type_string, order_size_t, axis_int) {
      void Buttfilt_Handler() {
        //std::vector<short> temp_vector = make_vector<short>(size);
        //return as_pyarray(std::move(temp_vector));

        py::array_t<float_t> in_data = py_buttfilt().cast<py::array_t<float_t>>();

        auto out_data = toData2D(in_data);
        RC_ForRange(i, 0, out_data.size2()) {
          RC_DEBOUT(RC::RStr::Join(out_data[i], ", "));
        }
      }

      PythonInterface() {
        py::initialize_interpreter();
        py_sqrt = py::module::import("math").attr("sqrt");
        //py_buttfilt = py::module::import("ptsa.filt").attr("buttfilt");
        py_buttfilt = py::module::import("temp").attr("buttfilt");
      }

      // Make sure to call release() on any pybind11 objects BEFORE calling finalize_interpreter()
      ~PythonInterface() {
        py_sqrt.release();
        py::finalize_interpreter();
      }

      /**
       * YannickJadoul https://github.com/pybind/pybind11/issues/1042#issuecomment-642215028
       * \brief Returns py:array_T<Sequence> from sequence. Efficient as zero-copy.
       * \tparam Sequence type.
       * \param passthrough sequence.
       * \return Numpy array with a clean and safe reference to contents of Numpy array.
       */
      template <typename Sequence>
      inline py::array_t<typename Sequence::value_type> as_pyarray(Sequence &&seq) {
        auto size = seq.size();
        auto data = seq.data();
        std::unique_ptr<Sequence> seq_ptr = std::make_unique<Sequence>(std::move(seq));
        auto capsule = py::capsule(seq_ptr.get(), [](void *p) { std::unique_ptr<Sequence>(reinterpret_cast<Sequence*>(p)); });
        seq_ptr.release();
        return py::array(size, data, capsule);
      }

      /**
       * Sharpe https://github.com/pybind/pybind11/issues/1042#issuecomment-663154709
       * \brief Returns Data1D<T> from py:array_T<T>. Efficient as zero-copy.
       * \tparam T Type.
       * \param passthrough Numpy array.
       * \return Data1D<T> that with a clean and safe reference to contents of Numpy array.
       */
      template<class T=float_t>
      inline RC::Data1D<T> toData1D(const py::array_t<T>& passthrough)
      {
        py::buffer_info passthroughBuf = passthrough.request();
        if (passthroughBuf.ndim != 1) {
          Throw_RC_Error((RC::RStr("Number of dimensions must be 1, instead of ") + static_cast<int>(passthroughBuf.ndim)).c_str());
        }
        size_t length = passthroughBuf.shape[0];
        T* passthroughPtr = static_cast<T*>(passthroughBuf.ptr);
        RC::Data1D<T> passthroughData1D(length, passthroughPtr);
        return passthroughData1D;
      }

      /**
       * Sharpe https://github.com/pybind/pybind11/issues/1042#issuecomment-663154709
       * \brief Returns Data2D<T> from py:array_T<T>. Efficient as zero-copy.
       * \tparam T Type.
       * \param passthrough Numpy array.
       * \return Data2D<T> that with a clean and safe reference to contents of Numpy array.
       */
      template<class T=float_t>
      inline RC::Data2D<T> toData2D(const py::array_t<T>& passthrough)
      {
        py::buffer_info passthroughBuf = passthrough.request();
        if (passthroughBuf.ndim != 2) {
          Throw_RC_Error((RC::RStr("Number of dimensions must be 2, instead of ") + static_cast<int>(passthroughBuf.ndim)).c_str());
        }
        size_t length1 = passthroughBuf.shape[0];
        size_t length2 = passthroughBuf.shape[1];
        T* passthroughPtr = static_cast<T*>(passthroughBuf.ptr);
        RC::Data2D<T> passthroughData2D(length2, length1);
        RC_ForRange(i, 0, length1) {
          // TODO: JPB: (feature)(optimize) This likely does a copy due to Data1D operator=
          passthroughData2D[i] = std::move(RC::Data1D<T>(length2, passthroughPtr+(i*length2)));
        }
        return passthroughData2D;
      }
  };
}

#endif // PYTHONINTERPRETER_H
