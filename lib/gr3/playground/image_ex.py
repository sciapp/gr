import gr3
import Image
import atoms
import sys
from itertools import combinations
import numpy
import numpy.linalg

def log_callback(message):
    print "Log: ", message
gr3.setlogcallback(log_callback)

if not len(sys.argv) == 2:
    print "Usage: python %s <filename>" % sys.argv[0]
    sys.exit()

atom_data = []
with open(sys.argv[1]) as xyzfile:
    for line in xyzfile.readlines()[3:]:
        if len(line.split()) >= 4:
            o, x, y, z = line.split()[:4]
            atom_data.append((o.upper(), numpy.array((x,y,z),float)))

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
        cylinder_lengths.append(dist/2)
        cylinder_colors.append((0.55,0.55,0.55))
        cylinder_radii.append(0.05)


# gr3 mit einem 1024x1024 Framebuffer initialisieren
gr3.init([gr3.GR3_InitAttribute.GR3_IA_FRAMEBUFFER_WIDTH, 1024, gr3.GR3_InitAttribute.GR3_IA_FRAMEBUFFER_HEIGHT, 1024])

# Atome zeichnen
gr3.drawspheremesh(len(atom_data), sphere_positions, sphere_colors, sphere_radii)
# Atombindungen zeichnen
gr3.drawcylindermesh(len(cylinder_positions), cylinder_positions, cylinder_directions, cylinder_colors, cylinder_radii, cylinder_lengths)

# Kamera einstellen
gr3.setcameraprojectionparameters(45, 1, 200)
gr3.cameralookat(0, 10, 1, 0, 0, 0, 0, 0, 1)

# Bild erzeugen
width, height = 2048, 2048
bitmap = gr3.getpixmap(width, height)

# gr3 beenden
gr3.terminate()

# Bild speichern
img = Image.frombuffer("RGBA", (width, height), bitmap, "raw", "RGBA", 0, 0)
img.save("image_ex.png", "PNG")

