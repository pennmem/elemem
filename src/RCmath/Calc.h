#ifndef RCMATH_CALC_H
#define RCMATH_CALC_H

#include "../RC/Data1D.h"
#include "../RC/Data2D.h"
#include "../RC/Data3D.h"
#include "../RC/Types.h"
#include <cmath>


class Calc {
  public:

  template<class T, typename F>
  static T Romberg(F func, T x_low, T x_high) {
    RC::AssertFloat<T>();
    const size_t rarr_size = 28;
    RC::Data2D<T> romb_arr(rarr_size, rarr_size);
    T h, sum, mpow, mpowterm, result;
    u64 n, m, end, k2min1, k2min1_end;

    h = (T)(x_high - x_low);
    k2min1_end = 1;
    romb_arr[0][0] = 0.5*h*(func((T)x_high) + func((T)x_low));

    for (n=1; n<rarr_size; n++) {
      h *= 0.5;

      k2min1_end *= 2;
      sum = 0;
      for (k2min1=1; k2min1<=k2min1_end; k2min1 += 2) {
        sum += func((T)(x_low + k2min1*h));
      }
      romb_arr[n][0] = 0.5*romb_arr[n-1][0] + h*sum;

      mpow = 4;
      for (m=1; m<=n; m++) {
        mpowterm = 1 / (mpow - 1);
        romb_arr[n][m] = romb_arr[n][m-1]
                         + mpowterm * (romb_arr[n][m-1] - romb_arr[n-1][m-1]);
        mpow *= 4;
      }

      end = n;
      if (romb_arr[end][end] == romb_arr[end][end-1] ||
          ! std::isnormal(romb_arr[end][end])) {
        break;
      }
    }

    result = romb_arr[end][end];

    return result;
  }
};


#endif // RCMATH_CALC_H

