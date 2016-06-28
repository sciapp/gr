import gr
import gr3
gr.setviewport(0, 1, 0, 1)
for i in range(360):
    gr.clearws()
    gr3.clear()
    gr3.drawmolecule('dna.xyz', bond_delta=2, tilt=45, rotation=i)
    gr3.drawimage(0, 1, 0, 1, 500, 500, gr3.GR3_Drawable.GR3_DRAWABLE_GKS)
    gr.settextcolorind(0)
    gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_TOP)
    gr.text(0.5, 1, "DNA rendered using gr3.drawmolecule")
    gr.updatews()