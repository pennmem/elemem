/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file File.h
/// Provides file input and output, and file / directory tools.
/////////////////////////////////////////////////////////////////////

#ifndef RC_FILE_H
#define RC_FILE_H

#include "RCconfig.h"
#include "Types.h"
#include "Errors.h"
#include "Ptr.h"
#include "Data1D.h"
#include "RStr.h"

#ifdef WIN32
#include <winsock2.h>
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef CPP11
#include <atomic>
#include <type_traits>
#endif

/// @cond UNDOC
#ifndef O_BINARY
#ifdef _O_BINARY
#define O_BINARY _O_BINARY
#else
#define O_BINARY 0
#endif
#endif
/// @endcond


#ifdef _MSC_VER
inline int open(const char *pathname, int flags) {
  return _open(pathname, flags);
}
inline int open(const char *pathname, int flags, mode_t mode) {
  return _open(pathname, flags, mode);
}
#endif


namespace RC {
  class FileRead;
  class FileWrite;

  /// Overload this with a specialization to support Get on a custom type.
  /** An implementation of this should use the FileRead's methods Get and
   *  RawGet to read other supported types or primitives, then place the
   *  result in data.
   *  @return True if the Get succeeded.
   */  
  template<class T> inline bool FileGetWrapper(FileRead& fr, T& data);
  /// Overload this with a specialization to support Put on a custom type.
  /** An implementation of this should take the value of data and use the
   *  FileWrite's methods Put and RawPut to write other supported types or
   *  primitives.
   */  
  template<class T> inline void FilePutWrapper(FileWrite& fw, const T& data);

  /// Valid write modes for a FileWrite or FileRW.
  /** TRUNCATE sets the file size to 0.  KEEP preserves the file contents,
   *  setting the initial position to the beginning of the file.  APPEND
   *  starts the read position at the beginning of the file but all Write/Put
   *  operations are forced to the end of the file.  NEWONLY will only
   *  succeed if the file does not already exist.
   */
  enum WriteMode { TRUNCATE=0, KEEP, APPEND, NEWONLY }; // Do not reorder.

  /// Provides the common methods for the FileRead/FileWrite/FileRW classes.
  class FileBase {
    protected:
    /// @cond PROTECTED

    // This class gets tossed along with assignments of FileRead/FileWrite,
    // and maintains a reference count
    class FileHelper {
      public:

      inline FileHelper()
        : fp (NULL),
          cnt (1),
          index (0),
          amnt_read (0),
          do_close (true),
          is_readable (false),
          is_writable (false),
          last_was_read (false),
          last_was_write (false) {
      }


      inline void Add() {
        ++cnt;
      }

      inline void Del() {
        u64 cur_cnt = --cnt;
        if (cur_cnt == 0) {
          Close();
          delete(this);
        }
      }

      inline void Close() {
        FILE* loc_fp;
  #ifdef CPP11
        loc_fp = fp.exchange(NULL);
  #else
        loc_fp = fp;
        fp = NULL;
  #endif
        if (do_close && (loc_fp != NULL)) {
          fclose(loc_fp);
        }
      }

      inline bool IsWriteBuf() const {
        return index > amnt_read;
      }

      inline bool IsReadBuf() const {
        return amnt_read > index;
      }

  #ifdef CPP11
      std::atomic<FILE*> fp;
      std::atomic<u64> cnt;
  #else
      FILE* fp;
      u64 cnt;
  #endif

      Data1D<u8> buf;
      size_t index;
      size_t amnt_read;

      bool do_close;
      RStr filename;
      bool is_readable;
      bool is_writable;
      bool last_was_read;
      bool last_was_write;
    };

    // Ordering coupled to Open implementations.
    enum BaseMode { R_ONLY=0, W_TRUNC, W_KEEP, W_APP, W_NEW,
                    RW_TRUNC, RW_KEEP, RW_APP, RW_NEW };

    inline bool BaseOpen(const RStr& filename, const BaseMode mode) {
      helper->Del();

      helper = new FileHelper();

      helper->filename = filename;
      RStr mode_str;

      int flags = O_BINARY;

      switch (mode) {
        case R_ONLY:    mode_str = "rb";
                        flags |= O_RDONLY;
                        break;
        case W_TRUNC:   mode_str = "wb";
                        flags |= O_WRONLY | O_CREAT | O_TRUNC;
                        break;
        case W_KEEP:    mode_str = "wb";
                        flags |= O_WRONLY | O_CREAT;
                        break;
        case W_APP:     mode_str = "ab";
                        flags |= O_WRONLY | O_CREAT | O_APPEND;
                        break;
        case W_NEW:     mode_str = "wb";
                        flags |= O_WRONLY | O_CREAT | O_EXCL;
#ifdef unix
                        flags |= O_NOFOLLOW;
#endif
                        break;
        case RW_TRUNC:  mode_str = "wb+";
                        flags |= O_RDWR | O_CREAT | O_TRUNC;
                        break;
        case RW_KEEP:   mode_str = "rb+";
                        flags |= O_RDWR | O_CREAT;
                        break;
        case RW_APP:    mode_str = "ab+";
                        flags |= O_RDWR | O_CREAT | O_APPEND;
                        break;
        case RW_NEW:    mode_str = "wb+";
                        flags |= O_RDWR | O_CREAT | O_EXCL;
#ifdef unix
                        flags |= O_NOFOLLOW;
#endif
                        break;
        default:        Throw_RC_Type(File, "Invalid mode");
      }

#ifdef WIN32
      int create_mode = _S_IREAD | _S_IWRITE;
#else
      int create_mode = 0664;
#endif
      int fd = open(filename.c_str(), flags, create_mode);
      if (fd >= 0) {
        helper->fp = fdopen(fd, mode_str.c_str());

        if (helper->fp == NULL) {
          close(fd);  // If fdopen fails, must close fd manually.
        }
      }

      if (helper->fp == NULL) {
        return false;
      }

      if ((mode == R_ONLY) || (mode == RW_TRUNC) ||
          (mode == RW_APP) || (mode == RW_NEW)) {
        helper->is_readable = true;
      }

      if ((mode == W_TRUNC)  || (mode == W_APP)  || (mode == W_NEW) ||
          (mode == RW_TRUNC) || (mode == RW_APP) || (mode == RW_NEW)) {
        helper->is_writable = true;
      }
      return true;
    }

    inline void BaseOpen(FILE *fp, bool do_close) {
      helper->Del();

      helper = new FileHelper();

      helper->fp = fp;
      helper->do_close = do_close;
    }

    inline void BaseSwitchWrite() const {
      if (helper->last_was_read) {
        BaseRelativePosition(0);
        helper->last_was_read = false;
      }
      helper->last_was_write = true;
    }

    inline void BaseSwitchRead() const {
      if (helper->last_was_write) {
        BaseRelativePosition(0);
        helper->last_was_write = false;
      }
      helper->last_was_read = true;
    }

    template<class T>
    inline void BaseWrite(const Data1D<T> &data,
                          const size_t amnt_to_write) const {
      size_t amnt_written;

      Assert();

      if (amnt_to_write == 0) {
        return;
      }
      data.Assert(amnt_to_write-1);

      BaseSwitchWrite();

      amnt_written = fwrite(data.Raw(), data.TypeSize(), amnt_to_write,
                            helper->fp);

      if (amnt_written < amnt_to_write) {
        if (ferror(helper->fp) != 0) {
          Throw_RC_Type(File, "File Write Error");
        }
      }
    }

    inline void BaseRelativePosition(const i64 amnt) const {
      if (fseek(helper->fp, amnt, SEEK_CUR) < 0) {
        if (amnt != 0) {
          Throw_RC_Type(File, "File Seek Error");
        }
      }
    }

    inline void BufFlush() {
      if (helper->IsWriteBuf()) {
        BaseWrite(helper->buf, helper->index);
        helper->index = 0;
      }
    }


    FileHelper *helper;
    static const size_t min_buf_size = 65536;

    /// @endcond

    public:

    /// Default constructor.
    inline FileBase() {
      helper = new FileHelper();
    }

    /// Copy constructor.
    inline FileBase(const FileBase& other) {
      other.helper->Add();
      helper = other.helper;
    }

    /// Assignment operator.
    inline FileBase& operator= (const FileBase& other) {
      helper->Del();

      other.helper->Add();
      helper = other.helper;

      return *this;
    }

    /// Flushes the write buffer before destructing.  This will close the file
    /// if it is the last FileBase sharing it.
    inline virtual ~FileBase() {
      try{
        Flush();
      }
      catch(...) {}
      helper->Del();
    }


    /// Returns the filename if one was given upon opening.
    inline RStr GetFilename() const {
      return (helper->filename);
    }


    /// Manually changes the associated filename.
    inline void SetFilename(const RStr& newfilename) const {
      helper->filename = newfilename;
    }


    /// Flushes the buffers and closes the file.
    inline void Close() {
      Flush();
      helper->Close();
    }


    /// True if the file is open.
    inline bool IsOpen() const {
      return (helper->fp != NULL);
    }

    /// True if the file is closed.
    inline bool IsClosed() const {
      return (helper->fp == NULL);
    }

    /// True if the file is open and readable.
    inline bool IsReadable() const {
      return (IsOpen() && (helper->is_readable));
    }

    /// True if the file is open and writable.
    inline bool IsWritable() const {
     return (IsOpen() && (helper->is_writable));
    }


    /// Returns a raw FILE* which can be used with the C file functions.
    inline FILE* Raw() const {
      return (helper->fp);
    }


    /// Throws ErrorMsgFile if the file is closed.
    inline void Assert() const {
      if (IsClosed()) {
        Throw_RC_Type(File, "File Is Closed");
      }
    }


    /// Returns the file size in bytes.
    inline size_t Size() const {
      struct stat filestats;

      Assert();

      if (fstat(fileno(helper->fp), &filestats) < 0) {
        Throw_RC_Type(File, "File Size Error");
      }

      return (filestats.st_size);
    }


    /// Sets the reading and writing position to pos bytes into the file.
    inline void SetPosition(const size_t pos) {
      Assert();
      ClearBuffer();

      if (fseek(helper->fp, pos, SEEK_SET) < 0) {
        Throw_RC_Type(File, "File Seek Error");
      }
    }


    /// Sets the position for the next read or write operation.
    /** Note:  This has no effect on write operations in APPEND mode.
     */
    inline void RelativePosition(const i64 amnt) {
      Assert();
      ClearBuffer();

      BaseRelativePosition(amnt);
    }

    /// Gets the position for the next read or write operation.
    /** @param quiet_fail If true, does not throw an error from being
     *  unable to find the position.  (0 is returned.)
     */
    inline size_t GetPosition(bool quiet_fail=false) const {
      Assert();
      long pos;

      if ( (pos = ftell(helper->fp)) < 0) {
        if (quiet_fail) { return 0; }
        Throw_RC_Type(File, "File Position Error");
      }

      return size_t(pos) + helper->index - helper->amnt_read;
    }

    /// Clears the read-ahead buffer used by FileRead::Get calls.
    inline void Rewind() {
      if (helper->IsReadBuf()) {
        BaseRelativePosition(i64(helper->index)-i64(helper->amnt_read));
        helper->amnt_read = 0;
        helper->index = 0;
      }
    }

    /// Flushes all unsaved data to the storage system.
    inline void Flush() {
      if (helper->is_writable) {
        BufFlush();
        if (fflush(helper->fp) != 0) {
          Throw_RC_Type(File, "File Flush Error");
        }
      }
    }

    /// Processes the remaining data in the Put/Get buffer.
    inline void ClearBuffer() {
      Rewind();
      BufFlush();
    }
  };


  /// A file reading class that provides buffered and unbuffered access to
  /// files with support for non-POD classes.
  class FileRead : public virtual FileBase {
    protected:
    /// @cond PROTECTED

    inline void FillBuff(size_t min_requested=0) {
      if (helper->buf.size() < min_requested) {
        if (min_requested < min_buf_size) {
          helper->buf.Resize(min_buf_size);
        }
        else {
          helper->buf.Resize(min_requested);
        }
      }
      if (helper->amnt_read > helper->index) {
        std::copy(helper->buf.Raw()+helper->index,
                  helper->buf.Raw()+helper->amnt_read,
                  helper->buf.Raw());
        helper->buf.SetOffset(helper->amnt_read - helper->index);
        helper->amnt_read += Read(helper->buf) - helper->index;
        helper->buf.SetOffset(0);
      }
      else {
        helper->amnt_read = Read(helper->buf);
      }
      helper->index = 0;
    }

    /// @endcond
    public:

    /// Default constructor.
    inline FileRead() {
    }

    /// Opens the file specified by filename for reading.
    /** Throws ErrorMsgFile if the file could not be opened.
     */
    inline FileRead(const RStr& filename) {
      if ( ! Open(filename)) {
        Throw_RC_Type(File, "File Open Error");
      }
    }

    /// Wraps the FILE* fp for reading.  It will close it when finished if
    /// do_close is true.
    inline FileRead(FILE *fp, bool do_close) {
      Open(fp, do_close);
    }

    /// Wraps the FILE* fp for reading.  It will close it when finished
    /// unless fp is stdin.
    inline FileRead(FILE *fp) {
      Open(fp);
    }

    /// Base copy constructor.
    inline FileRead(const FileBase& other) : FileBase(other) { }


    /// Opens the file specified by filename for reading.
    /** @return True if the file was successfully opened.
     */
    inline bool Open(const RStr& filename) {
      return BaseOpen(filename, R_ONLY);
    }

    /// Wraps the FILE* fp for reading.  It will close it when finished if
    /// do_close is true.
    inline void Open(FILE *fp, bool do_close) {
      BaseOpen(fp, do_close);
      helper->is_readable = true;
    }

    /// Wraps the FILE* fp for reading.  It will close it when finished
    /// unless fp is stdin.
    inline void Open(FILE *fp) {
      Open(fp, (fp != stdin));
    }


    /// Reads amnt_to_read elements of plain old data type T into data without
    /// buffering, resizing if too small.
    /** Throws ErrorMsgFile if there was an error reading.
     *  @return The number of elements successfully read.
     */
    template<class T>
    inline size_t Read(Data1D<T> &data, const size_t amnt_to_read) {
#ifdef CPP11
      // TODO - Uncomment after correct implementation broadly available.
//      static_assert(std::is_trivially_copyable<T>::value,
//          "Use only on trivially copyable types");
#endif
      size_t amnt_read;

      Assert();
      BaseSwitchRead();
      Rewind();

      if (data.size() < amnt_to_read) {
        data.Resize(amnt_to_read);
      }

      amnt_read = fread(data.Raw(), data.TypeSize(), amnt_to_read, helper->fp);

      if (amnt_read == 0) {
        if (ferror(helper->fp) != 0) {
          Throw_RC_Type(File, "File Read Error");
        }
      }

      return amnt_read;
    }


    /// Fills data with plain old data type T without buffering.
    /** Throws ErrorMsgFile if there was an error reading.
     *  @return The number of elements successfully read.
     */
    template<class T>
    inline size_t Read(Data1D<T> &data) {
      return Read(data, data.size());
    }


    /// Reads all data until the end of file into data as plain old data type
    /// T.
    template<class T>
    inline void ReadAll(Data1D<T> &data) {
#ifdef CPP11
      // TODO - Uncomment after correct implementation broadly available.
//      static_assert(std::is_trivially_copyable<T>::value,
//          "Use only on trivially copyable types");
#endif
      size_t amnt_read;
      size_t amnt_to_read = 1+65535/sizeof(T);

      size_t estimate = (Size()-GetPosition(true)) / sizeof(T);

      // If we can determine the filesize, read that.
      // Otherwise, read until EOF.
      if (estimate > 0) {
        data.Resize(estimate);
        data.Resize(Read(data));
      }
      else {
        size_t initial_offset = data.GetOffset();
        estimate = initial_offset + amnt_to_read;
        data.Reserve(estimate);

        size_t total_read = 0;
        while (1) {
          data.Resize(amnt_to_read);
          amnt_read = Read(data);
          if (amnt_read == 0) {
            break;
          }
          total_read += amnt_read;
          data.SetOffset(initial_offset + total_read);
          if ((data.reserved() - data.GetOffset()) < amnt_to_read) {
            estimate *= 2;
            data.Reserve(estimate);
          }
        }
        data.SetOffset(initial_offset);
        data.Resize(total_read);
      }
    }


    /// Reads one line until newline, null, or end of file.  The newline
    /// is removed if crop_newline is true.
    /** Throws ErrorMsgFile if there was a read error.
     *  @return True if the read was successful.
     */
    inline bool ReadLine(RStr &line, bool crop_newline = true) {
      Data1D<char> str(4096);
      char *retv;
      size_t amnt_read;
      bool readmore;
      size_t partsread;

      Assert();
      Rewind();
      BaseSwitchRead();

      line = "";

      readmore = true;
      for (partsread=0; readmore; partsread++) {
        retv = fgets(str.Raw(), str.size(), helper->fp);
        if (retv == NULL) {
          if (ferror(helper->fp) != 0) {
            Throw_RC_Type(File, "File Read Error");
          }
          else { // No error, but no more data.
            if (partsread == 0) {
              return false;
            }
            else {
              return true;
            }
          }
        }


        amnt_read = strlen(str.Raw());
        if ( (amnt_read != (str.size()-1)) || (str[amnt_read-1] == '\n') ) {
          readmore = false;
        }
        if (crop_newline) {
          while (amnt_read >= 1) {
            if ( (str[amnt_read-1] == '\n') || (str[amnt_read-1] == '\r') ) {
              str[amnt_read-1] = 0;
              amnt_read--;
            }
            else {
              break;
            }
          }
        }

        line += str.Raw();

      }

      return true;
    }

    /// Does ReadLine.
    inline bool Read(RStr &line, bool crop_newline = true) {
      return ReadLine(line, crop_newline);
    }


    /// Discards one line until newline, null, or end of file.
    inline bool SkipLine() {
      RStr discard;
      return ReadLine(discard);
    }


    /// Reads all the lines found until the end of the file.  If crop_newlines
    /// is true they are removed from each line.
    inline void ReadAllLines(Data1D<RStr> &lines, bool crop_newlines = true) {
      RStr str;
      size_t estimate;
      estimate = (Size()-GetPosition(true)) / 30;  // Estimate #lines in file.
      lines.Delete();
      lines.Reserve(estimate);

      while (ReadLine(str, crop_newlines)) {
        lines.Append(str);
      }
    }

    /// Does ReadAllLines.
    inline void ReadAll(Data1D<RStr> &lines, bool crop_newlines = true) {
      ReadAllLines(lines, crop_newlines);
    }


    /// Performs a buffered read of sizeof(T) bytes and assigns them to data.
    /** @return True if the read succeeded.
     */
    template<class T>
    inline bool RawGet(T& data) {
#ifdef CPP11
      // TODO - Uncomment after correct implementation broadly available.
//      static_assert(std::is_trivially_copyable<T>::value,
//          "Implement a FileGetWrapper for types with copy/move constructors");
#endif
      if (helper->IsWriteBuf()) {
        BufFlush();
      }
      if ( (helper->amnt_read - helper->index) < sizeof(T) ) {
        FillBuff(sizeof(T));
        if (helper->amnt_read < sizeof(T)) {
          return false;
        }
      }

      data = *reinterpret_cast<T*>(helper->buf.Raw()+helper->index);
      helper->index += sizeof(T);

      return true;
    }

    /// Gets data of type T, after passing through FileGetWrapper.
    /** @return True if the read succeeded.
     */
    template<class T>
    inline bool Get(T& data) { return FileGetWrapper(*this, data); }

    /// Gets one line into data, up to the newline, null, or end of file,
    /// removing the newline if crop_newline is true.
    /** @return True if the read succeeded.
     */
    inline bool Get(RStr& data, bool crop_newline=true) {
      Data1D<char> arr;
      arr.Reserve(4096);
      char ch;
      bool retval = false;

      while (RawGet(ch)) {
        retval = true;
        if (ch == '\0') {
          break;
        }
        if (ch == '\n') {
          if ( ! crop_newline ) {
            arr += ch;
          }
          break;
        }
        arr += ch;
      }

      if (crop_newline) {
        while (arr.size() > 0 && arr[arr.size()-1] == '\r') {
          arr.Resize(arr.size()-1);
        }
      }

      arr += '\0';
      data = arr.Raw();

      return retval;
    }

    /// Fill data with lines from the file, removing the newlines if
    /// crop_newlines is true.
    /** @return True if the reads succeeded.
     */
    inline bool Get(Data1D<RStr>& data, bool crop_newlines=true) {
      bool retval = false;
      for (size_t i=0; i<data.size(); i++) {
        retval = Get(data[i], crop_newlines);
        if (!retval) {
          return false;
        }
      }
      return retval;
    }

    /// Fill data with elements of type T, calling Get on each one.
    /** @return True if the reads succeeded.
     */
    template<class T>
    inline bool Get(Data1D<T>& data) {
      bool retval = false;
      for (size_t i=0; i<data.size(); i++) {
        retval = Get(data[i]);
        if (!retval) {
          return false;
        }
      }
      return retval;
    }

    /// Fill data with elements of type T, calling Get on each one.
    /** For data[y][x], x is the inner loop.
     *  @return True if the reads succeeded.
     */
    template<class T>
    inline bool Get(Data2D<T>& data) {
      return Get(data.RawData());
    }

    /// Fill data with elements of type T, calling Get on each one.
    /** For data[z][y][x], x is the inner loop.
     *  @return True if the reads succeeded.
     */
    template<class T>
    inline bool Get(Data3D<T>& data) {
      return Get(data.RawData());
    }

    /// Fill data with all lines from the file until the end, removing the
    /// newlines if crop_newlines is true.
    /** @return True if the reads succeeded.
     */
    inline bool GetAll(Data1D<RStr>& data, bool crop_newlines=true) {
      bool retval = false;
      data.Resize(0);
      RStr tmp;
      while(Get(tmp, crop_newlines)) {
        retval = true;
        data += tmp;
      }
      return retval;
    }

    /// Fill data with all elements of type T until the end of file, calling
    /// Get on each one.
    /** @return True if the reads succeeded.
     */
    template<class T>
    inline bool GetAll(Data1D<T>& data) {
      bool retval = false;
      data.Resize(0);
      T tmp;
      while(Get(tmp)) {
        retval = true;
        data += tmp;
      }
      return retval;
    }
  };


  /// A file writing class that provides buffered and unbuffered output to
  /// files with support for non-POD classes.
  class FileWrite : public virtual FileBase {
    public:

    /// Default constructor.
    inline FileWrite() {
    }

    /// Opens the file specified by filename for writing, using the WriteMode
    /// specified by mode.
    /** Throws ErrorMsgFile if the file could not be opened.
     */
    inline FileWrite(const RStr& filename,
                     const WriteMode mode=TRUNCATE) {
      if ( ! Open(filename, mode) ) {
        if (errno == EEXIST) {
          Throw_RC_Type(File, "File Exists");
        }
        else {
          Throw_RC_Type(File, "File Open Error");
        }
      }
    }

    /// Wraps the FILE* fp for writing.  It will close it when finished if
    /// do_close is true.
    inline FileWrite(FILE *fp, bool do_close) {
      Open(fp, do_close);
    }

    /// Wraps the FILE* fp for writing.  It will close it when finished
    /// unless fp is stdout.
    inline FileWrite(FILE *fp) {
      Open(fp);
    }

    /// Base copy constructor.
    inline FileWrite(const FileBase& other) : FileBase(other) { }


    /// Opens the file specified by filename for writing, using the WriteMode
    /// specified by mode.
    /** @return True if the file was successfully opened.
     */
    inline bool Open(const RStr& filename, const WriteMode mode=TRUNCATE) {
      return BaseOpen(filename, BaseMode(int(mode)+1));
    }

    /// Wraps the FILE* fp for writing.  It will close it when finished if
    /// do_close is true.
    inline void Open(FILE *fp, bool do_close) {
      BaseOpen(fp, do_close);
      helper->is_writable = true;
    }

    /// Wraps the FILE* fp for writing.  It will close it when finished
    /// unless fp is stdout or stderr.
    inline void Open(FILE *fp) {
      Open(fp, (fp != stdout) && (fp != stderr));
    }


    /// Writes amnt_to_write elements of plain old data type T from data
    /// without buffering.
    /** Throws ErrorMsgFile if there was an error writing.
     */
    template<class T>
    inline void Write(const Data1D<T> &data, const size_t amnt_to_write) {
      BufFlush();
      BaseWrite(data, amnt_to_write);
    }

    /// Writes all of data's plain old data type T without buffering.
    /** Throws ErrorMsgFile if there was an error writing.
     */
    template<class T>
    inline void Write(const Data1D<T> &data) {
      Write(data, data.size());
    }


    /// Writes the null-terminated contents of str without buffering.
    /** Throws ErrorMsgFile if there was a write error.
     */
    inline void WriteStr(const char *str) {
      Assert();
      BufFlush();

      BaseSwitchWrite();
      if (fputs(str, helper->fp) < 0) {
        if (ferror(helper->fp) != 0) {
          Throw_RC_Type(File, "File Write Error");
        }
      }
    }


    /// Writes the RStr str without buffering.
    /** Throws ErrorMsgFile if there was a write error.
     */
    inline void WriteStr(const RStr &str) {
      Assert();

      WriteStr(str.c_str());
    }


    /// Writes all lines without buffering, adding a newline after each if
    /// add_newlines is true.
    /** Throws ErrorMsgFile if there was a write error.
     */
    inline void WriteAllStr(const Data1D<RStr> &lines,
                            const bool add_newlines) {
      for (size_t i=0; i<lines.size(); i++) {
        WriteStr(lines[i]);
        if (add_newlines) {
          if (fputc('\n', helper->fp) == EOF) {
            Throw_RC_Type(File, "File Write Error");
          }
        }
      }
    }


    /// Performs a buffered write of sizeof(T) raw bytes extracted from data.
    template<class T>
    inline void RawPut(const T& data) {
#ifdef CPP11
      // TODO - Uncomment after correct implementation broadly available.
//      static_assert(std::is_trivially_copyable<T>::value,
//          "Implement a FilePutWrapper for types with copy/move constructors");
#endif
      if (helper->IsReadBuf()) {
        Rewind();
      }
      if ( (helper->buf.size() - helper->index) < sizeof(T) ) {
        BufFlush();
        if (helper->buf.size() < sizeof(T)) {
          if (sizeof(T) < min_buf_size) {
            helper->buf.Resize(min_buf_size);
          }
          else {
            helper->buf.Resize(sizeof(T));
          }
        }
      }

      memcpy(helper->buf.Raw()+helper->index, &data, sizeof(T));
      helper->index += sizeof(T);
    }

    /// Puts data of type T into the write buffer, after passing through
    /// FilePutWrapper.
    template<class T>
    inline void Put(const T& data) { FilePutWrapper(*this, data); }

    /// Puts the null-terminated character data str into the write buffer,
    /// excluding the null.
    inline void Put(const char* str) {
      const char* tmp = str;
      while (*tmp) {
        Put(*tmp);
        tmp++;
      }
    }

    /// Puts the RStr str into the write buffer.
    inline void Put(const RStr& str) {
      Put(str.c_str());
    }

    /// Calls Put for each element of data.
    template<class T>
    inline void Put(const Data1D<T>& data) {
      for (size_t i=0; i<data.size(); i++) {
        Put(data[i]);
      }
    }

    /// Calls Put for each element of data.
    /** For data[y][x], x is the inner loop.
     */
    template<class T>
    inline void Put(const Data2D<T>& data) {
      Put(data.RawData());
    }

    /// Calls Put for each element of data.
    /** For data[z][y][x], x is the inner loop.
     */
    template<class T>
    inline void Put(const Data3D<T>& data) {
      Put(data.RawData());
    }

    /// Puts each RStr in data into the write buffer, adding newlines to each
    /// if add_newline is true.
    inline void Put(const Data1D<RStr>& data, const bool add_newline) {
      for (size_t i=0; i<data.size(); i++) {
        Put(data[i]);
        if (add_newline) {
          Put('\n');
        }
      }
    }


    /// Get all the remaining data from the FileRead, and Put it to this
    /// FileWrite.
    inline FileWrite& operator<< (FileRead& read) {
#ifdef CPP11
      struct { u64 a, b, c, d; } tmp;
#else
      u64 tmp;
#endif
      while (read.Get(tmp)) {
        Put(tmp);
      }
      char ch;
      while (read.Get(ch)) {
        Put(ch);
      }

      return (*this);
    }
  };


  /// A file class for both reading and writing that provides buffered and
  /// unbuffered output to files.
  class FileRW : public FileRead, public FileWrite {
    public:

    /// Default constructor.
    inline FileRW() {
    }

    /// Opens the file specified by filename for reading and writing, using
    /// the WriteMode specified by mode.
    /** Throws ErrorMsgFile if the file could not be opened.
     */
    inline FileRW(const RStr& filename,
                  const WriteMode mode=KEEP) {
      if ( ! Open(filename, mode) ) {
        if (errno == EEXIST) {
          Throw_RC_Type(File, "File Exists");
        }
        else {
          Throw_RC_Type(File, "File Open Error");
        }
      }
    }

    /// Wraps the FILE* fp for reading and writing.  It will close it when
    /// finished if do_close is true.
    inline FileRW(FILE *fp, bool do_close) {
      Open(fp, do_close);
    }

    /// Wraps the FILE* fp for reading and writing.  It will close it when
    /// finished unless fp is stdin/stdout/stderr.
    inline FileRW(FILE *fp) {
      Open(fp);
    }

    /// Base copy constructor.
    inline FileRW(const FileBase& other) : FileBase(other) { }


    /// Opens the file specified by filename for reading and writing, using
    /// the WriteMode specified by mode.
    /** @return True if the file was successfully opened.
     */
    inline bool Open(const RStr& filename, const WriteMode mode=KEEP) {
      return BaseOpen(filename, BaseMode(int(mode)+5));
    }

    /// Wraps the FILE* fp for reading and writing.  It will close it when
    /// finished if do_close is true.
    inline void Open(FILE *fp, bool do_close) {
      BaseOpen(fp, do_close);
      helper->is_readable = true;
      helper->is_writable = true;
    }

    /// Wraps the FILE* fp for reading and writing.  It will close it when
    /// finished unless fp is stdin/stdout/stderr.
    inline void Open(FILE *fp) {
      Open(fp, (fp != stdin) && (fp != stdout) && (fp != stderr));
    }
  };


  /// The default FileGetWrapper for plain old data, which calls RawGet.
  template<class T> inline bool FileGetWrapper(FileRead& fr, T& data) {
    return fr.RawGet(data);
  }

  /// The default FilePutWrapper for plain old data, which calls RawPut.
  template<class T> inline void FilePutWrapper(FileWrite& fw, const T& data) {
    fw.RawPut(data);
  }

  /// A specialization for handling std::string objects.
  template<> inline bool FileGetWrapper<std::string>(FileRead& fr,
      std::string& data) {
    RStr tmp;
    bool retval = fr.Get(tmp);
    data = tmp.Raw();
    return retval;
  }

  /// A specialization for handling std::string objects.
  template<> inline void FilePutWrapper<std::string>(FileWrite& fw,
      const std::string& data) {
    fw.Put(RStr(data));
  }


  /// A class with static methods for file and directory info and manipulation.
  class File {
    public:

    /// The OS-specific divider between directories in a pathname. 
#ifdef WIN32
    static const char divider = '\\';
#else
    static const char divider = '/';
#endif

    /// Returns the file size of pathname, or 0 if the file does not exist.
    static inline size_t Size(const RStr& pathname) {
      struct stat filestats;

      if (stat(pathname.c_str(), &filestats) < 0) {
        if (errno == ENOENT) {
          return 0;  // File does not exist
        }
        else {
          Throw_RC_Type(File, "File Size Error");
        }
      }

      return (filestats.st_size);
    }


    /// Returns true if the file pathname exists.
    static inline bool Exists(const RStr& pathname) {
      struct stat filestats;

      if (stat(pathname.c_str(), &filestats) < 0) {
        return false;
      }
      else {
        return true;
      }
    }


    /// Deletes the file pathname, returning true if it succeeded.
    /** If quiet_fail is false, an exception is thrown upon failure. */
    static inline bool Delete(const RStr& pathname, bool quiet_fail=true) {
      if (remove(pathname.c_str()) != 0) {
        if ( ! quiet_fail ) {
          Throw_RC_Type(File, "File Delete Error");
        }
        return false;
      }
      return true;
    }


    /// Copies the contents of srcfile to destfile.  It will overwrite an
    /// existing file only if overwrite is true.
    /** Throws ErrorMsgFile if an error occurs. */
    static inline void Copy(const RStr& srcfile, const RStr& destfile,
                            bool overwrite=true) {
      FileRead in(srcfile);

      WriteMode mode = overwrite ? TRUNCATE : NEWONLY;
      FileWrite out(destfile, mode);

      out << in;
    }


    /// Moves srcfile to destfile by copying the contents and then deleting
    /// srcfile.
    /** Note:  This first attempts an efficient rename, but if that fails
     *  it falls back to a copy/delete operation.
     *  ErrorMsgFile is thrown if the move fails.
     */
    static inline void Move(const RStr& srcfile, const RStr& destfile,
                            bool overwrite=true) {
      int ren_ret;
      if (srcfile == destfile) { return; }

#ifdef WIN32
      ren_ret = rename(srcfile.c_str(), destfile.c_str());
      if (overwrite && ren_ret != 0 && errno == EACCES) {
        if (Delete(destfile)) {
          ren_ret = rename(srcfile.c_str(), destfile.c_str());
        }
      }
#else
      if (overwrite) {
        ren_ret = rename(srcfile.c_str(), destfile.c_str());
      }
      else {
        ren_ret = link(srcfile.c_str(), destfile.c_str());
        if (ren_ret == 0) {
          unlink(srcfile.c_str());
        }
      }
#endif

      if (ren_ret != 0) {
        Copy(srcfile, destfile, overwrite);
        Delete(srcfile);
      }
    }


    /// Makes a new directory dirname.
    /** @return True if it succeeded.
     *  If return_true_if_exists is true, success is if the directory exists
     *  upon return.  If it's false, success is if the directory was newly
     *  created.
     */
    static inline bool MakeDir(const RStr& dirname,
                               bool return_true_if_exists=true) {
#ifdef WIN32
      RStr tmpdir = dirname;
      if (CreateDirectory(tmpdir.ToLPCTSTR(), NULL)==0) {
        if (return_true_if_exists &&
            (GetLastError() == ERROR_ALREADY_EXISTS)) {
          return true;
        }
        return false;
      }
#else
      if (mkdir(dirname.c_str(), 0777)) {
        if (return_true_if_exists && (errno == EEXIST)) {
          return true;
        }
        return false;
      }
#endif
      return true;
    }


    /// Returns the current working directory.
    static inline RStr CurrentDir() {
      char cwd[65536];
      char *errchk;
      RStr retval;
#ifdef WIN32
      errchk = _getcwd(cwd, 65536);
#else
      errchk = getcwd(cwd, 65536);
#endif
      if (errchk != NULL) {
        retval = cwd;
      }
      return retval;
    }


    /// Returns a list of entries in the directory path.
    /** If qualified is true, the returned entries are fully qualified
     *  pathnames.  Throws ErrorMsgFile if the directory cannot be read.
     */
    static inline Data1D<RStr> DirList(const RStr &path,
                                       bool qualified=false) {
      Data1D<RStr> dir_list;
      const char err_msg[] = "Could not read directory";

#ifdef WIN32
      RStr path_appended = path + "\\*";
      WIN32_FIND_DATA file_data;
      HANDLE dir_handle;

      dir_handle = FindFirstFile(path_appended.ToLPCTSTR(), &file_data);

      if (dir_handle == INVALID_HANDLE_VALUE) {
        Throw_RC_Type(File, err_msg);
      }

      do {
        if (qualified) {
          dir_list += path + File::divider + RStr(file_data.cFileName);
        }
        else {
          dir_list += RStr(file_data.cFileName);
        }
      } while (FindNextFile(dir_handle, &file_data));
#else
      struct dirent **namelist;

      int entries = scandir(path.c_str(), &namelist, NULL, NULL);
      if (entries < 0) {
        Throw_RC_Type(File, err_msg);
      }

      for (int i=0; i<entries; i++) {
        RStr entry = RStr(namelist[i]->d_name);
        if (entry != "." && entry != "..") {
          if (qualified) {
            dir_list += path + File::divider + entry;
          }
          else {
            dir_list += entry;
          }
        }
        free(namelist[i]);
      }
      free(namelist);
#endif

      dir_list.Sort();

      return dir_list;
    }

    /// Extracts the directory portion of filename, or the current directory
    /// if filename has no directory.
    static inline RStr Dirname(const RStr& filename) {
      size_t mid;
      mid = filename.find_last_of("\\/");
      if (mid == RStr::npos) {
        return CurrentDir();
      }
      else {
        if (mid < filename.length()) {
          mid++;
        }
        return filename.substr(0, mid);
      }
    }


    /// Extracts the basename portion of filename, which excludes the
    /// directory.
    static inline RStr Basename(const RStr& filename) {
      Data1D<RStr> split = filename.SplitLast("\\/");
      if (split[1].length() == 0) {
        return filename;
      }
      else {
        return split[1];
      }
    }


    /// Merges path and filename into a path with a divider inserted if
    /// needed.
    static inline RStr FullPath(const RStr& path, const RStr& filename) {
      if (path.size()>0 && path[path.size()-1]==divider) {
        return path + filename;
      }
      else {
        return path + divider + filename;
      }
    }


    /// Extracts the filename's extension.
    static inline RStr Extension(const RStr& filename) {
      if (filename.size() == 0) { return ""; }
      for (size_t i=filename.size()-1; i<filename.size(); i--) {
        if (filename[i] == '.') {
          return filename.substr(i+1);
        }
        if (filename[i] == '\\' || filename[i] == '/') {
          break;
        }
      }
      return "";
    }


    /// Extracts the filename without the extension.
    static inline RStr NoExtension(const RStr& filename) {
      if (filename.size() == 0) { return ""; }
      for (size_t i=filename.size()-1; i<filename.size(); i--) {
        if (filename[i] == '.') {
          return filename.substr(0,i);
        }
        if (filename[i] == '\\' || filename[i] == '/') {
          break;
        }
      }
      return filename;
    }
  };
}


#endif // RC_FILE_H

