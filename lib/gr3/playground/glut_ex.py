import atoms
import sys
import math
import Image
import numpy
import numpy.linalg
from itertools import combinations
from OpenGL.GLUT import *
import gr
import gr3

def display():
    global window_width, window_height, rx
    # Kamera einstellen
    gr3.setcameraprojectionparameters(45, 1, 200)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 0, 0, 1)
    
    gr3.renderdirect(window_width,window_height)
    glutSwapBuffers()
    gr.clearws()
    gr3.drawscene(0.0,0.5,0.0,0.5,800,800)
    gr3.cameralookat(0, 0, 10, 0, 0, 0, math.cos(-rx*math.pi/2), math.sin(-rx*math.pi/2), 0)
    gr3.drawscene(0.5,1.0,0.0,0.5,800,800)
    gr3.cameralookat(10*math.cos(-rx*math.pi/2), 0, 10*math.sin(-rx*math.pi/2), 0, 0, 0, 0, 1, 0)
    gr3.drawscene(0.5,1.0,0.5,1.0,800,800)
    gr.settextcolorind(2)
    gr.text(0.5,0.75,"Test!")
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
        povray()
    elif key == 'j':
        gr3.writehtml("test.html",800,800)
    elif key == ' ':
        sys.exit()
    else:
        print key, ord(key)
def povray():
    # Bild erzeugen
    width, height = 1024, 1024
    bitmap = gr3.getpovray(width, height)
    
    # Bild speichern
    img = Image.frombuffer("RGBA", (width, height), bitmap, "raw", "RGBA", 0, 0)
    img.save("glut_ex.png", "PNG")
    gr3.terminate()
    sys.exit()

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
