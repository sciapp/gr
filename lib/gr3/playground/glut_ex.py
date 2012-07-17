# coding=utf-8
import atoms
import sys
import math
import numpy
import numpy.linalg
from itertools import combinations
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GLU import *
import gr
import gr3
from gr import * 
import math, time

def display():
    global window_width, window_height, rx
    gr3.setbackgroundcolor(1,1,1,1)
    # Kamera einstellen
    gr3.setcameraprojectionparameters(45, 1, 200)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 0, 0, 1)

    gr3.drawimage(0, window_width, 0, window_height, window_width, window_height, gr3.GR3_Window.GR3_WINDOW_OPENGL)
    glViewport(0,0,window_width,window_height);
    glDisable(GL_LIGHTING)
    glDisable(GL_DEPTH_TEST)
    glMatrixMode(GL_MODELVIEW)
    glPushMatrix()
    glLoadIdentity()
    glMatrixMode(GL_PROJECTION)
    glPushMatrix()
    glLoadIdentity()
    glColor4f(1,0,0,1)
    x, y = 0, 0.22
    glRasterPos2f(x*2-1,y*2-1)
    for c in u"Dies ist ein GLUT-Fenster, in dem mit GR3 eine":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,ord(c))
    y-=0.05
    glRasterPos2f(x*2-1,y*2-1)
    for c in u"Szene gerendert wird. Mit der Maus kann man":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,ord(c))
    y-=0.05
    glRasterPos2f(x*2-1,y*2-1)
    for c in u"das dargestellte Molekül rotieren lassen.":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,ord(c))
    y-=0.04
    glRasterPos2f(x*2-1,y*2-1)
    for c in u"(Rechtsklick öffnet das Kontextmenü)":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,ord(c))
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_LIGHTING)
    glMatrixMode(GL_MODELVIEW)
    glPopMatrix()
    glMatrixMode(GL_PROJECTION)
    glPopMatrix()
    glutSwapBuffers()
    gr.clearws()
    gr3.setquality(4)
    gr3.drawimage(0.15, 0.5, 0.46, 0.81,175,175, gr3.GR3_Window.GR3_WINDOW_GKS)
    gr.settextcolorind(1)
    gr.settextfontprec(6,0)

    setcharheight(0.02)
    settextalign(TEXT_HALIGN_LEFT, TEXT_VALIGN_BASE)
    gr.mathtex(0.05,0.01,"i\\hbar\\frac{\\partial\\psi}{\\partial t} = \\frac{\\hbar^2}{2m}\\nabla^2\\psi + V(\\mathbf{r})\\psi")

    x = list(range(5))
    y = list(range(5))

    selntran(0)

    # text fonts
    for i in range(-1, -23, -1):
      settextfontprec(i, TEXT_PRECISION_STROKE)
      if i >= -11:
        text(0.6, 0.75 + i * 0.05, "Font" + "%d" %i)
      else:
        text(0.8, 0.75 + (i + 11) * 0.05, "Font" + "%d" %i)

    settextfontprec(-23, TEXT_PRECISION_STROKE)
    text(0.6, 0.15, "Font-23")
    settextfontprec(-24, TEXT_PRECISION_STROKE)
    text(0.8, 0.15, "Font-24")

    # colors

    settextfontprec(-5, TEXT_PRECISION_STRING)
    settextcolorind(0)
    text(0.45, 0.35, "White")
    settextcolorind(1)
    text(0.45, 0.32, "Black")
    settextcolorind(2)
    text(0.45, 0.29, "Red")
    settextcolorind(3)
    text(0.45, 0.26, "Green")
    settextcolorind(4)
    text(0.45, 0.23, "Blue")
    settextcolorind(5)
    text(0.45, 0.20, "Cyan")
    settextcolorind(6)
    text(0.45, 0.17, "Yellow")
    settextcolorind(7)
    text(0.45, 0.14, "Magenta")

    # linetypes

    x[0] = 0.12
    x[1] = 0.28
    y[0] = 0.95
    y[1] = y[0]

    for j in range(-8, 5):
      if j != 0:
        setlinetype(j)
        y[0] = y[0] - 0.02
        y[1] = y[0]
        polyline(2, x, y)

    for i in range(1,4):
      y[0] = y[0] - 0.03
      setlinewidth(i)
      for j in range(1, 5):
        setlinetype(j)
        y[0] = y[0] - 0.02
        y[1] = y[0]
        polyline(2, x, y)

    setlinewidth(1)

    # markertypes

    setmarkersize(4.0)

    x[0] = 0.3
    y[0] = 0.95
    for j in range (-20, -12):
      setmarkertype(j)
      x[0] = x[0] + 0.065
      polymarker(1, x, y)

    x[0] = 0.3
    y[0] = 0.875
    for j in range (-12, -4):
      setmarkertype(j)
      x[0] = x[0] + 0.065
      polymarker(1, x, y)

    x[0] = 0.3
    y[0] = 0.8
    for j in range(-4, 5):
      if j != 0:
        setmarkertype(j)
        x[0] = x[0] + 0.065
        polymarker(1, x, y)

    # fill areas

    x[0] = 0.02
    x[1] = 0.12
    x[2] = 0.12
    x[3] = 0.02
    x[4] = x[0]
    y[0] = 0.02
    y[1] = 0.02
    y[2] = 0.12
    y[3] = 0.12
    y[4] = y[0]

    setfillstyle(4)
    for j in range(0, 4):
      for i in range(0, 5):
        x[i] = x[i] + 0.1
        y[i] = y[i] + 0.1
      setfillintstyle(j)
      fillarea(5, x, y)
     
    # patterns

    x[0] = 0.05
    x[1] = 0.1
    x[2] = 0.1
    x[3] = 0.05
    x[4] = x[0]
    y[0] = 0.2
    y[1] = 0.2
    y[2] = 0.25
    y[3] = 0.25
    y[4] = y[0]

    setfillintstyle(2)
    for j in range(4, 16):
      for i in range(0, 5):
        y[i] = y[i] + 0.06
      setfillstyle(j - 3)
      fillarea(5, x, y)

    # cell array

    setcharheight(0.06)
    settextalign(TEXT_HALIGN_CENTER, TEXT_VALIGN_HALF)
    settextcolorind(1)
    settextfontprec(12, TEXT_PRECISION_STROKE)
    text(0.75, 0.05, "Hello World")
    gr.updatews()
    gr3.setcameraprojectionparameters(45, 1, 200)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 0, 0, 1)

def reshape(width, height):
    global window_width, window_height
    window_width = width
    window_height = height

rx = 0
def mouse_motion(x,y):
    global rx
    rx = (x-window_width*0.5)*2/min(window_width,window_height)
    rx = max(min(rx,1),-1)
    glutPostRedisplay()

def keyboard(key, *args):
    if key == 'p':
        gr3.export("test.pov",0,0)
    elif key == 'j':
        gr3.export("test.html",800,800)
    elif key == 'n':
        gr3.export("test.png",800,800)
    elif key == 'm':
        gr3.export("test.jpg",800,800)
    elif key == ' ':
        gr3.terminate()
        sys.exit()
    elif key == '0':
        gr3.setquality(0)
    elif key == '1':
        gr3.setquality(2)
    elif key == '2':
        gr3.setquality(4)
    elif key == '3':
        gr3.setquality(8)
    elif key == '4':
        gr3.setquality(3)
    elif key == '5':
        gr3.setquality(5)
    elif key == '6':
        gr3.setquality(9)
    else:
        print key, ord(key)

window_width = 400
window_height = 400
export_quality = gr3.GR3_Quality.GR3_QUALITY_OPENGL_NO_SSAA
def init_glut():
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH)
    glutInitWindowSize(window_width,window_height)
    glutCreateWindow("GLUT window")
    glutDisplayFunc(display)
    glutReshapeFunc(reshape)
    glutMotionFunc(mouse_motion)
    glutKeyboardFunc(keyboard)
    export_menu = glutCreateMenu(on_export_menu)
    glutAddMenuEntry("POV-Ray scene (scene.pov)", 1)
    glutAddMenuEntry("HTML5/WebGL (scene.html)", 2)
    glutAddMenuEntry("JPEG image (scene.jpg)", 3)
    glutAddMenuEntry("PNG image (scene.png)", 4)
    opengl_quality_menu = glutCreateMenu(on_quality_menu)
    glutAddMenuEntry(" No SSAA", gr3.GR3_Quality.GR3_QUALITY_OPENGL_NO_SSAA)
    glutAddMenuEntry(" 2x SSAA", gr3.GR3_Quality.GR3_QUALITY_OPENGL_2X_SSAA)
    glutAddMenuEntry(" 4x SSAA", gr3.GR3_Quality.GR3_QUALITY_OPENGL_4X_SSAA)
    glutAddMenuEntry(" 8x SSAA", gr3.GR3_Quality.GR3_QUALITY_OPENGL_8X_SSAA)
    glutAddMenuEntry("16x SSAA", gr3.GR3_Quality.GR3_QUALITY_OPENGL_16X_SSAA)
    povray_quality_menu = glutCreateMenu(on_quality_menu)
    glutAddMenuEntry(" No SSAA", gr3.GR3_Quality.GR3_QUALITY_POVRAY_NO_SSAA)
    glutAddMenuEntry(" 2x SSAA", gr3.GR3_Quality.GR3_QUALITY_POVRAY_2X_SSAA)
    glutAddMenuEntry(" 4x SSAA", gr3.GR3_Quality.GR3_QUALITY_POVRAY_4X_SSAA)
    glutAddMenuEntry(" 8x SSAA", gr3.GR3_Quality.GR3_QUALITY_POVRAY_8X_SSAA)
    glutAddMenuEntry("16x SSAA", gr3.GR3_Quality.GR3_QUALITY_POVRAY_16X_SSAA)
    quality_menu = glutCreateMenu(on_quality_menu)
    glutAddSubMenu("OpenGL", opengl_quality_menu)
    glutAddSubMenu("POV-Ray", povray_quality_menu)
    glutCreateMenu(on_main_menu)
    glutAddSubMenu("Export as...", export_menu)
    glutAddSubMenu("Set export quality", quality_menu)
    glutAddMenuEntry("Quit", 0)
    glutAttachMenu(GLUT_RIGHT_BUTTON);
def init_gr3():
    def log_callback(message):
        print "Log: ", message
    gr3.setlogcallback(log_callback)
    
    gr3.init([gr3.GR3_InitAttribute.GR3_IA_FRAMEBUFFER_WIDTH, 1024, gr3.GR3_InitAttribute.GR3_IA_FRAMEBUFFER_HEIGHT, 1024])
    print gr3.getrenderpathstring()
    
    # Atome zeichnen
    gr3.drawspheremesh(len(atom_data), sphere_positions, sphere_colors, sphere_radii)
    # Atombindungen zeichnen
    gr3.drawcylindermesh(len(cylinder_positions), cylinder_positions, cylinder_directions, cylinder_colors, cylinder_radii, cylinder_lengths)

def on_quality_menu(entry):
    global export_quality
    export_quality = entry
    return 0

def on_export_menu(entry):
    if entry == 1:
        filename = "scene.pov"
    elif entry == 2:
        filename = "scene.html"
    elif entry == 3:
        filename = "scene.jpg"
    elif entry == 4:
        filename = "scene.png"
    else:
        return 1
    gr3.setquality(export_quality)
    gr3.export(filename,800,800)
    gr3.setquality(0)
    return 0

def on_main_menu(entry):
    gr3.terminate()
    sys.exit()
    return 0

if __name__ == "__main__":
    if not len(sys.argv) == 2:
        print "Usage: python %s <filename>" % sys.argv[0]
        sys.exit()

    atom_data = []
    with open(sys.argv[1]) as xyzfile:
        for line in xyzfile.readlines()[3:]:
            if len(line.split()) >= 4:
                o, x, y, z = line.split()[:4]
                atom_data.append((o.upper(), numpy.array((x,y,z),float)))
            else:
                break

    sphere_positions = []
    sphere_colors = []
    sphere_radii = []
    for o, pos in atom_data:
        sphere_positions.append(pos)
        sphere_colors.append([c/255.0 for c in atoms.atom_color_list[atoms.atomic_number_dict[o]]])
        sphere_radii.append(0.2)

    # ... und Atombindungen als Zylinder darstellen
    cylinder_positions = []
    cylinder_directions = []
    cylinder_lengths = []
    cylinder_colors = []
    cylinder_radii = []
    for pos1, pos2 in combinations(sphere_positions,2):
        dist = numpy.linalg.norm(pos2-pos1)
        if dist < 2.2:
            cylinder_positions.append(pos1)
            cylinder_directions.append(pos2-pos1)
            cylinder_lengths.append(dist)
            cylinder_colors.append((0.55,0.55,0.55))
            cylinder_radii.append(0.05)

    
    init_glut()
    init_gr3()
    glutMainLoop()
