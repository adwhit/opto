from graph_tool.all import *
import matplotlib.cm as cm
import numpy as np
import sys

if len(sys.argv) < 2:
    print "please supply file name"
    sys.exit(0)

if "/" in sys.argv[1]:
    rootpath = sys.argv[1].split("/")[-1]
else:
    rootpath = sys.argv[1]

path = "data/" + rootpath
lines = [line.strip() for line in open(path).readlines()]
(nnodes, nedges) = (int(x) for x in lines[0].split())
nodes = [(int(line.split()[0]), int(line.split()[1])) for line in lines[1:]]

path2 = "output/" + rootpath
lines = [line.strip() for line in open(path2).readlines()]
colours = [[int(v)+1 for v in line.split()] for line in lines]
ncolours = max(max(colours))
cmapped = [[cm.Spectral(float(x)/ncolours) for x in col] for col in colours]
first_set_vertex = colours[0].index(1)

G = Graph(directed=False)
G.add_vertex(nnodes)
for (n1, n2) in nodes:
    G.add_edge(n1, n2)

layout = graph_tool.draw.fruchterman_reingold_layout(G)
#layout = graph_tool.draw.radial_tree_layout(G, first_set_vertex)
colour = G.new_vertex_property("vector<float>")
degree = G.degree_property_map("out")


#for x in range(nnodes-1, nnodes):  #for testing
for x in range(nnodes):
    colour.set_2d_array(np.array(cmapped[x]).T)
    output = "pics/%s_%03d.png" % (rootpath, x)
    graph_draw(G, layout, vertex_fill_color=colour, vertex_size=20, edge_pen_width=0.1, 
               vertex_halo=True, vertex_halo_color=degree, output= output, output_size=(800,800), vcmap=cm.Blues)

