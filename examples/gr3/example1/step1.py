import wx

class MainFrame(wx.Frame):
    def __init__(self, parent=None, *args, **kwargs):
        super(MainFrame,self).__init__(parent, *args, **kwargs)
        
        
        self.panel = wx.Panel(self)
        
        self.panel_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.panel.SetSizerAndFit(self.panel_sizer)
        
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(self.panel,1,wx.EXPAND)
        self.SetSizerAndFit(self.sizer)


if __name__ == "__main__":
    app = wx.App()
    frame = MainFrame(title="Example 1")
    frame.Show()
    app.MainLoop()
