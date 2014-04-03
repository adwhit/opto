from graph_tool.all import *
import numpy as np
import sys

def loadnodes(path):
    input_data = open(path).read()
    lines = input_data.split("\n")
    nnodes = int(lines[0].strip())
    arr = [(float(n[0]), float(n[1])) for n in [line.split() for line in lines[1:] if line]]
    return arr

def creategraph(nnodes, route=None):
    g = Graph()
    g.add_vertex(nnodes)
    if route:
        for ix in range(len(route)-1):
            g.add_edge(g.vertex(route[ix]), g.vertex(route[ix+1]))
        g.add_edge(g.vertex(route[-1]), g.vertex(route[0]))
    return g

def draw(g, nodes, output=None):
    locs = np.array(nodes)
    pos = g.new_vertex_property("vector<float>") 
    pos.set_2d_array(locs.T)
    graph_draw(g, pos, vertex_size=10, output=output)

def makeSlides(inputpath, respath):
    nodearr = loadnodes(inputpath)

    lines = open(respath).readlines()
    snapsstr = [line.strip() for (ix,line) in enumerate(lines) if ix %2 == 0]
    scores = [float(line.strip()) for (ix,line) in enumerate(lines) if ix %2 == 1]
    snaps = [[int(x) for x in line.split()] for line in snapsstr]
    
    print snaps, scores

    for (ix, route) in enumerate(snaps):
        g = creategraph(len(route), route)
        draw(g, nodearr, "slides/%s_%d.png" % (inputpath[5:], ix))



if __name__== "__main__":
    if len(sys.argv) < 3:
        print "Please supply input, results"
        sys.exit()
    makeSlides(sys.argv[1], sys.argv[2])
