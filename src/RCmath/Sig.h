#ifndef SIGLIB_H
#define SIGLIB_H

#include "MathBits.h"
#include "Stats.h"
#include "../RC/Data1D.h"

namespace RCmath {

class Sig {
  public:

  template<class T>
  static void FFT (const RC::Data1D<T> &in_real,
                   const RC::Data1D<T> &in_imag,
                   RC::Data1D<T> &out_real,
                   RC::Data1D<T> &out_imag) {
    T ang_fact;

    size_t datasize = Min(in_real.size(), in_imag.size());
    out_real.Resize(datasize);
    out_imag.Resize(datasize);

    size_t logsize = ILog2(datasize);
    datasize = ((size_t)1) << logsize;

    // Reposition by bitwise reversal of index.
    size_t halfsize = datasize >> 1;
    size_t swapi = 0;

    for (size_t i=0; i<datasize-1; i++) {
      out_real[i] = in_real[swapi];
      out_imag[i] = in_imag[swapi];

      size_t mask = halfsize;
      while (swapi >= mask) {
        swapi -= mask;
        mask >>= 1;
      }
      swapi += mask;
    }
    out_real[datasize-1] = in_real[datasize-1];
    out_imag[datasize-1] = in_imag[datasize-1];


    // Butterfly calculation.
    size_t step = 1;
    for (size_t level=0; level<logsize; level++) {
      size_t increm = step << 1;
      ang_fact = TWO_PI / increm;

      for (size_t ang=0; ang<step; ang++) {
        T phi = ang_fact * ang;
        T wa_real = cos(phi);
        T wa_imag = -sin(phi);

        for (size_t i=ang; i<datasize; i+=increm) {
          T rrprod = wa_real * out_real[i+step];
          T irprod = wa_imag * out_real[i+step];
          T riprod = wa_real * out_imag[i+step];
          T iiprod = wa_imag * out_imag[i+step];
          T rriidiff = rrprod - iiprod;
          T riirsum = riprod + irprod;
          out_real[i+step] = out_real[i] - rriidiff;
          out_imag[i+step] = out_imag[i] - riirsum;
          out_real[i] += rriidiff;
          out_imag[i] += riirsum;
        }
      }

      step = increm;
    }
  }


  // Note, in_imag is altered.
  template<class T>
  static void InvFFT (const RC::Data1D<T> &in_real,
                      RC::Data1D<T> &in_imag,
                      RC::Data1D<T> &out_real,
                      RC::Data1D<T> &out_imag) {
    size_t datasize = Min(in_real.size(), in_imag.size());

    for (size_t i=0; i<datasize; i++) {
      in_imag[i] = -in_imag[i];
    }

    FFT(in_real, in_imag, out_real, out_imag);

    T inv_size = ((T)1.0) / datasize;
    T neg_inv_size = -inv_size;

    for (size_t i=0; i<out_real.size(); i++) {
      out_real[i] = inv_size*out_real[i];
      out_imag[i] = neg_inv_size*out_imag[i];
    }
  }


  template<class T>
  static void FFTReal (const RC::Data1D<T> &in_real,
                       RC::Data1D<T> &out_abs, RC::Data1D<T> &out_phase) {
    out_abs.Resize(in_real.size());
    out_phase.Resize(in_real.size());
    out_phase.Zero();

    // Reuse buffers, actually real/imag.  Okay since phase = 0.
    FFT(in_real, out_phase, out_abs, out_phase);

    for (size_t i=0; i<out_abs.size(); i++) {
      T g = out_abs[i];
      T s = out_phase[i];
      out_abs[i] = Sqrt(g*g + s*s);
      out_phase[i] = Atan2(s, g);
    }
  }


  template<class T> class TempBuffer {
    public:
    RC::Data1D<T> g1;
    RC::Data1D<T> s1;
    RC::Data1D<T> g2;
    RC::Data1D<T> s2;
    void Resize(size_t size) {
      g1.Resize(size);
      s1.Resize(size);
      g2.Resize(size);
      s2.Resize(size);
    }
  };


  // The temporary buff parameter is optional, and provided for allocation
  // efficiency purposes if this function will be repeatedly called.
  template<class T>
  static void FFT (const RC::Data2D<T> &in_real,
                   const RC::Data2D<T> &in_imag,
                   RC::Data2D<T> &out_real,
                   RC::Data2D<T> &out_imag,
                   RC::Ptr< Sig::TempBuffer<T> > buff=NULL) {
    size_t datasize_y = Min(in_real.size2(), in_imag.size2());
    size_t datasize_x = Min(in_real.size1(), in_imag.size1());

    size_t logsize_y = ILog2(datasize_y);
    datasize_y = ((size_t)1) << logsize_y;

    size_t logsize_x = ILog2(datasize_x);
    datasize_x = ((size_t)1) << logsize_x;

    if (buff.IsNull()) {
      buff = new Sig::TempBuffer<T>();
    }
    buff->Resize(datasize_y);

    out_real.Resize(datasize_x, datasize_y);

    for (size_t y=0; y<datasize_y; y++) {
      FFT(in_real[y], in_imag[y], out_real[y], out_imag[y]);
    }

    for (size_t x=0; x<datasize_x; x++) {
      for (size_t y=0; y<datasize_y; y++) {
        buff->g1[y] = out_real[y][x];
        buff->s1[y] = out_imag[y][x];
      }
      FFT(buff->g1, buff->s1, buff->g2, buff->s2);
      for (size_t y=0; y<datasize_y; y++) {
        out_real[y][x] = buff->g2[y];
        out_imag[y][x] = buff->s2[y];
      }
    }
  }


  // Note:  in_imag is altered.
  // The temporary buff parameter is optional, and provided for allocation
  // efficiency purposes if this function will be repeatedly called.
  template<class T>
  static void InvFFT (const RC::Data2D<T> &in_real,
                      RC::Data2D<T> &in_imag,
                      RC::Data2D<T> &out_real,
                      RC::Data2D<T> &out_imag,
                      RC::Ptr< Sig::TempBuffer<T> > buff=NULL) {
    size_t datasize_y = Min(in_real.size2(), in_imag.size2());
    size_t datasize_x = Min(in_real.size1(), in_imag.size1());

    for (size_t y=0; y<datasize_y; y++) {
      for (size_t x=0; x<datasize_x; x++) {
        in_imag[y][x] = -in_imag[y][x];
      }
    }

    FFT(in_real, in_imag, out_real, out_imag);

    T inv_size = ((T)1.0) / (out_real.size1()*out_real.size2());
    T neg_inv_size = -inv_size;

    for (size_t y=0; y<out_real.size2(); y++) {
      for (size_t x=0; x<out_real.size1(); x++) {
        out_real[y][x] = inv_size * out_real[y][x];
        out_imag[y][x] = neg_inv_size * out_imag[y][x];
      }
    }
  }


  // The temporary buff parameter is optional, and provided for allocation
  // efficiency purposes if this function will be repeatedly called.
  template<class T>
  static void Convolve(const RC::Data1D<T> &in1, const RC::Data1D<T> &in2,
                       RC::Data1D<T> &result,
                       RC::Ptr< Sig::TempBuffer<T> > buff=NULL) {
    size_t datasize = Min(in1.size(), in2.size());

    if (buff.IsNull()) {
      buff = new Sig::TempBuffer<T>();
    }
    buff->Resize(datasize);
    result.Resize(datasize);

    buff->s1.Zero();
    buff->s2.Zero();

    // Buffer reuse okay since buff->s1 and s2 are zero.
    FFT(in1, buff->s1, buff->g1, buff->s1);
    FFT(in2, buff->s2, buff->g2, buff->s2);

    for (size_t i=0; i<buff->g1.size(); i++) {
      T temp_g1 = buff->g1[i];
      T temp_s1 = buff->s1[i];
      // (g1+i*s1) * (g2+i*s2) = g1+g2-s1*s2 + i*(g1*s2 + g2*s1)
      buff->g1[i] = temp_g1 * buff->g2[i] - temp_s1 * buff->s2[i];
      buff->s1[i] = temp_g1 * buff->s2[i] + temp_s1 * buff->g2[i];
    }

    InvFFT(buff->g1, buff->s1, result, buff->s2);

    for (size_t i=0; i<result.size(); i++) {
      result[i] = Sqrt(result[i]*result[i] + buff->s2[i]*buff->s2[i]);
    }
  }


  // m = Cov(x,y) / Var(x);  b = Mean(y) - m * Mean(x);
  template<class T>
  static inline void LinearRegression(const RC::Data1D<T> &x,
                                      const RC::Data1D<T> &y,
                                      T &m, T &b) {
    size_t i;
    T mean_x, mean_y;
    size_t size;
    Stats::CommonMean(x, y, size, mean_x, mean_y);

    T sum_var_x = 0;
    T sum_covar = 0;
    for (i=0; i<size; i++) {
      T diff_x = x[i] - mean_x;
      T diff_y = y[i] - mean_y;
      sum_var_x += diff_x * diff_x;
      sum_covar += diff_x * diff_y;
    }

    m = sum_covar / sum_var_x;
    b = mean_y - m * mean_x;
  }


  template<class T>
  static inline void LinearRegression(const RC::Data2D<T> &x,
                                      const RC::Data2D<T> &y,
                                      T &m, T &b) {
    size_t i1, i2;
    T mean_x, mean_y;
    size_t size1, size2;
    Stats::CommonMean(x, y, size1, size2, mean_x, mean_y);

    T sum_var_x = 0;
    T sum_covar = 0;
    for (i2=0; i2<size2; i2++) {
      for (i1=0; i1<size1; i1++) {
        T diff_x = x[i2][i1] - mean_x;
        T diff_y = y[i2][i1] - mean_y;
        sum_var_x += diff_x * diff_x;
        sum_covar += diff_x * diff_y;
      }
    }

    m = sum_covar / sum_var_x;
    b = mean_y - m * mean_x;
  }

  template<class T>
  static inline void LinearRegression(const RC::Data3D<T> &x,
                                      const RC::Data3D<T> &y,
                                      T &m, T &b) {
    size_t i1, i2, i3;
    T mean_x, mean_y;
    size_t size1, size2, size3;
    Stats::CommonMean(x, y, size1, size2, size3, mean_x, mean_y);

    T sum_var_x = 0;
    T sum_covar = 0;
    for (i3=0; i3<size3; i3++) {
      for (i2=0; i2<size2; i2++) {
        for (i1=0; i1<size1; i1++) {
          T diff_x = x[i2][i1] - mean_x;
          T diff_y = y[i2][i1] - mean_y;
          sum_var_x += diff_x * diff_x;
          sum_covar += diff_x * diff_y;
        }
      }
    }

    m = sum_covar / sum_var_x;
    b = mean_y - m * mean_x;
  }


  template<class T>
  static inline T PearsonCoefficient(const RC::Data1D<T> &x,
                                     const RC::Data1D<T> &y) {
    return Stats::Cov(x, y) / (Stats::SD(x) * Stats::SD(y));
  }
  template<class T>
  static inline T PearsonCoefficient(const RC::Data2D<T> &x,
                                     const RC::Data2D<T> &y) {
    return Stats::Cov(x, y) / (Stats::SD(x) * Stats::SD(y));
  }
  template<class T>
  static inline T PearsonCoefficient(const RC::Data3D<T> &x,
                                     const RC::Data3D<T> &y) {
    return Stats::Cov(x, y) / (Stats::SD(x) * Stats::SD(y));
  }
};

}
#endif // SIGLIB_H

