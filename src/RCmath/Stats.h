#ifndef RCMATH_STATS_H
#define RCMATH_STATS_H

#include "Calc.h"
#include "MathBits.h"
#include "../RC/Types.h"
#include "../RC/RND.h"

#include <cmath>

namespace RCmath {

class Stats {
  public:

  template<class T>
  static T Sum(const RC::Data1D<T> &arr) {
    T val = 0;
    for (size_t i=0; i<arr.size(); i++) {
      val += arr[i];
    }
    return val;
  }

  template<class T>
  static T Sum(const RC::Data2D<T> &arr) {
    T val = 0;
    for (size_t i=0; i<arr.size2(); i++) {
      val += Sum(arr[i]);
    }
    return val;
  }

  template<class T>
  static T Sum(const RC::Data3D<T> &arr) {
    T val = 0;
    for (size_t i=0; i<arr.size3(); i++) {
      val += Sum(arr[i]);
    }
    return val;
  }


  template<class T>
  static T Min(const RC::Data1D<T> &arr) {
    T minval = arr[0];
    for (size_t i=1; i<arr.size(); i++) {
      if (arr[i] < minval) {
        minval = arr[i];
      }
    }
    return minval;
  }

  template<class T>
  static T Min(const RC::Data2D<T> &arr) {
    T minval = arr[0];
    for (size_t i=1; i<arr.size2(); i++) {
      T currval = Min(arr[i]);
      if (currval < minval) {
        minval = currval;
      }
    }
    return minval;
  }

  template<class T>
  static T Min(const RC::Data3D<T> &arr) {
    T minval = arr[0];
    for (size_t i=1; i<arr.size3(); i++) {
      T currval = Min(arr[i]);
      if (currval < minval) {
        minval = currval;
      }
    }
    return minval;
  }

  template<class T>
  static T Max(const RC::Data1D<T> &arr) {
    T maxval = arr[0];
    for (size_t i=1; i<arr.size(); i++) {
      if (maxval < arr[i]) {
        maxval = arr[i];
      }
    }
    return maxval;
  }

  template<class T>
  static T Max(const RC::Data2D<T> &arr) {
    T maxval = arr[0];
    for (size_t i=1; i<arr.size2(); i++) {
      T currval = Max(arr[i]);
      if (maxval < currval) {
        maxval = currval;
      }
    }
    return maxval;
  }

  template<class T>
  static T Max(const RC::Data3D<T> &arr) {
    T maxval = arr[0];
    for (size_t i=1; i<arr.size3(); i++) {
      T currval = Max(arr[i]);
      if (maxval < currval) {
        maxval = currval;
      }
    }
    return maxval;
  }


  template<class T>
  static T Mean(const RC::Data1D<T> &arr) { return Sum(arr) / arr.size(); }
  template<class T>
  static T Mean(const RC::Data2D<T> &arr) {
    return Sum(arr) / (arr.size1() * arr.size2());
  }
  template<class T>
  static T Mean(const RC::Data3D<T> &arr) {
    return Sum(arr) / (arr.size1() * arr.size2() * arr.size3());
  }


  template<class T>
  static T Var(const RC::Data1D<T> &arr) {
    T meanval = Mean(arr);
    T var = 0;
    for (size_t i=0; i<arr.size(); i++) {
      T diff = arr[i] - meanval;
      var += diff * diff;
    }
    return var / (arr.size() - 1);
  }

  template<class T>
  static T Var(const RC::Data2D<T> &arr) {
    T meanval = Mean(arr);
    T var = 0;
    for (size_t j=0; j<arr.size2(); j++) {
      for (size_t i=0; i<arr.size1(); i++) {
        T diff = arr[j][i] - meanval;
        var += diff * diff;
      }
    }
    return var / (arr.size1() * arr.size2() - 1);
  }

  template<class T>
  static T Var(const RC::Data3D<T> &arr) {
    T meanval = Mean(arr);
    T var = 0;
    for (size_t k=0; k<arr.size3(); k++) {
      for (size_t j=0; j<arr.size2(); j++) {
        for (size_t i=0; i<arr.size1(); i++) {
          T diff = arr[k][j][i] - meanval;
          var += diff * diff;
        }
      }
    }
    return var / (arr.size1() * arr.size2() * arr.size3() - 1);
  }


  // Computes overlapped size and mean across that size.
  template<class T>
  static inline void CommonMean(const RC::Data1D<T> &x, const RC::Data1D<T> &y,
                         size_t& size, T& mean_x, T& mean_y) {
    size = RCmath::Min(x.size(), y.size());

    mean_x = 0;
    mean_y = 0;
    for (size_t i=0; i<size; i++) {
      mean_x += x[i];
      mean_y += y[i];
    }
    mean_x /= size;
    mean_y /= size;
  }

  template<class T>
  static inline void CommonMean(const RC::Data2D<T> &x, const RC::Data2D<T> &y,
                         size_t& size1, size_t& size2, T& mean_x, T& mean_y) {
    size1 = RCmath::Min(x.size1(), y.size1());
    size2 = RCmath::Min(x.size2(), y.size2());

    mean_x = 0;
    mean_y = 0;
    for (size_t i2=0; i2<size2; i2++) {
      for (size_t i1=0; i1<size1; i1++) {
        mean_x += x[i2][i1];
        mean_y += y[i2][i1];
      }
    }
    size_t count = size1*size2;
    mean_x /= count;
    mean_y /= count;
  }

  template<class T>
  static inline void CommonMean(const RC::Data3D<T> &x, const RC::Data3D<T> &y,
                         size_t& size1, size_t& size2, size_t& size3,
                         T& mean_x, T& mean_y) {
    size1 = RCmath::Min(x.size1(), y.size1());
    size2 = RCmath::Min(x.size2(), y.size2());
    size3 = RCmath::Min(x.size3(), y.size3());

    mean_x = 0;
    mean_y = 0;
    for (size_t i3=0; i3<size3; i3++) {
      for (size_t i2=0; i2<size2; i2++) {
        for (size_t i1=0; i1<size1; i1++) {
          mean_x += x[i3][i2][i1];
          mean_y += y[i3][i2][i1];
        }
      }
    }
    size_t count = size1*size2*size3;
    mean_x /= count;
    mean_y /= count;
  }


  // Returns covariance, ignoring extra data if different sizes.
  template<class T>
  static inline T Cov(const RC::Data1D<T> &x, const RC::Data1D<T> &y) {
    size_t i;

    size_t size;
    T mean_x, mean_y;
    CommonMean(x, y, size, mean_x, mean_y);

    T covar = 0;
    for (i=0; i<size; i++) {
      covar += (x[i] - mean_x) * (y[i] - mean_y);
    }
    covar /= size - 1;

    return covar;
  }

  template<class T>
  static inline T Cov(const RC::Data2D<T> &x, const RC::Data2D<T> &y) {
    size_t i1, i2;

    size_t size1, size2;
    T mean_x, mean_y;
    CommonMean(x, y, size1, size2, mean_x, mean_y);
    size_t count = size1*size2;

    T covar = 0;
    for (i2=0; i2<size2; i2++) {
      for (i1=0; i1<size1; i1++) {
        covar += (x[i2][i1] - mean_x) * (y[i2][i1] - mean_y);
      }
    }
    covar /= count - 1;

    return covar;
  }


  template<class T>
  static inline T Cov(const RC::Data3D<T> &x, const RC::Data3D<T> &y) {
    size_t i1, i2, i3;

    size_t size1, size2, size3;
    T mean_x, mean_y;
    CommonMean(x, y, size1, size2, size3, mean_x, mean_y);

    size_t count = size1*size2*size3;

    T covar = 0;
    for (i3=0; i3<size3; i3++) {
      for (i2=0; i2<size2; i2++) {
        for (i1=0; i1<size1; i1++) {
          covar += (x[i3][i2][i1] - mean_x) * (y[i3][i2][i1] - mean_y);
        }
      }
    }
    covar /= count - 1;

    return covar;
  }


  template<class T>
  static T SD(const RC::Data1D<T> &arr) { return Sqrt(Var(arr)); }
  template<class T>
  static T SD(const RC::Data2D<T> &arr) { return Sqrt(Var(arr)); }
  template<class T>
  static T SD(const RC::Data3D<T> &arr) { return Sqrt(Var(arr)); }


  template<class T>
  static T Median(const RC::Data1D<T> &arr) {
    RC::Data1D<T> copy = arr;
    copy.Sort();
    return copy[copy.size()/2];
  }


  template<class T>
  static f64 DistMean(const RC::Data1D<T> &arr) {
    f64 weighted_sum = 0;
    f64 sum = 0;
    for (size_t i=0; i<arr.size(); i++) {
      weighted_sum += i*arr[i];
      sum += arr[i];
    }
    return weighted_sum / sum;
  }

  template<class T>
  static f64 DistMedian(const RC::Data1D<T> &arr) {
    T half = Sum(arr) / 2;
    T part = 0;
    for (size_t i=0; i<arr.size(); i++) {
      part += arr[i];
      if (part >= half) {
        return (i + 0.5 - (part-half)/(f64)arr[i]);
      }
    }
    return arr.size()-1;
  }


  template<class T>
  static void MultBy(RC::Data1D<T> &arr, const T &mult_by) {
    for (size_t i=0; i<arr.size(); i++) {
      arr[i] *= mult_by;
    }
  }

  template<class T>
  static void MultBy(RC::Data2D<T> &arr, const T &mult_by) {
    for (size_t j=0; j<arr.size2(); j++) {
      for (size_t i=0; i<arr.size1(); i++) {
        arr[i] *= mult_by;
      }
    }
  }

  template<class T>
  static void MultBy(RC::Data3D<T> &arr, const T &mult_by) {
    for (size_t k=0; k<arr.size3(); k++) {
      for (size_t j=0; j<arr.size2(); j++) {
        for (size_t i=0; i<arr.size1(); i++) {
          arr[i] *= mult_by;
        }
      }
    }
  }

  template<class T>
  static void Normalize(RC::Data1D<T> &arr) {
    RC::AssertFloat<T>();
    T sum = Sum(arr);
    MultBy(arr, 1/sum);
  }

  template<class T>
  static void Normalize(RC::Data2D<T> &arr) {
    RC::AssertFloat<T>();
    T sum = Sum(arr);
    MultBy(arr, 1/sum);
  }

  template<class T>
  static void Normalize(RC::Data3D<T> &arr) {
    RC::AssertFloat<T>();
    T sum = Sum(arr);
    MultBy(arr, 1/sum);
  }


  template<class T> // (1/sqrt(2*PI)*exp(-x*x/2)
  static inline T Gaus(T x) { return 0.3989422804014326779L*Exp(-x*x/2); }

//  template<class T>
//  class BetaHelper {
//    public: BetaHelper(T x, T y) : x(x), y(y) { }
//            T operator()(T t) const { return Pow(t,x-1)*Pow(1-t,y-1); }
//    private: const T x, y;
//  };
//  template<class T>
//  static inline T Beta(T x, T y) {
//    BetaHelper<T> f(x, y);
//    return Calc::Romberg<T>(f, 0, 1);
//  }
  template<class T>
  static inline T Beta(T x, T y) { return Gamma(x)*Gamma(y)/Gamma(x+y); }

  template<class T>
  static inline T FDist(T x, T d1, T d2) {
    if (x == 0) { return 0; }
    return (Pow(d2,d2/2)*Pow(d1,d1/2)*Pow(x,d1/2-1))
           / (Pow(d2+d1*x,(d1+d2)/2)*Beta(d1/2,d2/2));
  }
  template<class T>
  class FDistFunc {
    public: FDistFunc(T d1, T d2) : d1(d1), d2(d2) { }
            T operator()(T x) const { return FDist(x, d1, d2); }
    private: const T d1, d2;
  };

  // Returns p-val for outside, or rejecting null hypothesis.
  template<class T>
  static inline T FDistPVal(T F_val, T d1, T d2) {
    FDistFunc<T> fdist(d1, d2);
    return 1-Calc::Romberg<T>(fdist, (T)0, F_val);
  }

  template<class T>
  static inline T Fact(T x) { return Gamma(x+1); }

  static inline u64 Fact(u64 n) {
    const u64 fact_table[21] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880,
      3628800, 39916800, 479001600, 6227020800ull, 87178291200ull,
      1307674368000ull, 20922789888000ull, 355687428096000ull,
      6402373705728000ull, 121645100408832000ull, 2432902008176640000ull};
    if (n > 20) { return -1; }
    return fact_table[n];
  }
  static inline u64 Fact(i64 n) { return Fact((u64)n); }
  static inline u32 Fact(u32 n) {
    if (n > 12) { return -1; }
    return Fact(n);
  }
  static inline u32 Fact(i32 n) { return Fact((u32)n); }

  template<class T>  // n choose c
  static inline T Comb(T n, T c) { return Fact(n)/(Fact(c)*Fact(n-c)); }

  template<class T>
  static inline T Binomial(T prob, T trials, T succ) {
    return Comb(trials, succ)*Pow(prob, succ)*Pow(1-prob, trials-succ);
  }
  template<class T>
  class BinomialFunc {
    public: BinomialFunc(T prob, T trials) : prob(prob), trials(trials) { }
            T operator()(T succ) const {return Binomial(prob, trials, succ); }
    private: const T prob, trials;
  };

  template<class T>  // mean is expected value = prob*trials
  static inline T Poisson(T mean, T succ) {
    return Pow(mean, succ)*Exp(-mean)/Fact(succ);
  }
  template<class T>
  class PoissonFunc {
    public: PoissonFunc(T mean) : mean(mean) { }
            T operator()(T succ) const {return Poisson(mean, succ); }
    private: const T mean;
  };

};

class NormRND : public RC::RND {
#ifdef CPP11
  bool flipflop = false;
  f64 store;
#endif
  public:
  inline f64 GetNormal() {
#ifndef CPP11
    static bool flipflop = false;
    static f64 store;
#endif
    if (flipflop) {
      flipflop = !flipflop;
      return store;
    }
    else {
      flipflop = !flipflop;
      f64 tmp1 = Get_f64();
      f64 tmp2 = Get_f64();
      store = Sqrt(-2*Log(tmp1))*Cos(TWO_PI*tmp2);
      return Sqrt(-2*Log(tmp1))*Sin(TWO_PI*tmp2);
    }
  }
};


class Avgf64 {
  protected:
  f64 avg;
  u64 cnt;
  public:
  inline Avgf64() { avg = 0; cnt = 0; }
  inline Avgf64& operator+= (const f64 x) {
    cnt++;
    avg += (x-avg)/cnt;
    return *this;
  }
  inline operator f64 () const { return avg; }
  inline u64 Count() const { return cnt; }
};

}

#endif // RCMATH_STATS_H

