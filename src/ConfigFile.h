#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "nlohmann/json.hpp"
#include "Utils.h"
#include "ValIter.h"
#include "RC/RC.h"
#include <type_traits>


namespace CML {
  // Removes # and // comments, left/right whitespace, and blank lines.
  void RemoveComments(RC::Data1D<RC::RStr>& lines);

  // Remove UTF-8 byte order marks if present. (There's no byte order to mark!)
  void RemoveBOM(RC::RStr& line);

  class JSONFile {
    public:
    JSONFile() {
      rev_ptr = this;
      rev_ptr.AutoRevoke();
    }

    JSONFile(RC::RStr pathname) {
      Load(pathname);
      rev_ptr = this;
      rev_ptr.AutoRevoke();
    }

    void Load(RC::RStr pathname) {
      filename = pathname;
      RC::FileRead fr;
      if (!fr.Open(pathname)) {
        Throw_RC_Type(File, ("Could not open " + pathname).c_str());
      }
      Load(fr);
    }

    void Load(RC::FileRead& fr) {
      RC::Data1D<RC::RStr> lines;
      fr.ReadAllLines(lines);
      if (lines.size() >= 1) {
        RemoveBOM(lines[0]);
      }
      RemoveComments(lines);
      json = json.parse(RC::RStr::Join(lines, "\n").c_str());
    }

    void Parse(RC::RStr json_text) {
      json = nlohmann::json::parse(json_text.c_str());
    }

    void SetFilename(RC::RStr new_filename) {
      filename = new_filename;
    }

    RC::RStr Line() const {
      return RC::RStr::Join(RC::RStr(json.dump(0)).Split('\n'))+"\n";
    }

    void Save(RC::RStr pathname) const {
      RC::FileWrite fw(pathname);
      Save(fw);
    }

    void Save(RC::FileWrite& fw) const {
      fw.WriteStr(json.dump(2));
    }

    template<class T, class... Keys>
    void Get(T& data, Keys... keys) const {
      GetRecurse(data, RC::RStr("Could not get ")+filename+" key, ",
          json, keys...);
    }

    template<class... Keys>
    void Get(RC::RStr& data, Keys... keys) const {
      Get(data.Raw(), keys...);
    }

    template<class... Keys>
    void Get(JSONFile& data, Keys... keys) const {
      Get(data.json, keys...);
      data.filename = filename + ((RC::RStr(":") + RC::RStr(keys)) + ...);
    }

    template<class T, class... Keys>
    void Get(RC::Data1D<RC::Data1D<T>>& data, Keys... keys) const {
      std::vector<std::vector<T>> v;
      Get(v, keys...);
      data.Resize(v.size());
      for (size_t i=0; i<v.size(); i++) {
        data[i].Resize(v[i].size());
        for (size_t j=0; j<v[i].size(); j++) {
          data[i][j] = v[i][j];
        }
      }
    }

    template<class T, class... Keys>
    void Get(RC::Data1D<T>& data, Keys... keys) const {
      std::vector<T> v;
      Get(v, keys...);
      data.Resize(v.size());
      for (size_t i=0; i<v.size(); i++) {
        data[i] = v[i];
      }
    }

    template<class... Keys>
    void Get(RC::Data1D<RC::RStr>& data, Keys... keys) const {
      std::vector<std::string> v;
      Get(v, keys...);
      data.Resize(v.size());
      for (size_t i=0; i<v.size(); i++) {
        data[i] = v[i];
      }
    }

    // Handle Linux and Windows path dividers portably on both systems, and
    // substitute the supported $HOME and $DESKTOP special config variables.
    template<class... Keys>
    RC::RStr GetPath(Keys... keys) const {
      RC::RStr path;
      Get(path, keys...);
      path.Subst("^~\\\\",
          GetHomeDir()+RC::File::divider);
      path.Subst("^~/",
          GetHomeDir()+RC::File::divider);
      path.Subst("\\$HOME", GetHomeDir());
      path.Subst("\\$DESKTOP", GetDesktop());
#ifdef WIN32
      path.Subst("/", RC::File::divider);
#else
      path.Subst("\\\\", RC::File::divider);
#endif
      return path;
    }

    // Returns true if succeeded
    template<class T, class... Keys>
    bool TryGet(T& data, Keys... keys) const {
      return TryGetRecurse(data, &json, keys...);
    }

    template<class... Keys>
    bool TryGet(RC::RStr& data, Keys... keys) const {
      return TryGet(data.Raw(), keys...);
    }

    template<class... Keys>
    bool TryGet(JSONFile& data, Keys... keys) const {
      bool retval = Get(data.json, keys...);
      data.filename = filename + ((RC::RStr(":") + RC::RStr(keys)) + ...);
      return retval;
    }

    template<class T, class... Keys>
    bool TryGet(RC::Data1D<T>& data, Keys... keys) const {
      std::vector<T> v;
      bool retval = TryGet(v, keys...);
      if (retval) {
        data.Resize(v.size());
        for (size_t i=0; i<v.size(); i++) {
          data[i] = v[i];
        }
      }
      return retval;
    }

    template<class T, class... Keys>
    bool TryGet(RC::Data1D<RC::Data1D<T>>& data, Keys... keys) const {
      std::vector<std::vector<T>> v;
      bool retval = TryGet(v, keys...);
      if (retval) {
        data.Resize(v.size());
        for (size_t i=0; i<v.size(); i++) {
          data[i].Resize(v[i].size());
          for (size_t j=0; j<v[i].size(); j++) {
            data[i][j] = v[i][j];
          }
        }
      }
      return retval;
    }

    template<class... Keys>
    bool TryGet(RC::Data1D<RC::RStr>& data, Keys... keys) const {
      std::vector<std::string> v;
      bool retval = TryGet(v, keys...);
      if (retval) {
        data.Resize(v.size());
        for (size_t i=0; i<v.size(); i++) {
          data[i] = v[i];
        }
      }
      return retval;
    }

    // Use this on base JSONFile for updates.
    template<class T, class... Keys>
    void Set(const T& data, Keys... keys) {
      SetRecurse(data, RC::RStr("Could not set ")+filename+" key, ",
          &json, keys...);
    }

    template<class... Keys>
    void Set(const RC::RStr& data, Keys... keys) {
      Set(data.Raw(), keys...);
    }

    template<class... Keys>
    void Set(const JSONFile& data, Keys... keys) {
      Set(data.json, keys...);
    }

    template<class T, class... Keys>
    void Set(const RC::Data1D<RC::Data1D<RC::Data1D<T>>>& data, Keys... keys) {
      std::vector<std::vector<std::vector<T>>> v(data.size());
      for (size_t i=0; i<v.size(); i++) {
        v[i].resize(data[i].size());
        for (size_t j=0; j<v[i].size(); j++) {
          v[i][j].resize(data[i][j].size());
          for (size_t k=0; k<v[i][j].size(); k++) {
            v[i][j][k] = data[i][j][k];
          }
        }
      }
      Set(v, keys...);
    }

    template<class T, class... Keys>
    void Set(const RC::Data1D<RC::Data1D<T>>& data, Keys... keys) {
      std::vector<std::vector<T>> v(data.size());
      for (size_t i=0; i<v.size(); i++) {
        v[i].resize(data[i].size());
        for (size_t j=0; j<v[i].size(); j++) {
          v[i][j] = data[i][j];
        }
      }
      Set(v, keys...);
    }

    template<class T, class... Keys>
    void Set(const RC::Data1D<T>& data, Keys... keys) {
      std::vector<T> v(data.size());
      for (size_t i=0; i<v.size(); i++) {
        v[i] = data[i];
      }
      Set(v, keys...);
    }

    template<class... Keys>
    auto Node(Keys... keys) const {
      return NodeRecurse(RC::RStr("Could not get ")+filename+" key, ",
          filename, json, keys...);
    }

    // Value copy
    JSONFile operator[](size_t i) const {
      RC::RStr label = filename + "[" + RC::RStr(i) +"]";
      try {
        if (i >= json.size()) {
          Throw_RC_Type(File, (label + " out of bounds").c_str());
        }
        JSONFile new_json;
        new_json.json = json[i];
        new_json.filename = label;
        return new_json;
      }
      catch (nlohmann::json::exception &e) {
        Throw_RC_Type(File, (label + " missing").c_str());
      }
    }

    size_t size() const { return json.size(); }

    auto begin() {
      return ValIter<JSONFile, JSONFile> (rev_ptr, 0);
    }
    auto begin() const {
      return ValIter<JSONFile, JSONFile> (rev_ptr, 0);
    }
    auto end() {
      return ValIter<JSONFile, JSONFile> (rev_ptr, size());
    }
    auto end() const {
      return ValIter<JSONFile, JSONFile> (rev_ptr, size());
    }


    nlohmann::json json;

    protected:

    template<class T, class JSONT, class... Keys>
    void GetRecurse(T& data, RC::RStr err_msg, JSONT& cur_node,
        __attribute__((unused)) Keys... keys) const {
      try {
        cur_node.get_to(data);
      }
      catch (nlohmann::json::exception &e) {
        Throw_RC_Type(File, (err_msg + "\n" + e.what()).c_str());
      }
    }

    template<class T, class JSONT, class Key, class... Keys,
      std::enable_if_t<!std::is_integral<Key>::value, bool> = true>
    void GetRecurse(T& data, RC::RStr err_msg, JSONT& cur_node,
        Key key, __attribute__((unused)) Keys... keys) const {
      err_msg += RC::RStr(key) + ":";
      auto itr = cur_node.find(key);
      if (itr == cur_node.end()) {
        Throw_RC_Type(File, (err_msg + "\nError found").c_str());
      }
      GetRecurse(data, err_msg, *itr, keys...);
    }

    template<class T, class JSONT, class Key, class... Keys,
      std::enable_if_t<std::is_integral<Key>::value, bool> = true>
    void GetRecurse(T& data, RC::RStr err_msg, JSONT& cur_node,
        Key key, __attribute__((unused)) Keys... keys) const {
      err_msg += RC::RStr(key) + ":";
      try {
        auto res = cur_node.at(key);
        GetRecurse(data, err_msg, res, keys...);
      }
      catch (nlohmann::json::exception &e) {
        Throw_RC_Type(File, (err_msg + "\nError found").c_str());
      }
    }

    template<class T, class JSONI, class... Keys>
    bool TryGetRecurse(T& data, JSONI itr, 
        __attribute__((unused)) Keys... keys) const {
      try {
        itr->get_to(data);
      }
      catch (nlohmann::json::exception &e) {
        return false;
      }
      return true;
    }

    template<class T, class JSONI, class Key, class... Keys>
    bool TryGetRecurse(T& data, JSONI itr, Key key, 
        __attribute__((unused)) Keys... keys) const {
      auto next_itr = itr->find(key);
      if (next_itr == itr->end()) {
        return false;
      }
      return TryGetRecurse(data, next_itr, keys...);
    }

    template<class T, class JSONI, class... Keys>
    void SetRecurse(const T& data, RC::RStr err_msg, JSONI itr,
        __attribute__((unused)) Keys... keys) {
      try {
        nlohmann::json update_node = data;
        *itr = update_node;
      }
      catch (nlohmann::json::exception &e) {
        Throw_RC_Type(File, (err_msg + "\n" + e.what()).c_str());
      }
    }

    template<class T, class JSONI, class... Keys>
    void SetRecurse(const T& data, RC::RStr err_msg, JSONI itr,
        size_t key, __attribute__((unused)) Keys... keys) {
      err_msg += RC::RStr(key) + ":";
      if (key >= itr->size()) {
        Throw_RC_Type(File, (err_msg + "\nOut of bounds").c_str());
      }
      auto next_itr = &((*itr)[key]);
      SetRecurse(data, err_msg, next_itr, keys...);
    }
    template<class T, class JSONI, class... Keys>
    void SetRecurse(const T& data, RC::RStr err_msg, JSONI itr,
        int32_t key, Keys... keys) {
      SetRecurse(data, err_msg, itr, size_t(key), keys...);
    }
    template<class T, class JSONI, class... Keys>
    void SetRecurse(const T& data, RC::RStr err_msg, JSONI itr,
        int64_t key, Keys... keys) {
      SetRecurse(data, err_msg, itr, size_t(key), keys...);
    }
    template<class T, class JSONI, class... Keys>
    void SetRecurse(const T& data, RC::RStr err_msg, JSONI itr,
        uint32_t key, Keys... keys) {
      SetRecurse(data, err_msg, itr, size_t(key), keys...);
    }

    template<class T, class JSONI, class Key, class... Keys>
    void SetRecurse(const T& data, RC::RStr err_msg, JSONI itr,
        Key key, Keys... keys) {
      err_msg += RC::RStr(key) + ":";
      itr->operator[](key);  // Create if missing.
      auto next_itr = itr->find(key);
      if (next_itr == itr->end()) {
        Throw_RC_Type(File, (err_msg + "\nError found").c_str());
      }
      SetRecurse(data, err_msg, next_itr, keys...);
    }

    template<class JSONT, class... Keys>
    auto NodeRecurse(RC::RStr err_msg, RC::RStr new_name, JSONT& cur_node,
        __attribute__((unused)) Keys... keys) const {
      JSONFile new_json;
      try {
        new_json.json = cur_node;
        new_json.filename = new_name;
      }
      catch (nlohmann::json::exception &e) {
        Throw_RC_Type(File, (err_msg + "\nError found").c_str());
      }
      return new_json;
    }

    template<class JSONT, class Key, class... Keys>
    auto NodeRecurse(RC::RStr err_msg, RC::RStr new_name, JSONT& cur_node,
        Key key, Keys... keys) const {
      err_msg += RC::RStr(key) + ":";
      new_name += ":" + RC::RStr(key);
      auto itr = cur_node.find(key);
      if (itr == cur_node.end()) {
        Throw_RC_Type(File, (err_msg + "\nError found").c_str());
      }
      return NodeRecurse(err_msg, new_name, *itr, keys...);
    }

    RC::RStr filename;
    RC::RevPtr<JSONFile> rev_ptr;
  };


  class CSVFile {
    public:
    CSVFile() {}

    CSVFile(RC::RStr pathname) {
      Load(pathname);
    }

    void Load(RC::RStr pathname) {
      filename = pathname;
      RC::FileRead fr;
      if (!fr.Open(pathname)) {
        Throw_RC_Type(File, ("Could not open " + pathname +
                             "\nError found").c_str());
      }
      RC::Data1D<RC::RStr> lines;
      fr.ReadAllLines(lines);
      if (lines.size() >= 1) {
        RemoveBOM(lines[0]);
      }
      file_lines = lines;
      RemoveComments(lines);

      data.Clear();
      if (lines.size() == 0) {
        return;
      }

      size_t xsize = lines[0].SplitCSV().size();
      data.Resize(xsize, lines.size());

      for (size_t i=0; i<lines.size(); i++) {
        auto row = lines[i].SplitCSV();
        if (row.size() != xsize) {
          Throw_RC_Type(File, (RC::RStr("CSV dimension mismatch in ") +
              filename + ", data line " + RC::RStr(i) + ", \"" + lines[i] +
              "\"\n").c_str());
        }
        data[i] = row;
      }
    }

    void Save(RC::RStr pathname) const {
      RC::FileWrite fw(pathname);
      fw.WriteStr(RC::RStr::MakeCSV(data));
    }

    RC::RStr GetFilename() const {
      return filename;
    }

    RC::Data2D<RC::RStr> data;
    RC::Data1D<RC::RStr> file_lines;

    protected:

    RC::RStr filename;
  };

  inline std::ostream& operator<< (std::ostream &out, const JSONFile &j) {
    out << j.json;
    return out;
  }

  inline std::ostream& operator<< (std::ostream &out, const CSVFile &c) {
    out << c.data;
    return out;
  }
}

#endif // CONFIGFILE_H

