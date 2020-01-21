#ifndef EXPCONFIG_H
#define EXPCONFIG_H

#include "nlohmann/json.hpp"
#include "RC/RC.h"


namespace CML {
  // Removes # and // comments, left/right whitespace, and blank lines.
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

      line.Trim();
      if (line.empty()) {
        lines.Remove(i);
        i--;
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


  class CSVFile {
    public:
    CSVFile() {}

    CSVFile(RC::RStr pathname) {
      Load(pathname);
    }

    void Load(RC::RStr pathname) {
      filename = pathname;
      RC::FileRead fr(pathname);
      RC::Data1D<RC::RStr> lines;
      fr.ReadAllLines(lines);
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

    void Save(RC::RStr pathname) {
      RC::FileWrite fw(pathname);
      fw.WriteStr(RC::RStr::MakeCSV(data));
    }

    RC::Data2D<RC::RStr> data;

    protected:

    RC::RStr filename;
  };
}

#endif // EXPCONFIG_H

