# -*- coding: utf-8 -*-

import atoms
import sys
import math
import numpy
import numpy.linalg
from itertools import combinations
import math, time
import gr
import gr3
from OpenGL.GLUT import *
from OpenGL.GL import *
from OpenGL.GLU import *
from gr import * 

def display():
    global window_width, window_height, rx
    gr3.setbackgroundcolor(1,1,1,1)
    # Kamera einstellen
    gr3.setcameraprojectionparameters(45, 1, 200)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 0, 0, 1)

    gr3.drawimage(0, window_width, 0, window_height, window_width, window_height, gr3.GR3_Drawable.GR3_DRAWABLE_OPENGL)
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
    selntran(0)
    gr3.setquality(4)
    gr3.drawimage(0, 0.5, 0.5, 1, 250, 250, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
    gr.settextcolorind(1)
    gr.settextfontprec(6,0)
    
    x = list(range(5))
    y = list(range(5))
    
    nominalWindowHeight = 500.0
    pointSize = ( 8, 9, 10, 11, 12, 14, 18, 24, 36 )
    s = "i\\hbar\\frac{\\partial\\psi}{\\partial t} = \\frac{\\hbar^2}{2m}\\nabla^2\\psi + V(\\mathbf{r})\\psi"
    x = 0.9
    y = 0.9;
    gr.settextalign(3, 3)
    for i in range(8):
        gr.setcharheight(pointSize[i] / nominalWindowHeight)
        gr.mathtex(x, y, s)
        y -= 4 * pointSize[i] / nominalWindowHeight
    
    gr.setcharheight(0.1)
    gr.mathtex(0.9, 0.05, "Hello World!")
    gr.settextcolorind(8)
    gr.text(0.9, 0.05, "Hello World!")

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
    if not len(sys.argv) >= 2:
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
