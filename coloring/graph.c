//TODO:
//Come up with a decentbacktracking/optimisation algorithm
//Make python ffi
//Implement debug/verbose mode

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum {false, true} bool;
typedef struct Node Node;

struct Tuple {
    int *arr1;
    int *arr2;
};

struct Node {
    int index;
    int colour;
    int next_colour;             //for propagation
    int nneighbours;
    int nposscolours;
    int *neighbour_colours;
    Node **neighbours;
};

typedef struct {
    int ncolours;
    int maxsearchdepth;
    int *node_colours;
    Node *nodes;
    int **colourstate;
} Graph;

typedef struct {
    int itercount;
} SolveState;

int NNODES = 0;
int NEDGES = 0;

bool VERBOSE = false;
bool DEBUG = false;
bool DETAILEDRES = false;
bool PRINTSTATE = false;

void print_colours(Graph *g);
void print_colour_state(Graph *g);
void print_output(Graph *g);
void save_colourstate(int iter, Graph *g);

int cmp_node_nneighbours(const void *v1, const void *v2) {
    const Node *i1 = *(const Node **)v1;
    const Node *i2 = *(const Node **)v2;
    return i1->nneighbours > i2->nneighbours ? -1 : (i1->nneighbours < i2->nneighbours);
}

int *node_argsort_nneighbours(Node *array) {
    Node **parray = malloc(NNODES * sizeof(Node *));
    int *position = malloc(NNODES * sizeof(int));
    for(int i = 0; i < NNODES; i++) parray[i] = &array[i];
    qsort(parray, NNODES, sizeof(Node *), cmp_node_nneighbours);
    for(int i = 0;i < NNODES;i++) position[i] = (parray[i] - array);
    free(parray);
    return position;
}

int *find_most_constrained(Graph *g) {
    int *position = node_argsort_nneighbours(g->nodes);
    /*puts("Node constraint order");*/
    /*for (int i=0;i < NNODES; i++) {*/
        /*printf("index: %d  Rank %d\n", i, position[i]);*/
    /*}*/
    return position;
}

int cmp_colour_freq(const void *v1, const void *v2) {
    const int i1 = **(const int **)v1;
    const int i2 = **(const int **)v2;
    if (i1 < i2) return -1;
    if (i1 > i2) return 1;
    return 0;
}

int *colour_freq_sort(int *array, int ncolours) {
    int **parray = malloc(ncolours * sizeof(int *));
    int *position = malloc(ncolours * sizeof(int));
    for(int i = 0; i < ncolours; i++) parray[i] = &array[i];
    qsort(parray, ncolours, sizeof(int *), cmp_colour_freq);
    if (DEBUG) for (int i=0;i<ncolours;i++) {
        printf("after parray val %p, array val %p position %li\n", parray[i], array, parray[i] - array);
    }
    for(int i = 0;i < ncolours;i++) position[i] = (parray[i] - array);
    free(parray);
    return position;
}

int *find_rarest_colours(Graph *g) {
    int *counts = calloc(g->ncolours, sizeof(int));
    for (int i=0;i<NNODES;i++) {
        for (int j=0;j<g->ncolours;j++) {
            if (g->nodes[i].neighbour_colours[j]==0) counts[j]++;
        }
    }
    int *positions = colour_freq_sort(counts, g->ncolours);
    if (DEBUG) {
        puts("color count ranking");
        for (int i=0;i < g->ncolours; i++) {
            printf("Rank: %d Index: %d  Counts %d\n",i, positions[i], counts[positions[i]]);
        }
    }
    free(counts);
    return positions;
}

Graph construct_graph(int ncolours, int maxsearchdepth, int *n1inds,int *n2inds) {
    //initialise graph and nodes. Possible cffi entry point
    Graph g = {
        ncolours, maxsearchdepth,
        calloc(NNODES, sizeof(int)), 
        calloc(NNODES,sizeof(Node)),
        malloc(NNODES * sizeof(int *))};
    for (int i=0;i<NNODES;i++) {
        g.node_colours[i] = -1;
        g.colourstate[i] = malloc(NNODES * sizeof(int));
        Node n = { i, -1, -1, 0, g.ncolours,
            calloc(NNODES,sizeof(int)),
            calloc(NNODES,sizeof(Node *))
        };
        g.nodes[i] = n;
    };
    for (int i=0;i<NEDGES;i++) {
        int n1ind = n1inds[i];
        int n2ind = n2inds[i];
        Node *n1 = &g.nodes[n1ind];
        Node *n2 = &g.nodes[n2ind];
        n1->neighbours[n1->nneighbours] = n2;
        n2->neighbours[n2->nneighbours] = n1;
        n1->nneighbours++;
        n2->nneighbours++;
    }
    return g;
}

void free_graph(Graph *g) {
    free(g->node_colours);
    for (int i=0;i<NNODES;i++) {
        free(g->nodes[i].neighbour_colours);
        free(g->nodes[i].neighbours);
    };
    free(g->nodes);
};

void increase_ncolours(Graph *g) {
    if (VERBOSE) printf("Increasing ncolours to %d\n", g->ncolours+1);
    for (int i=0;i<NNODES;i++) {
        g->nodes[i].nposscolours++;
    }
    g->ncolours++;
}

bool can_change_colour(Node *n, int from_node_ix, Graph *g, int depth) {
    //see if a node can change colour immediately
    if (n->nposscolours > 1) {
        for (int i=0;i<g->ncolours;i++) {
            if ((n->colour != i) && (n->neighbour_colours[i] == 0)) {
                n->next_colour = i;
            }
        }
        return true;
    }
    if (depth >= g->maxsearchdepth) return false;
    //see if children can change
    //Assume all colours are available except node's current colour
    bool *colour_unavailable = calloc(g->ncolours, sizeof(int));
    colour_unavailable[g->node_colours[n->index]] = true;
    for (int i=0;i<n->nneighbours;i++) {
        // make sure to exlude from_node from checks
        int nodetocheck = n->neighbours[i]->index;
        if (nodetocheck != from_node_ix) {
            colour_unavailable[g->node_colours[nodetocheck]] = !can_change_colour;
        }
    }
    for (int i=0;i<g->ncolours;i++) {
        if (!colour_unavailable[i]) {
            n->next_colour = i;
            return true;
    }
    n->next_colour = -1;
    return false;
}

bool change_and_propagate(Node *n, Graph *g, int depth) {
    if (n->nposscolours == 1) {
        //ask further along chain
        for (int i=0;i<n->nneighbours;i++) {
            if (n->neighbours[i]->colour == n->next_colour) {
                change_and_propagate(n->neighbours[i], g, depth+1);
            }
        }
    }
    if (n->nposscolours != 1) return false;
    for (int i=0;i<n->nneighbours;i++) {
        n->neighbours[i]->neighbour_colours[n->next_colour]++;
        n->neighbours[i]->neighbour_colours[n->colour]--;
    }
    n->colour = n->next_colour;
    n->next_colour = -1;
    g->node_colours[n->index] = n->colour;
    return true;
}


void set_and_propagate(int next_node_ix, int next_node_colour, Graph *g) {
    if (VERBOSE) printf("Setting node %d to colour %d\n", next_node_ix, next_node_colour);
    //set
    g->node_colours[next_node_ix] = next_node_colour;
    g->nodes[next_node_ix].colour = next_node_colour;
    //propagate - remove colour from nearby nodes
    Node node = g->nodes[next_node_ix];
    for (int j=0;j<node.nneighbours;j++) {
        Node *linkednode = node.neighbours[j];
        if (linkednode->neighbour_colours[next_node_colour]==0) {
            linkednode->neighbour_colours[next_node_colour]++;
            linkednode->nposscolours--;
            if (linkednode->nposscolours == 0) increase_ncolours(g);
        }
    }
}


void choose_next(int *next_node, int *next_colour, Graph *g) {
    int *constrained_order = find_most_constrained(g);
    int *rarest_colours = find_rarest_colours(g);
    int nodeind = 0;
    int colourind = 0;
    // make sure node hasn't already been set
    while (g->node_colours[constrained_order[nodeind]] != -1) nodeind ++;
    // make sure node colour is possible
    Node most_constrained_node = g->nodes[constrained_order[nodeind]];
    while (most_constrained_node.neighbour_colours[rarest_colours[colourind]]!=0) colourind++;
    *next_node = constrained_order[nodeind];
    *next_colour = rarest_colours[colourind];
    free(constrained_order);
    free(rarest_colours);
}

void solver(int next_node_ix, int next_node_colour, Graph *g, int ncoloured) {
    set_and_propagate(next_node_ix, next_node_colour, g);
    save_colourstate(ncoloured, g);
    if (VERBOSE) print_colour_state(g);
    if (ncoloured == NNODES) return;
    else {
        int next_node;
        int next_colour;
        choose_next(&next_node, &next_colour, g);
        solver(next_node, next_colour, g, ncoloured +1);
    }
    return;
}

void solve(int *n1ind, int *n2ind) {
    // TODO: free graph on failure
    Graph graph = construct_graph(2, 2, n1ind, n2ind);
    int *constrained_order = find_most_constrained(&graph);
    solver(constrained_order[0], 0, &graph, 1);
    if (DETAILEDRES) print_colour_state(&graph);
    print_output(&graph);
}

void save_colourstate(int ncoloured, Graph *g) {
    for (int j=0;j<NNODES;j++) {
        g->colourstate[ncoloured-1][j] = g->node_colours[j];
    }
}

void print_colour_state(Graph *g) {
    printf("Colour state:\n");
    for (int i=0;i<NNODES;i++) {
        printf("Node %02d: | ", i);
        for (int j=0;j<g->ncolours;j++) {
            if (g->nodes[i].neighbour_colours[j]==0) printf("0 ");
            else printf(". ");
        }
        if (g->node_colours[i] == -1) printf("| X\n");
        else printf("| %d\n", g->node_colours[i]);
    }
}

void print_neighbours(Graph *g) {
    for (int i=0;i<NNODES;i++) {
        printf("Node %d: ", i);
        Node n = g->nodes[i];
        for (int j=0;j<n.nneighbours;j++)
            printf("%d ", n.neighbours[j]->index);
        puts("");
    }
}

void print_output(Graph *g) {
    if (PRINTSTATE) {
        for (int i=0;i<NNODES;i++) {
            for (int j=0;j<NNODES;j++) {
                printf("%02d ", g->colourstate[i][j]);
            }
            puts("");
        }
    } else {
        printf("%d 0\n", g->ncolours);
        for (int i=0;i<NNODES;i++) printf("%d ",g->node_colours[i]);
        puts("");
    }
}

struct Tuple parse_file(char *fpath) {
    FILE *myfile = fopen (fpath,"r");
    if (myfile == NULL) {
        perror ("Error opening file"); //or return 1;
        exit(1);
    }
    fscanf(myfile, "%d %d", &NNODES, &NEDGES);
    int *n1ind = malloc(NEDGES * sizeof(int));
    int *n2ind = malloc(NEDGES * sizeof(int));
    for (int i=0; i<(NEDGES); i++) {
        fscanf(myfile, "%d %d", &n1ind[i], &n2ind[i]);
    }
    fclose(myfile);
    struct Tuple tup = {n1ind, n2ind};
    return tup;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("No file name specified\n");
        exit(1);
    }
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-v") == 0) VERBOSE = true;
        if (strcmp(argv[i],"-d") == 0) DEBUG = true;
        if (strcmp(argv[i],"-r") == 0) DETAILEDRES = true;
        if (strcmp(argv[i],"-s") == 0) PRINTSTATE = true;
    }
    struct Tuple tup = parse_file(argv[1]);
    solve(tup.arr1, tup.arr2);
}
