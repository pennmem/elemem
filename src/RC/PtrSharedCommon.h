/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
//
/// \file PtrSharedCommon.h
/// Common components for the shared *Ptr.h files.  Do not include directly.
/////////////////////////////////////////////////////////////////////

    public:

    /// Returns a direct reference to the enclosed pointer.
    /** The reference is read/write, so it can be used as a function
     *  parameter which updates the value of this pointer.
     *  @return A reference to the enclosed pointer.
     */
    inline T* Raw() const { return helper->t_ptr; }

    protected:
    /// @cond PROTECTED

    /// This class gets tossed along with assignments, and maintains
    /// a reference count.
    class PtrHelper {
      public:

      inline PtrHelper(T *t_ptr, bool auto_delete)
        : t_ptr (t_ptr),
          cnt (1),
          auto_delete(auto_delete) {
      }


      inline void Add() {
        ++cnt;
      }

      inline void Del() {
        u64 cur_cnt = --cnt;
        if (cur_cnt == 0) {
          if (auto_delete) {
            Delete();
          }
          delete(this);
        }
      }

      inline void Delete() {
        T* loc_t_ptr;
#ifdef CPP11
        loc_t_ptr = t_ptr.exchange(NULL);
#else
        loc_t_ptr = t_ptr;
        t_ptr = NULL;
#endif
        if (loc_t_ptr != NULL) {
          delete(loc_t_ptr);
        }
      }

      inline void Revoke() {
        t_ptr = NULL;
      }

#ifdef CPP11
      std::atomic<T*> t_ptr;
      std::atomic<u64> cnt;
#else
      T *t_ptr;
      u64 cnt;
#endif
      bool auto_delete;
    };
    /// @endcond

