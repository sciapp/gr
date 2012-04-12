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

def display():
    global window_width, window_height, rx
    # Kamera einstellen
    gr3.setcameraprojectionparameters(45, 1, 200)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 0, 0, 1)

    gr3.renderdirect(window_width,window_height)
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
    for c in u"'j' - HTML5/WebGL Export":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,ord(c))
    y-=0.03
    glRasterPos2f(x*2-1,y*2-1)
    for c in u"'p' - POV-Ray Export":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,ord(c))
    y-=0.03
    glRasterPos2f(x*2-1,y*2-1)
    for c in u"' ' - Beenden":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,ord(c))
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_LIGHTING)
    glMatrixMode(GL_MODELVIEW)
    glPopMatrix()
    glMatrixMode(GL_PROJECTION)
    glPopMatrix()
    glutSwapBuffers()
    gr.clearws()
    gr3.drawscene(0.0,0.5,0.0,0.5,800,800)
    gr3.cameralookat(0, 0, 10, 0, 0, 0, math.cos(-rx*math.pi/2), math.sin(-rx*math.pi/2), 0)
    gr3.drawscene(0.5,1.0,0.0,0.5,800,800)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 0, 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 1, 0)
    gr3.drawscene(0.5,1.0,0.5,1.0,800,800)
    gr.settextcolorind(2)
    gr.text(0.05,0.9,"Dies ist ein GKS-Fenster,")
    gr.text(0.05,0.86,"in welches mit GR3 mehrere")
    gr.text(0.05,0.82,"Ansichten auf eine 3D-Szene")
    gr.text(0.05,0.78,"gezeichnet werden. Bewegt")
    gr.text(0.05,0.74,"man die Szene im GLUT-")
    gr.text(0.05,0.70,"Fenster, dann wird auch ")
    gr.text(0.05,0.66,"dieses Fenster angepasst.")
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
    else:
        print key, ord(key)

window_width = 400
window_height = 400
def init_glut():
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH)
    glutInitWindowSize(window_width,window_height)
    glutCreateWindow("GLUT window")
    glutDisplayFunc(display)
    glutReshapeFunc(reshape)
    glutMotionFunc(mouse_motion)
    glutKeyboardFunc(keyboard)
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
