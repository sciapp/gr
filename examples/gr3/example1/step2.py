import wx
import wx.glcanvas
from OpenGL.GL import glGetString, GL_VERSION

class MainFrame(wx.Frame):
    def __init__(self, parent=None, *args, **kwargs):
        super(MainFrame,self).__init__(parent, *args, **kwargs)
        
        self.panel = wx.Panel(self)
        
        gl_canvas_attribs = [wx.glcanvas.WX_GL_RGBA,
                             wx.glcanvas.WX_GL_DOUBLEBUFFER,
                             wx.glcanvas.WX_GL_DEPTH_SIZE, 16]
        self.gl_canvas = wx.glcanvas.GLCanvasWithContext(self.panel, attribList = gl_canvas_attribs)
        self.gl_canvas.SetMinSize((150,150))
        self.gl_canvas.Bind(wx.EVT_PAINT, self.on_paint_gl_canvas)
        
        self.panel_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.panel_sizer.Add(self.gl_canvas, 1, wx.EXPAND)
        self.panel.SetSizerAndFit(self.panel_sizer)
        
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(self.panel,1,wx.EXPAND)
        self.SetSizerAndFit(self.sizer)

    def on_paint_gl_canvas(self,evt):
        self.gl_canvas.SetCurrent()
        print glGetString(GL_VERSION)


if __name__ == "__main__":
    app = wx.App()
    frame = MainFrame(title="Example 1")
    frame.Show()
    app.MainLoop()
