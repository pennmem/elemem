#ifndef RCMATH_COORD_H
#define RCMATH_COORD_H

#include "Affine.h"
#include "MathBits.h"
#include "../RC/RCconfig.h"
#include "../RC/RStr.h"
#include "../RC/Caller.h"
#include <iostream>
#include <map>

namespace RCmath {
  template <class T>
  class Coord {
    public:
    inline Coord() : x(0), y(0) { }
    inline Coord(const T& x, const T& y) : x(x), y(y) { }
    T x;
    T y;

    template<class T2>
    inline Coord(const Coord<T2>& other) : x(other.x), y(other.y) { }
    template<class T2>
    inline Coord& operator=(const Coord<T2>& other) {
      x = other.x;
      y = other.y;
    }

    inline T Angle() const {
      return Atan2(y, x);
    }


    inline T Mag() const {
      return Sqrt(x*x+y*y);
    }

    template<class T2>
    inline T Dot(const Coord<T2>& other) const {
      return x*other.x + y*other.y;
    }


    // Changes this object.
    template<class T2>
    inline Coord& Rot(const T2& angle) {
#ifdef CPP11
      typename std::remove_reference<T>::type tmp;
#else
      T tmp;
#endif
      tmp = x*Cos(angle) - y*Sin(angle);
      y = x*Sin(angle) + y*Cos(angle);
      x = tmp;
      return (*this);
    }


    template<class T2>
    inline Coord& operator*= (const T2& val) {
      x *= val;
      y *= val;
      return (*this);
    }

    template<class T2>
    inline Coord operator* (const T2& val) const {
      Coord c = (*this);
      c *= val;
      return c;
    }

    template<class T2>
    inline Coord& operator/= (const T2& val) {
      x /= val;
      y /= val;
      return (*this);
    }

    template<class T2>
    inline Coord operator/ (const T2& val) const {
      Coord c = (*this);
      c /= val;
      return c;
    }


    template<class T2>
    inline Coord& operator*= (const Affine<T2>& a) {
#ifdef CPP11
      typename std::remove_reference<T>::type tmp;
#else
      T tmp;
#endif
      tmp = a.a00*x + a.a01*y + a.a02;
      y = a.a10*x + a.a11*y + a.a12;
      x = tmp;
      return (*this);
    }


    template<class T2>
    inline Coord& operator*= (const Coord<T2>& other) {
      x *= other.x;
      y *= other.y;
      return (*this);
    }

    template<class T2>
    inline Coord operator* (const Coord<T2>& other) const {
      Coord c = (*this);
      c *= other;
      return c;
    }


    template<class T2>
    inline Coord& operator+= (const Coord<T2>& other) {
      x += other.x;
      y += other.y;
      return (*this);
    }

    template<class T2>
    inline Coord operator+ (const Coord<T2>& other) const {
      Coord c = (*this);
      c += other;
      return c;
    }


    template<class T2>
    inline Coord& operator-= (const Coord<T2>& other) {
      x -= other.x;
      y -= other.y;
      return (*this);
    }

    template<class T2>
    inline Coord operator- (const Coord<T2>& other) const {
      Coord<decltype(x-other.x)> c = (*this);
      c -= other;
      return c;
    }


    inline Coord Unit() const {
      return (*this) / Mag();
    }


    inline RC::RStr ToRStr() const {
      return (RC::RStr("(") + RC::RStr(x) + ", " + RC::RStr(y) + ")");
    }
  };


  template<class T1, class T2>
  inline Coord<T2> operator* (const Affine<T1>& a, const Coord<T2>& other) {
    Coord<T2> c = other;
    c *= a;
    return c;
  }

  template<class T1, class T2>
  inline Coord<T2> operator* (const T1& val, const Coord<T2>& other) {
    Coord<T2> c = other;
    other *= val;
    return c;
  }

  template<class T>
  inline std::ostream& operator<< (std::ostream &out, const Coord<T>& c) {
    out << "(" << c.x << ", " << c.y << ")";
    return out;
  }


  template<class T, class CT=T>
  class ParaCoord : public Coord<CT> {
    public:
    ParaCoord() : para(0) { }
    ParaCoord(T para, CT x, CT y)
      : Coord<CT>(x, y), para(para) { }
    ParaCoord(T para, const Coord<CT>& coord)
      : Coord<CT>(coord), para(para) { }

    T para;
  };


  template<class T, class CT=T>
  class ParaCurve {
    protected:
    typedef std::map< T, Coord<CT> > DType;
    typedef std::pair< T, Coord<CT> > DValType;

    inline ParaCoord<T,CT> ClosestHelper(ParaCoord<T,CT> p1, ParaCoord<T,CT>p2,
                                         Coord<CT> targ) {
      Coord<CT> diff = p2-p1;
      Coord<CT> tdiff = targ-p1;
      Coord<CT> unit1 = diff.Unit();
      Coord<CT> unit2(-unit1.x, -unit1.y);

      CT dist1 = tdiff.Dot(unit1);  // Signed distance.
      CT total_dist = diff.Mag();

      if (dist1 <= 0) {
        return p1;
      }
      if (dist1 >= total_dist) {
        return p2;
      }

      T frac = (T)dist1 / (T)total_dist;
      T para = p1.para * frac + p2.para * (1-frac);

      return ParaCoord<T,CT>(para, (p1 + unit1 * dist1));
    }

    public:

    inline ParaCurve() : trace_valid(true) { }

    inline void Clear() { data.clear(); }

    inline Coord<CT> Interp(const T& seek) {
      if (data.size() < 1) {
        Throw_RC_Error("Empty curve");
      }

      typename DType::iterator it = data.upper_bound(seek);
      if (it == data.end() || (++it-- == data.end())) {
        return data.rbegin()->second;
      }

      T par1 = it->first;
      Coord<CT> c1 = it->second;
      ++it;
      T par2 = it->first;
      Coord<CT> c2 = it->second;

      f64 ratio = (seek - par1)/((f64)(par2-par1));
      f64 omratio = 1-ratio;

      return Coord<CT>(omratio*c1.x + ratio*c2.x, omratio*c1.y + ratio*c2.y);
    }

    inline ParaCurve& operator+= (const ParaCoord<T,CT>& newpc) {
      trace_valid = false;
      data.insert(DValType(newpc.para, newpc));
      return *this;
    }

    inline RC::Data1D< ParaCoord<T,CT> >& Trace() {
      if ( ! trace_valid ) {
        trace.Resize(0);
        trace.Reserve(data.size());
        for (typename DType::iterator it=data.begin(); it!=data.end(); ++it) {
          trace += ParaCoord<T,CT>(it->first, it->second);
        }
      }
      return trace;
    }

    public:
    inline ParaCoord<T,CT> Closest(const Coord<CT>& coord) {
      if (data.size() < 1) {
        Throw_RC_Error("Empty curve");
      }

      Trace();

      CT min_dist = (trace[0] - coord).Mag();
      ParaCoord<CT> closest = trace[0];
      for (size_t i=0; i<trace.size()-1; i++) {
        ParaCoord<CT> tmp = ClosestHelper(trace[i], trace[i+1], coord);
        CT dist = (tmp - coord).Mag();
        if (dist < min_dist) {
          min_dist = dist;
          closest = tmp;
        }
      }

      return closest;
    }

#ifdef CPP11
    protected:
    inline bool ValidCoord(Coord<CT> coord, Coord<CT>& min_coords,
                           Coord<CT>& max_coords) {
      return (coord.x >= min_coords.x && coord.x <= max_coords.x &&
              coord.y >= min_coords.y && coord.y <= max_coords.y);
    }
    inline void ConditionalAdd(T val, Coord<CT> coord, Coord<CT>& min_coords,
                               Coord<CT>& max_coords) {
      if (ValidCoord(coord, min_coords, max_coords)) {
        (*this) += ParaCoord<T,CT>(val, coord);
      }
    }
    inline void AddInRange(RC::Caller<Coord<CT>, T>& func,
                           RC::RND& rng, T min, T max, Coord<CT>& min_coords,
                           Coord<CT>& max_coords) {
        f64 rnd = rng.Get_f64();
        T val = min*(1-rnd) + max*rnd;
        ConditionalAdd(val, func(val), min_coords, max_coords);
    }
    inline bool BinaryShrink(T& val, RC::Caller<Coord<CT>, T>& func,
                           Coord<CT>& min_coords, Coord<CT>& max_coords) {
      if (ValidCoord(func(val), min_coords, max_coords)) {
        return true;
      }
      f64 mult = 0.5;
      f64 last_valid = 0;
      do {
        if (ValidCoord(func(val*mult), min_coords, max_coords)) {
          last_valid = mult;
          mult = 1-0.5*(1-mult);
        }
        else {
          val *= mult;
          if (val == 0) {
            return false;
          }
        }
      } while (last_valid < 0.999999);
      val *= last_valid;
      return true;
    }
    inline bool BinaryGrow(T& val, RC::Caller<Coord<CT>, T>& func,
                           Coord<CT>& min_coords, Coord<CT>& max_coords) {
      if (ValidCoord(func(val), min_coords, max_coords)) {
        return true;
      }
      f64 mult = 2;
      f64 last_valid = mult;
      T previous_val = val;
      do {
        if (ValidCoord(func(val*mult), min_coords, max_coords)) {
          last_valid = mult;
          mult = 1+0.5*(mult-1);
        }
        else {
          previous_val = val;
          val *= mult;
          if (!(Abs(previous_val) < Abs(val))) {
            return false;
          }
        }
      } while (last_valid > 1.000001);
      val *= last_valid;
      return true;
    }
    public:

    // Probabilistically fills in points that are roughly equidistant.
    // Runs in O(num_points * Sqrt(num_points))
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func,
                         size_t num_points, T min, T max,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      Clear();

      RC::RND rng(RC::RND::CONST_SEED);

      if (num_points == 0) {
        return;
      }

      size_t points_per_iter = Sqrt(num_points);

      bool min_valid = ValidCoord(func(min), min_coords, max_coords);
      bool max_valid = ValidCoord(func(max), min_coords, max_coords);

      if (!max_valid && max > 0) {
        max_valid = BinaryShrink(max, func, min_coords, max_coords);
      }
      if (!max_valid && max <= 0) {
        if (max == 0) { max = -RC::MIN_POS<T>(); }
        max_valid = BinaryGrow(max, func, min_coords, max_coords);
      }

      if (!min_valid && min < 0) {
        min_valid = BinaryShrink(min, func, min_coords, max_coords);
      }
      if (!min_valid && min >= 0) {
        if (min == 0) { min = RC::MIN_POS<T>(); }
        min_valid = BinaryGrow(min, func, min_coords, max_coords);
      }

      ConditionalAdd(min, func(min), min_coords, max_coords);
      if (data.size() >= num_points) {
        return;
      }
      ConditionalAdd(max, func(max), min_coords, max_coords);

      if ( ! (min < max) ) {
        return;
      }

      size_t loop_size = Min(4*points_per_iter, num_points - data.size() + 1);
      for (size_t loop=0; loop<loop_size; loop++) {
        if (data.size() >= num_points) {
          return;
        }
        f64 frac = (loop+(f64)1)/loop_size; 
        T val = min*(1-frac) + max*frac;
        ConditionalAdd(val, func(val), min_coords, max_coords);
      }

      size_t iter_count = 0;
      while (data.size() < num_points && (iter_count++ < num_points)) {
        typename DType::iterator it=data.begin();
        typename DType::iterator last = it;
        f64 dist_sum = 0;
        if (it == data.end()) {
          Throw_RC_Error("Error iterating in AutoPlot");
        }
        for (++it; it!=data.end(); ++it) {
          f64 dist = (last->second - it->second).Mag();
          dist_sum += dist;

          last = it;
        }

        f64 dist_sum_inv = ((f64)1) / dist_sum;
        it = data.begin();
        last = it;
        for (++it; it!=data.end(); ++it) {
          if (data.size() >= num_points) {
            return;
          }

          f64 dist = (last->second - it->second).Mag();
          f64 frac = dist * dist_sum_inv;
          f64 prob = frac * points_per_iter;

          if (prob > 1) {
            for (size_t i=1; i<prob; i++) {
              AddInRange(func, rng, last->first, it->first,
                         min_coords, max_coords);
            }
          }
          else if (rng.GetProb(prob)) {
            AddInRange(func, rng, last->first, it->first,
                       min_coords, max_coords);
          }

          last = it;
        }
      }
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func,
                         size_t num_points, T min, T max) {
      AutoPlot(func, num_points, min, max,
               Coord<CT>(RC::LOW_VAL<CT>(), RC::LOW_VAL<CT>()),
               Coord<CT>(RC::MAX_VAL<CT>(), RC::MAX_VAL<CT>()));
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func, T min, T max) {
      AutoPlot(func, 16384, min, max);
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func,
                         size_t num_points) {
      AutoPlot(func, num_points, RC::LOW_VAL<T>(), RC::MAX_VAL<T>());
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func) {
      AutoPlot(func, 16384, RC::LOW_VAL<T>(), RC::MAX_VAL<T>());
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      AutoPlot(func, 16384, RC::LOW_VAL<T>(), RC::MAX_VAL<T>(),
               min_coords, max_coords);
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func, size_t num_points,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      AutoPlot(func, num_points, RC::LOW_VAL<T>(), RC::MAX_VAL<T>(),
               min_coords, max_coords);
    }
    inline void AutoPlot(RC::Caller<Coord<CT>, T> func, T min, T max,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      AutoPlot(func, 16384, min, max, min_coords, max_coords);
    }

#endif // CPP11


    protected:
    DType data;
    bool trace_valid;
    RC::Data1D< ParaCoord<T,CT> > trace;
  };

#ifdef CPP11
  template<class CT>
  class CurveCallerWrapper {
    public:
    CurveCallerWrapper(RC::Caller<CT, CT> func) : func(func) { }
    Coord<CT> operator() (CT val) const { return Coord<CT>(val, func(val)); }
    protected:
    RC::Caller<CT, CT> func;
  };
#endif // CPP11

  template<class CT>
  class Curve : public ParaCurve<CT> {
    public:

#ifdef CPP11
    inline void AutoPlot(RC::Caller<CT, CT> func,
                         size_t num_points, CT min, CT max,
                         Coord<CT> min_coords, Coord<CT> max_coords) {

      ParaCurve<CT>::AutoPlot(CurveCallerWrapper<CT>(func), num_points,
                              min, max, min_coords, max_coords);
    }
    inline void AutoPlot(RC::Caller<CT, CT> func,
                         size_t num_points, CT min, CT max) {
      AutoPlot(func, num_points, min, max,
               Coord<CT>(RC::LOW_VAL<CT>(), RC::LOW_VAL<CT>()),
               Coord<CT>(RC::MAX_VAL<CT>(), RC::MAX_VAL<CT>()));
    }
    inline void AutoPlot(RC::Caller<CT, CT> func, CT min, CT max) {
      AutoPlot(func, 16384, min, max);
    }
    inline void AutoPlot(RC::Caller<CT, CT> func,
                         size_t num_points) {
      AutoPlot(func, num_points, RC::LOW_VAL<CT>(), RC::MAX_VAL<CT>());
    }
    inline void AutoPlot(RC::Caller<CT, CT> func) {
      AutoPlot(func, 16384, RC::LOW_VAL<CT>(), RC::MAX_VAL<CT>());
    }
    inline void AutoPlot(RC::Caller<CT, CT> func,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      AutoPlot(func, 16384, RC::LOW_VAL<CT>(), RC::MAX_VAL<CT>(),
               min_coords, max_coords);
    }
    inline void AutoPlot(RC::Caller<CT, CT> func, size_t num_points,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      AutoPlot(func, num_points, RC::LOW_VAL<CT>(), RC::MAX_VAL<CT>(),
               min_coords, max_coords);
    }
    inline void AutoPlot(RC::Caller<CT, CT> func, CT min, CT max,
                         Coord<CT> min_coords, Coord<CT> max_coords) {
      AutoPlot(func, 16384, min, max, min_coords, max_coords);
    }
#endif // CPP11
  };
}


#endif // RCMATH_COORD_H

