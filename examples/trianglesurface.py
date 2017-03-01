import gr
import gr3
import numpy as np
from scipy.spatial import Delaunay

# Set up example data
radii = np.linspace(0, 1, 20)
angles = np.linspace(0, np.pi*2, 30)
points = np.zeros((len(angles), len(radii), 3))
points[:, :, 0] = np.cos(angles).reshape(len(angles), 1) * radii.reshape(1, len(radii))
points[:, :, 1] = np.sin(angles).reshape(len(angles), 1) * radii.reshape(1, len(radii))
points[:, :, 2] = -radii.reshape(1, len(radii))
points.shape = (len(angles)*len(radii), 3)
points[:, :] = points[:, :]*0.5+0.5

# Perform 2D delaunay triangulation
triangles = Delaunay(points[:, :2]).simplices.copy()
points = points[triangles]

# Set up GR state
gr.setcolormap(1)
gr.clearws()
gr.setspace(points[:, 2].min(), points[:, 2].max(), 0, 0)

# Draw using GR3
gr3.drawtrianglesurface(points)
gr.updatews()
