using System;
using System.Collections.Generic;
using System.Text;

namespace GUI_Testing {
  class Config_Reader {
    public string _doc = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + "\\ArduinoMixerData";
    public struct preset {
      public int index;
      public string name { get; set; }
      public preset(string _name, int _index) {
        name = _name;
        index = _index;
      }
    };


    public struct renamed {
      public string _old { get; set; }
      public string _new { get; set; }

      public renamed(string _o, string _n) {
        _old = _o;
        _new = _n;
      }

    };
    public string token;
    public List<string> programs = new List<string>();
    public List<preset> pre_set_sessions = new List<preset>();
    public List<renamed> _renamed_sessions = new List<renamed>();
    public List<string> _exclude = new List<string>();
    public bool enable_logging = new bool();
    private List<string> parse_libconfig_array(string line) {
      List<string> buffer = new List<string>();
      for (int a = 0; a < line.Length; a++) {
        string buff = "";
        if (line[a] == '\"') {
          a++;
          while (line[a] != '\"') {
            buff += line[a];
            a++;
          }
          buffer.Add(buff);
        }
      }
      return buffer;
    }

    public void read_config_file() {
      read_token_file();
      string _p = _doc + "\\friendly.txt";
      System.IO.StreamReader reader = new System.IO.StreamReader(_p);
      string line;
      while ((line = reader.ReadLine()) != null) {
        if (line.IndexOf("{") > 0) {
          while ((line = reader.ReadLine()) != null) {
            if (line.IndexOf("}") >= 0)
              break;
            programs.Add(line.Substring(0, line.IndexOf(",")));
          }
        }
      }
      reader.Close();
      string ini_p = _doc + "\\settings.ini";
      string _p_list = "";
      string _r_list = "";
      string _e_list = "";
      System.IO.StreamReader ini_reader = new System.IO.StreamReader(ini_p);

      while ((line = ini_reader.ReadLine()) != null) {
        if (line.Contains("Prefered_List")) {
          _p_list = line;
        }
        else if (line.Contains("Rename_List")) {
          _r_list = line;
        }
        else if (line.Contains("Exclude_List")) {
          _e_list = line;
        }else if(line.Contains("Enable_Logging")) {
          if (line.Contains("false"))
            enable_logging = false;
          else if (line.Contains("true"))
            enable_logging = true;
        }
      }

      List<string> __p = parse_libconfig_array(_p_list);
      for (int a = 0; a < __p.Count; a++) {
        if (!__p[a].Contains("UNUSED"))
          pre_set_sessions.Add(new preset(__p[a], a + 1));
      }

      List<string> _r = parse_libconfig_array(_r_list);

      for (int a = 0; a < _r.Count; a++) {
        string _o_buff = _r[a].Substring(0, _r[a].IndexOf("-"));
        string _n_buff = _r[a].Substring(_r[a].IndexOf(">") + 1);
        _renamed_sessions.Add(new renamed(_o_buff, _n_buff));
      }

      _exclude = parse_libconfig_array(_e_list);

      ini_reader.Close();
    }

    public void remove_from_pre_set_list(string _t) {
      for (int a = 0; a < pre_set_sessions.Count; a++) {
        if (pre_set_sessions[a].name == _t)
          pre_set_sessions.RemoveAt(a);
      }
    }

    public void add_to_pre_set_list(System.Windows.Controls.TextBox _t) {
      int y = Int32.Parse(_t.Name.ToString().ToCharArray()[1].ToString());
      int x = Int32.Parse(_t.Name.ToString().ToCharArray()[4].ToString());
      int _session = ((y - 1) * 4) + (x - 1);
      Console.WriteLine(_session);
      for (int a = 0; a < pre_set_sessions.Count; a++) {
        if (_session + 1 == pre_set_sessions[a].index) {
          pre_set_sessions[a] = new preset(_t.Text, pre_set_sessions[a].index);
          return;
        }
      }
      pre_set_sessions.Add(new preset(_t.Text, _session + 1));
    }

    public void add_to_exclude_list(string s) {
      _exclude.Add(s);
    }

    public void remove_from_exclude_list(string s) {
      _exclude.Remove(s);
    }

    public void remove_from_rename_list(string s) {
      for(int a = 0; a < _renamed_sessions.Count; a++) {
        if(_renamed_sessions[a]._old == s) {
          _renamed_sessions.RemoveAt(a);
        }
      }
    }

    private void read_token_file() {
      string _p = _doc + "\\token";
      if (!System.IO.File.Exists(_p))
        return;
      System.IO.StreamReader reader = new System.IO.StreamReader(_p);
      token = reader.ReadLine();
      reader.Close();
      Console.WriteLine("Read token! " + token);
    }

    public void write_token_file() {
      string _p = _doc + "\\token";
      System.IO.StreamWriter writer = new System.IO.StreamWriter(_p);
      writer.WriteLine(token);
      writer.Close();
    }

    public void write_settings_file() {
      string _p = _doc + "\\settings.ini";
      System.IO.StreamReader reader = new System.IO.StreamReader(_p);
      List<string> vs = new List<string>();
      string line;
      while ((line = reader.ReadLine()) != null) {
        vs.Add(line);
      }
      reader.Close();
      System.IO.StreamWriter writer = new System.IO.StreamWriter(_p);
      
      List<string> full_list = new List<string>();
      for (int a = 0; a < 36; a++) {
        full_list.Add("UNUSED");
      }

      for (int a = 0; a < pre_set_sessions.Count; a++) {
        full_list[pre_set_sessions[a].index - 1] = pre_set_sessions[a].name;
      }

      string _final = "  Prefered_List = [";

      _final += " \"" + full_list[0] + "\"";

      for (int a = 1; a < 36; a++) {
        _final += ", ";
        _final += "\"" + full_list[a] + "\"";
      }

      _final += " ];";

      string _r_final = "  Rename_List = [";
      if (_renamed_sessions.Count > 0) {
        _r_final += " \"" + _renamed_sessions[0]._old + "->" + _renamed_sessions[0]._new + "\"";

        for (int a = 1; a < _renamed_sessions.Count; a++) {
          _r_final += ", ";
          _r_final += "\"" + _renamed_sessions[a]._old + "->" + _renamed_sessions[a]._new + "\"";
        }
      }
      _r_final += " ];";

      string _e_final = "  Exclude_List = [";
      if (_exclude.Count > 0) {
        _e_final += " \"" + _exclude[0] + "\"";

        for (int a = 1; a < _exclude.Count; a++) {
          _e_final += ", ";
          _e_final += "\"" + _exclude[a] + "\"";
        }
      }
      _e_final += " ];";

      for (int a = 0; a < vs.Count; a++) {
        if (vs[a].Contains("Prefered_List"))
          vs[a] = _final;
        else if (vs[a].Contains("Rename_List"))
          vs[a] = _r_final;
        else if (vs[a].Contains("Exclude_List"))
          vs[a] = _e_final;
        else if (vs[a].Contains("Enable_Logging"))
          vs[a] = "  Enable_Logging = " + (enable_logging ? "true" : "false") + ";";
        writer.WriteLine(vs[a]);
      }
      writer.Close();
    }
  }
}