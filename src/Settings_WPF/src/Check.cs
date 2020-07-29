using System;
using System.Collections.Generic;
using System.Text;

namespace GUI_Testing {
  class Check {
    static public bool isTextBox(System.Windows.UIElement _ui) {
      return (_ui.GetType().ToString() == "System.Windows.Controls.TextBox");
    }
    static public bool isTextBlock(System.Windows.UIElement _ui) {
      return (_ui.GetType().ToString() == "System.Windows.Controls.TextBlock");
    }
  }
}
