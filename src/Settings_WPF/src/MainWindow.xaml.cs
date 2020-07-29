using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace GUI_Testing {
  public partial class MainWindow : Window {

    const int RETURN_OLD_NAME = 10;
    const int RETURN_NEW_NAME = 11;

    List<TextBlock> pref_list = new List<TextBlock>();

    string friendly = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + "\\ArduinoMixerData";
    Config_Reader config = new Config_Reader();
    ContextMenu cm;
    ContextMenu _lh_cm; // List handler context menu
    ContextMenu _e_lh_cm; // Excluded tab list handler context menu
    List<MenuItem> mi = new List<MenuItem>();
    MenuItem _lh_mi;
    MenuItem _e_lh_mi;

    string _rename_buffer = "";
    public MainWindow() {
      string[] args = Environment.GetCommandLineArgs();

      if (args.Length > 1)
      {
        if (args[1] == "1337_sweet_call_bro")
          Console.WriteLine("OK GOT IT!");
        else
          Environment.Exit(0);
      }else
      {
        Environment.Exit(0);
      }
      InitializeComponent();
      Console.WriteLine(config._doc);
      _lh_cm = new ContextMenu();
      _lh_mi = new MenuItem();
      _lh_mi.Header = "Exclude";
      _lh_mi.AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(mi_lh_exclude));
      _lh_cm.Items.Add(_lh_mi);

      _e_lh_cm = new ContextMenu();
      _e_lh_mi = new MenuItem();
      _e_lh_mi.Header = "Restore";
      _e_lh_mi.AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(mi_e_lh_restore));
      _e_lh_cm.Items.Add(_e_lh_mi);

      cm = new ContextMenu();
      mi.Add(new MenuItem());
      mi[mi.Count - 1].Header = "Clear";
      mi[mi.Count - 1].AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(mi_click_clear));
      cm.Items.Add(mi[mi.Count - 1]);

      mi.Add(new MenuItem());
      mi[mi.Count - 1].Header = "Clear Rename";
      mi[mi.Count - 1].AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(mi_click_clear_rename));
      cm.Items.Add(mi[mi.Count - 1]);

      mi.Add(new MenuItem());
      mi[mi.Count - 1].Header = "Rename";
      mi[mi.Count - 1].AddHandler(MenuItem.ClickEvent, new RoutedEventHandler(mi_click_rename));
      cm.Items.Add(mi[mi.Count - 1]);
      for (int a = 0; a < sessions.Children.Count; a++) {
        if (Check.isTextBox(sessions.Children[a])) {
          ((TextBox)sessions.Children[a]).ContextMenu = cm;
          ((TextBox)sessions.Children[a]).KeyDown += enable_rename;
          ((TextBox)sessions.Children[a]).FontSize = 13;
          ((TextBox)sessions.Children[a]).TextAlignment = TextAlignment.Center;
          ((TextBox)sessions.Children[a]).FontWeight = FontWeight.FromOpenTypeWeight(500);
          ((TextBox)sessions.Children[a]).Cursor = Cursors.Arrow;
        }
      }

      config.read_config_file();

      logging.IsChecked = config.enable_logging;

      for(int a = 0; a < config._exclude.Count; a++) {
        add_text_to_excluded_tab(config._exclude[a].ToUpper());
      }

      for(int a = 0; a < config.programs.Count; a++) {
        bool found = false;
        for(int b = 0; b < config._exclude.Count; b++) {
          if(config.programs[a].ToUpper() == config._exclude[b].ToUpper()) {
            found = true;
            break;
          }
        }
        if (!found) {
          add_text_to_main_tab(config.programs[a].ToUpper(), has_rename(config.programs[a].ToUpper()));
        }
          
      }

      ((TextBox)this.FindName("token")).Text = config.token;

      if(config.pre_set_sessions.Count > 0) {
        for(int a = 0; a < config.pre_set_sessions.Count; a++) {
          add_text_to_sessions(config.pre_set_sessions[a].name, config.pre_set_sessions[a].index);
          for(int b = 0; b < main.Children.Count; b++) {
            if(Check.isTextBlock(main.Children[b])) {
              if(get_name(((TextBlock)main.Children[b]).Text, RETURN_OLD_NAME) == config.pre_set_sessions[a].name) {
                if(has_rename(config.pre_set_sessions[a].name))
                  ((TextBlock)main.Children[b]).Foreground = Brushes.LightBlue;
                else
                  ((TextBlock)main.Children[b]).Foreground = Brushes.Gray;
              }
            }
          }
        }
      }
    }

    string get_name(string s, int type_by) {
      if (type_by == RETURN_NEW_NAME) {
        for (int a = 0; a < config._renamed_sessions.Count; a++) {
          if (config._renamed_sessions[a]._old == s || config._renamed_sessions[a]._new == s)
            return config._renamed_sessions[a]._new;
        }
      }else if(type_by == RETURN_OLD_NAME) {
        for (int a = 0; a < config._renamed_sessions.Count; a++) {
          if (config._renamed_sessions[a]._new == s || config._renamed_sessions[a]._old == s)
            return config._renamed_sessions[a]._old;
        }
      }
      return s;
    }

    bool has_rename(string s) {
      for (int a = 0; a < config._renamed_sessions.Count; a++) {
        if (config._renamed_sessions[a]._old == s || config._renamed_sessions[a]._new == s)
          return true;
      }
      return false;
    }

    void mi_e_lh_restore(object sender, RoutedEventArgs e) {
      TextBlock _t = (TextBlock)_e_lh_cm.PlacementTarget;
      config.remove_from_exclude_list(_t.Text);
      config.write_settings_file();
      add_text_to_main_tab(_t.Text);
      excluded.Children.Remove(_t);
    }

    void mi_lh_exclude(object sender, RoutedEventArgs e) {
      TextBlock _tb = (TextBlock)_lh_cm.PlacementTarget;

      for (int a = 0; a < sessions.Children.Count; a++) {
        if (Check.isTextBox(sessions.Children[a])) {
          if (get_name(((TextBox)sessions.Children[a]).Text, RETURN_NEW_NAME) == _tb.Text) {
            ((TextBox)sessions.Children[a]).Text = "";
          }
        }
      }

      bool found = false;
      for (int a = 0; a < main.Children.Count; a++) {
        if (Check.isTextBlock(main.Children[a])) {
          if (get_name(((TextBlock)main.Children[a]).Text, RETURN_NEW_NAME) == _tb.Text) {
            Console.WriteLine(((TextBlock)main.Children[a]).Text);
            add_text_to_excluded_tab(get_name(((TextBlock)main.Children[a]).Text, RETURN_OLD_NAME));
            config.add_to_exclude_list(get_name(((TextBlock)main.Children[a]).Text, RETURN_OLD_NAME));
            config.remove_from_pre_set_list(get_name(((TextBlock)main.Children[a]).Text, RETURN_OLD_NAME));
            config.remove_from_rename_list(get_name(((TextBlock)main.Children[a]).Text, RETURN_OLD_NAME));
            config.write_settings_file();
            main.Children.Remove(((TextBlock)main.Children[a]));
            found = true;
          }
          if (found) {
            ((TextBlock)main.Children[a]).Margin = new Thickness(-5, 20 + (a - 1) * (((TextBlock)main.Children[a]).FontSize + 3), 0, 0);
          }
        }
      }
    }

    void mi_click_clear(object sender, RoutedEventArgs e) {
      MenuItem mi = (MenuItem)sender;
      TextBox _t = (TextBox)cm.PlacementTarget;
      for(int a = 0; a < main.Children.Count; a++) {
        if(Check.isTextBlock(main.Children[a])) {
          if (((TextBlock)main.Children[a]).Text == get_name(_t.Text, RETURN_NEW_NAME)) {
            _t.Text = "";
            ((TextBlock)main.Children[a]).Foreground = Brushes.Black;
            config.remove_from_pre_set_list(get_name(((TextBlock)main.Children[a]).Text, RETURN_OLD_NAME));
            config.write_settings_file();
            break;
          }
        }
      }
    }

    void mi_click_clear_rename(object sender, RoutedEventArgs e) {
      TextBox _t = (TextBox)cm.PlacementTarget;
      if (has_rename(get_name(_t.Text, RETURN_NEW_NAME))) {
        for (int a = 0; a < main.Children.Count; a++) {
          if (Check.isTextBlock(main.Children[a])) {
            if (((TextBlock)main.Children[a]).Text == get_name(_t.Text, RETURN_NEW_NAME)) {
              ((TextBlock)main.Children[a]).Text = get_name(_t.Text, RETURN_OLD_NAME);
              ((TextBlock)main.Children[a]).Foreground = Brushes.Gray;
              break;
            }
          }
        }

        for (int a = 0; a < config._renamed_sessions.Count; a++) {
          if (config._renamed_sessions[a]._new == _t.Text) {
            _t.Text = get_name(_t.Text, RETURN_OLD_NAME);
            config._renamed_sessions.RemoveAt(a);
          }
        }
        config.write_settings_file();
      }
    }

    void mi_click_rename(object sender, RoutedEventArgs e) {
      TextBox _t = (TextBox)cm.PlacementTarget;
      if (_t.Text == "")
        return;
      _rename_buffer = get_name(_t.Text, RETURN_OLD_NAME);
      _t.CaretBrush = Brushes.Black;
      _t.Cursor = Cursors.IBeam;
      _t.Clear();
      _t.Focusable = true;
      _t.Focus();
    }

    void mi_change_logging(object sender, RoutedEventArgs e) {
      MenuItem mi = (MenuItem)sender;
      config.enable_logging = mi.IsChecked;
      config.write_settings_file();
    }

    private TextBox find_textbox_by_name(string s) { 
      for(int a = 0; a < sessions.Children.Count; a++) {
        if(Check.isTextBox(sessions.Children[a])) {
          if (((TextBox)sessions.Children[a]).Name == s) {
            return ((TextBox)sessions.Children[a]);
          }
        }
      }
      return new TextBox();
    }

    private void add_text_to_main_tab(string text, bool _has_rename = false) {
      TextBlock tb = new TextBlock();
      tb.Text = get_name(text, RETURN_NEW_NAME);
      tb.FontSize = 17;
      tb.Margin = new Thickness(-5, ((main.Children.Count - 1) * (tb.FontSize + 3)) + 20, 0, 0);
      tb.MouseEnter += Tb_MouseEnter;
      tb.MouseLeave += Tb_MouseLeave;
      tb.MouseLeftButtonDown += move_mouseevent;
      tb.MouseRightButtonUp += Tb_MouseRightButtonUp;
      tb.VerticalAlignment = VerticalAlignment.Top;
      tb.HorizontalAlignment = HorizontalAlignment.Center;
      tb.Height = tb.FontSize + 3;
      tb.Padding = new Thickness(tb.Padding.Left + 30, tb.Padding.Top, tb.Padding.Right + 30, tb.Padding.Bottom);
      if (_has_rename)
        tb.Foreground = Brushes.DarkBlue;
      main.Children.Add(tb);
    }

    private void add_text_to_excluded_tab(string text) {
      TextBlock tb = new TextBlock();
      tb.Text = text;
      tb.FontSize = 17;
      tb.Margin = new Thickness(-5, 20 + (excluded.Children.Count - 1) * (tb.FontSize + 3), 0, 0);
      tb.MouseEnter += Tb_MouseEnter;
      tb.MouseLeave += Tb_MouseLeave;
      tb.MouseRightButtonUp += Tb_e_MouseRightButtonUp;
      tb.HorizontalAlignment = HorizontalAlignment.Center;
      excluded.Children.Add(tb);
    }


    private void Tb_MouseRightButtonUp(object sender, MouseButtonEventArgs e) {
      _lh_cm.HasDropShadow = true;
      _lh_cm.PlacementTarget = (TextBlock)sender;
      _lh_cm.IsOpen = true;
    }

    private void Tb_e_MouseRightButtonUp(object sender, MouseButtonEventArgs e) {
      _e_lh_cm.HasDropShadow = true;
      _e_lh_cm.PlacementTarget = (TextBlock)sender;
      _e_lh_cm.IsOpen = true;
    }


    private void Tb_MouseLeave(object sender, MouseEventArgs e) {
      if (e.LeftButton == MouseButtonState.Released) {
        TextBlock tb = (TextBlock)sender;
        if (has_assigned_session(tb.Text) && has_rename(get_name(tb.Text, RETURN_OLD_NAME))) {
          tb.Foreground = Brushes.LightBlue;
          tb.Text = get_name(tb.Text, RETURN_NEW_NAME);
        }
        else if (has_rename(get_name(tb.Text, RETURN_OLD_NAME))) {
          tb.Foreground = Brushes.DarkBlue;
          tb.Text = get_name(tb.Text, RETURN_NEW_NAME);
        }
        else if (!has_assigned_session(tb.Text)) {
          tb.Foreground = Brushes.Black;
        }
      }
    }

    private void Tb_MouseEnter(object sender, MouseEventArgs e) {
      if (e.LeftButton == MouseButtonState.Released) {
        TextBlock tb = (TextBlock)sender;
        if (has_assigned_session(tb.Text)) {
          tb.Foreground = Brushes.Gray;
          tb.Text = get_name(tb.Text, RETURN_OLD_NAME);
        }
        else if (has_rename(get_name(tb.Text, RETURN_OLD_NAME))) {
          tb.Foreground = Brushes.Black;
          tb.Text = get_name(tb.Text, RETURN_OLD_NAME);
        }
        else {
          tb.Foreground = (SolidColorBrush)(new BrushConverter().ConvertFrom("#4D6466"));
        }
      }
    }



    private void add_text_to_sessions(string s, int index) {
      int y = ((index - 1) / 4) + 1;
      int x = ((index - 1) % 4) + 1;
      string name = "p" + y.ToString() + "_s" + x.ToString();
      TextBox _r = find_textbox_by_name(name);
      _r.MouseRightButtonUp += _open_context_menu_for_rectangles;
      _r.Text = get_name(s, RETURN_NEW_NAME);
    }

    private bool has_assigned_session(String s) {
      for (int a = 0; a < sessions.Children.Count; a++) {
        if (Check.isTextBox(sessions.Children[a])) {
          if (get_name(((TextBox)sessions.Children[a]).Text, RETURN_OLD_NAME) == get_name(s, RETURN_OLD_NAME))
            return true;
        }
      }
      return false;
    }

    private void enable_rename(object sender, KeyEventArgs e) {
      if(e.Key == Key.Enter) {        
        TextBox _t = (TextBox)sender;
        _t.Text = _t.Text.ToUpper();
        Console.WriteLine("{0}->{1}", _rename_buffer, _t.Text);
        _t.Focusable = false;
        _t.CaretBrush = Brushes.Transparent;
        _t.Cursor = Cursors.Arrow;
        bool found = false;
        for(int a =0;a < config._renamed_sessions.Count; a++) {
          if(get_name(_rename_buffer, RETURN_OLD_NAME) == config._renamed_sessions[a]._old) {
            config._renamed_sessions[a] = new Config_Reader.renamed(config._renamed_sessions[a]._old, _t.Text);
            found = true;
          }
        }
        if(!found)
          config._renamed_sessions.Add(new Config_Reader.renamed(_rename_buffer, _t.Text));

        for(int a = 0; a < main.Children.Count; a++) {
          if (Check.isTextBlock(main.Children[a])) {
            if(((TextBlock)main.Children[a]).Text == _rename_buffer) {
              ((TextBlock)main.Children[a]).Foreground = Brushes.LightBlue;
              ((TextBlock)main.Children[a]).Text = _t.Text;
            }
          }
        }

        config.write_settings_file();
      }else if(e.Key == Key.Escape) {
        TextBox _t = (TextBox)sender;
        _t.Text = _rename_buffer;
        _t.Focusable = false;
        _t.CaretBrush = Brushes.Transparent;
        _t.Cursor = Cursors.Arrow;
      }
    }

    private void move_mouseevent(object sender, MouseButtonEventArgs e) {
      TextBlock t = (TextBlock)sender;
      if (t.Foreground != Brushes.Gray && t.Foreground != Brushes.LightBlue) {
        t.Foreground = Brushes.Red;

        for(int a = 0; a < sessions.Children.Count; a++) {
          if (Check.isTextBox(sessions.Children[a])) {
            TextBox _r = (TextBox)sessions.Children[a];
            if(_r.Text == "") {
              _r.BorderBrush = Brushes.Red;
              _r.BorderThickness = new Thickness(3);
            }
          }
        }
      }
    }

    private void add_to_list(object sender, KeyEventArgs e) {
      if (e.Key == Key.Enter) {
        for(int a = 0; a < main.Children.Count; a++) {
          if (Check.isTextBlock(main.Children[a])) {
            if (((TextBox)sender).Text.ToUpper() == ((TextBlock)main.Children[a]).Text)
              return;
          }
        }
        TextBlock tb = new TextBlock();
        add_text_to_main_tab(Add_to_list.Text.ToUpper());
        ((TextBox)sender).Clear();
        Console.WriteLine(Add_to_list.Text);
      }else if(e.Key == Key.Escape) {
        Final_Handler.Focus();
      }
    }

    private void _open_context_menu_for_rectangles(object sender, RoutedEventArgs e) {
      this.cm.HasDropShadow = true;
      this.cm.PlacementTarget = (TextBox)sender;
      this.cm.IsOpen = true;
    }

    private void Window_MouseLeftButtonUp(object sender, MouseButtonEventArgs e) {
      for (int a = 0; a < main.Children.Count; a++) {
        TextBlock t = new TextBlock();
        if (Check.isTextBlock(main.Children[a])) {
          t = (TextBlock)main.Children[a];
        }
        if (t.Foreground.Equals(Brushes.Red)) {
          for (int b = 0; b < sessions.Children.Count; b++) {
            if (Check.isTextBox(sessions.Children[b])) {
              TextBox _r = (TextBox)sessions.Children[b];

              if (_r.IsMouseOver) {
                Console.WriteLine(_r.Name);
                t.Foreground = Brushes.Gray;
                for (int c = 0; c < sessions.Children.Count; c++) {
                  if (Check.isTextBox(sessions.Children[c])) {
                    ((TextBox)sessions.Children[c]).BorderBrush = Brushes.Black;
                    ((TextBox)sessions.Children[c]).BorderThickness = new Thickness(1);
                  }
                }
                _r.Text = t.Text;
                config.add_to_pre_set_list(_r);
                config.write_settings_file();
                //sessions.Children.Add(tb);
                return;
              }
            }
          }
          for (int c = 0; c < sessions.Children.Count; c++) {
            if (Check.isTextBox(sessions.Children[c])) {
              ((TextBox)sessions.Children[c]).BorderBrush = Brushes.Black;
              ((TextBox)sessions.Children[c]).BorderThickness = new Thickness(1);
            }
          }
          t.Foreground = Brushes.Black;
        }
      }
    }

    private void token_changed(object sender, KeyEventArgs e) {
      if (e.Key == Key.Enter) {
        Console.Out.WriteLine("Saved: " + ((TextBox)sender).Text);
        config.token = ((TextBox)sender).Text;
        config.write_token_file();
      }
    }
  }
}
