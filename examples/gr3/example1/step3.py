import wx
import wx.glcanvas
import gr3

class MainFrame(wx.Frame):
    def __init__(self, parent=None, *args, **kwargs):
        super(MainFrame,self).__init__(parent, *args, **kwargs)
        
        self.Bind(wx.EVT_CLOSE,self.on_close)
        self.panel = wx.Panel(self)
        
        gl_canvas_attribs = [wx.glcanvas.WX_GL_RGBA,
                             wx.glcanvas.WX_GL_DOUBLEBUFFER,
                             wx.glcanvas.WX_GL_DEPTH_SIZE, 16]
        self.gl_canvas = wx.glcanvas.GLCanvasWithContext(self.panel, attribList = gl_canvas_attribs)
        self.gl_canvas.SetMinSize((150,150))
        self.gl_canvas.Bind(wx.EVT_PAINT, self.on_paint_gl_canvas)
        self.gr3_initialized = False 
        
        self.panel_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.panel_sizer.Add(self.gl_canvas, 1, wx.EXPAND)
        self.panel.SetSizerAndFit(self.panel_sizer)
        
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(self.panel,1,wx.EXPAND)
        self.SetSizerAndFit(self.sizer)

    def on_paint_gl_canvas(self,evt):
        self.gl_canvas.SetCurrent()
        size = self.gl_canvas.GetSize()
        if not self.gr3_initialized:
            self.init_gr3()
        gr3.drawimage(0, size.width, 0, size.height, int(size.width), int(size.height), gr3.GR3_Drawable.GR3_DRAWABLE_OPENGL)
        self.gl_canvas.SwapBuffers()

    def init_gr3(self):
        if self.gr3_initialized:
            return
        self.gr3_initialized = True
        
        gr3.init()
        gr3.setcameraprojectionparameters(45, 1, 200)
        gr3.cameralookat(0, 0, -3, 0, 0, 0, 0, 1, 0)
        
        self.update_scene()
    
    def update_scene(self):
        gr3.clear()
        self.Refresh()
    
    def on_close(self, event):
        if self.gr3_initialized:
            gr3.terminate()
        event.Skip()

if __name__ == "__main__":
    app = wx.App()
    frame = MainFrame(title="Example 1")
    frame.Show()
    app.MainLoop()
