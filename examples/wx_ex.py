#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
GR / wxPython interoperability example
"""

import wx
import os
import gr

# get_address = lambda obj_str: obj_str[obj_str.find('0x'): obj_str.find('>', obj_str.find('0x'))]

def get_address(obj_str):
    digits = '0123456789abcdef'
    obj_str = obj_str.lower()
    pos = obj_str.find('0x') + 2
    if pos >= 0:
        tmp_string = '0x'
        while pos < len(obj_str) and obj_str[pos] in digits:
            tmp_string += obj_str[pos]
            pos += 1
        return int(tmp_string, 16)
    else:
        return None

class GrWidget(wx.Panel):
    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id)
        
        self.bm = None
        
        self.setupUi()

        os.environ['GKS_WSTYPE'] = "380"
        os.environ['GKS_DOUBLE_BUF'] = "True"

        self.DrawButton.Bind(wx.EVT_BUTTON, self.draw)
        self.QuitButton.Bind(wx.EVT_BUTTON, self.quit)
        self.Bind(wx.EVT_PAINT, self.paintEvent)
                
    def setupUi(self):
        self.DrawButton = wx.Button(self, 1, 'Draw')
        self.DrawButton.SetPosition((300, 10))
        self.QuitButton = wx.Button(self, 2, 'Quit')
        self.QuitButton.SetPosition((400, 10))

    def quit(self, event):
        gr.emergencyclosegks()
        self.GetParent().Destroy()

    def draw(self, event):
        self.bm = wx.EmptyBitmap(self.GetSizeTuple()[0], self.GetSizeTuple()[1])
        dc = wx.MemoryDC(self.bm)
        
        os.environ['GKSconid'] = "%x!%x" % (get_address(repr(self)), get_address(repr(dc)))
        
        dc.SetBackground(wx.WHITE_BRUSH)
        dc.Clear()
        
        dc.DrawText("Surface Plot using wxWidgets ...", 15, 15)
        
        x = range(1, 481)
        y = range(1, 481)
        w, h, d = gr.readimage(
            os.path.join(os.path.dirname(os.path.realpath(__file__)),
                         'surf.png'))
        z = map(lambda x: x & 0xff, d)

        gr.setviewport(0, 1, 0, 1)
        gr.setwindow(1, 480, 1, 480)
        gr.setspace(1, 1000, 30, 80)
        gr.setcolormap(3)
        gr.surface(x, y, z, 6)
        gr.contour(x, y, range(1), z, 0)
        gr.updatews()
        
        self.Refresh()
        event.Skip()

    def paintEvent(self, ev):
        painter = wx.PaintDC(self)
        
        if self.bm != None:
            painter.DrawBitmap(self.bm, 0, 0)
            
class MainFrame(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title=title)
        self.widget = GrWidget(self, -1)
        self.widget.SetSize((500, 500))
        self.Fit()
        self.Centre()

if __name__ == '__main__':
    app = wx.App()

    win = MainFrame(None, -1, title='GRWidget')
    win.Show()

app.MainLoop()
