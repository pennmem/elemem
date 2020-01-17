#ifndef EXPCONFIG_H
#define EXPCONFIG_H

#include <nlohmann/json.hpp>
#include <RC/RC.h>


namespace CML {
  void RemoveComments(RC::Data1D<RC::RStr>& lines) {
    for (size_t i=0; i<lines.size(); i++) {
      bool in_quote = false;
      bool escaped = false;
      bool start_comment = false;
      auto& line = lines[i];
      for (size_t c=0; c<line.size(); c++) {
        if (in_quote) {
          if (escaped) {
            escaped = false;
          }
          else if (line[c] == '"') {
            in_quote = false;
          }
          else if (line[c] == '\\') {
            escaped = true;
          }
        }
        else {
          if (start_comment) {
            if (line[c] == '/') {
              line = line.substr(0, c-1);
              break;
            }
            start_comment = false;
          }
          else {
            if (line[c] == '/') {
              start_comment = true;
            }
            else if (line[c] == '#') {
              line = line.substr(0, c);
              break;
            }
            else if (line[c] == '"') {
              in_quote = true;
            }
          }
        }
      }
    }
  }


  class JSONFile {
    public:
    JSONFile() {}

    JSONFile(RC::RStr pathname) {
      Load(pathname);
    }

    void Load(RC::RStr pathname) {
      filename = pathname;
      RC::FileRead fr(pathname);
      RC::Data1D<RC::RStr> lines;
      fr.ReadAllLines(lines);
      RemoveComments(lines);
      json = json.parse(RC::RStr::Join(lines, "\n").c_str());
    }

    void Save(RC::RStr pathname) {
      RC::FileWrite fw(pathname);
      fw.WriteStr(json.dump(2));
    }

    template<class T, class... Keys>
    void Get(T& data, Keys... keys) {
      GetRecurse(data, RC::RStr("Could not get ")+filename+" key, ", json, keys...);
    }


    nlohmann::json json;

    protected:

    template<class T, class JSONT, class... Keys>
    void GetRecurse(T& data, RC::RStr err_msg, JSONT& node,
        Keys... keys) {
      try {
        node.get_to(data);
      }
      catch (nlohmann::json::exception &e) {
        Throw_RC_Type(File, (err_msg + "\n" + e.what()).c_str());
      }
    }

    template<class T, class JSONT, class Key, class... Keys>
    void GetRecurse(T& data, RC::RStr err_msg, JSONT& node,
        Key key, Keys... keys) {
      err_msg += RC::RStr(key) + ":";
      auto itr = node.find(key);
      if (itr == node.end()) {
        Throw_RC_Type(File, (err_msg + "\nError found").c_str());
      }
      GetRecurse(data, err_msg, *itr, keys...);
    }

    RC::RStr filename;
  };
}

#endif // EXPCONFIG_H

