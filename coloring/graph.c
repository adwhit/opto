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
    int *n1arr;
    int *n2arr;
    int nnodes;
    int nedges;
};

struct Node {
    int index;
    int colour;
    int next_colour;             //for propagation
    int nneighbours;
    int *neighbour_colours;
    Node **neighbours;
};

struct Result {
    int ncolours;
    int nnodes;
    int *colours;
};

typedef struct {
    Node *nodes;
    int **colourstate;
} Graph;

typedef struct {
    int itercount;
} SolveState;

int NNODES = 0;
int NEDGES = 0;
int NCOLOURS = 1;
int MAXSEARCHDEPTH = 1;

bool VERBOSE = false;
bool DEBUG = false;
bool DETAILEDRES = false;
bool PRINTSTATE = false;

void print_colours(Graph *g);
void print_colour_state(Graph *g);
void print_output(struct Result res);
void save_colourstate(int iter, Graph *g);
int nposscolours(Node *n);
void print_state(Graph *g);
void print_neighbours(Graph *g);
void print_kemp_chain(int *chain);
bool check_valid(Graph *g);

int cmp_node_nneighbours(const void *v1, const void *v2) {
    const Node *i1 = *(const Node **)v1;
    const Node *i2 = *(const Node **)v2;
    //already-assigned nodes should move to the right
    if (i1->colour != -1) return 1;
    if (i2->colour != -1) return -1;
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

Node *choose_next_node(Graph *g) {
    /*for (int i=0;i<NNODES;i++) {
        if (g->nodes[i].colour == -1) return &g->nodes[i];
    }*/
    int *position = node_argsort_nneighbours(g->nodes);
    return &g->nodes[position[0]];
}

int cmp_colour_freq(const void *v1, const void *v2) {
    const int i1 = **(const int **)v1;
    const int i2 = **(const int **)v2;
    if (i1 < i2) return -1;
    if (i1 > i2) return 1;
    return 0;
}

int *colour_freq_sort(int *array) {
    int **parray = malloc(NCOLOURS * sizeof(int *));
    int *position = malloc(NCOLOURS * sizeof(int));
    for(int i = 0; i < NCOLOURS; i++) parray[i] = &array[i];
    qsort(parray, NCOLOURS, sizeof(int *), cmp_colour_freq);
    if (DEBUG) for (int i=0;i<NCOLOURS;i++) {
        printf("after parray val %p, array val %p position %li\n", parray[i], array, parray[i] - array);
    }
    for(int i = 0;i < NCOLOURS;i++) position[i] = (parray[i] - array);
    free(parray);
    return position;
}

int *colour_counts(Graph *g) {
    int *counts = calloc(NCOLOURS, sizeof(int));
    for (int i=0;i<NNODES;i++) {
        if (g->nodes[i].colour != -1) counts[g->nodes[i].colour]++;
    }
    return counts;
}

int *free_colour_counts(Graph *g) {
    int *counts = calloc(NCOLOURS, sizeof(int));
    for (int i=0;i<NNODES;i++) {
        for (int j=0;j<NCOLOURS;j++) {
            if (g->nodes[i].neighbour_colours[j]==0) counts[j]++;
        }
    }
    return counts;
}

int *find_rarest_colours(Graph *g) {
    int *counts = free_colour_counts(g);
    int *positions = colour_freq_sort(counts);
    if (DEBUG) {
        puts("color count ranking");
        for (int i=0;i <NCOLOURS; i++) {
            printf("Rank: %d Index: %d  Counts %d\n",i, positions[i], counts[positions[i]]);
        }
    }
    free(counts);
    return positions;
}

Graph construct_graph(int *n1inds,int *n2inds) {
    //initialise graph and nodes. Possible cffi entry point
    Graph g = {
        calloc(NNODES,sizeof(Node)),                //Node list
        malloc(NNODES * sizeof(int *))};            //Colour state
    for (int i=0;i<NNODES;i++) {
        g.nodes[i].colour = -1;
        g.colourstate[i] = malloc(NNODES * sizeof(int));
        Node n = { 
            i,                                      //index
            -1,                                     //colour
            -1,                                     //next colour
            0,                                      //number neighbours
            calloc(NNODES,sizeof(int)),             //neighbour colour counts
            calloc(NNODES,sizeof(Node *))           //neighbours
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
    for (int i=0;i<NNODES;i++) {
        free(g->nodes[i].neighbours);
        free(g->nodes[i].neighbour_colours);
    };
    free(g->nodes);
};


int nposscolours(Node *n) {
    int nposs = 0;
    for (int i=0;i<NCOLOURS;i++) {
        if (n->neighbour_colours[i] <= 0) nposs++;
    }
    return nposs;
}

void find_kemp_chain(Node *n, int othercolour, int *chain) {
    chain[n->index] = n->colour;
    n->next_colour = othercolour;
    for (int i=0;i<n->nneighbours;i++) {
        if (n->neighbours[i]->colour == othercolour && chain[n->neighbours[i]->index] == -1) {
            find_kemp_chain(n->neighbours[i], n->colour, chain);
        }
    }
}

void update_node_colour(Node *n) {
    if (VERBOSE) printf("Setting node %d to colour %d\n", n->index, n->next_colour);
    for (int i=0;i<n->nneighbours;i++) {
        n->neighbours[i]->neighbour_colours[n->next_colour]++;
        if (n->colour != -1) {
            n->neighbours[i]->neighbour_colours[n->colour]--;
        }
    }
    n->colour = n->next_colour;
}

bool can_change_colour(Node *n) {
    for (int i=0;i<NCOLOURS;i++) {
        if (i!=n->colour && !n->neighbour_colours[i]) {
            n->next_colour = i;
            return true;
        }
    }
    return false;
}

void reset_next_colour(Graph *g) {
    for (int i=0;i<NNODES;i++) g->nodes[i].next_colour = -1;
}

void set_and_propagate(Node *n, int next_colour, Graph *g) {
    if (nposscolours(n) > 0) {
        // if can add immediately, do so
        n->next_colour = next_colour;
        update_node_colour(n);
        return;
    }
    // no free colours - try to change surrounding nodes
    if (VERBOSE) printf("Attempting to change neighbours of %d\n",n->index);
    bool *available_colours = malloc(NCOLOURS * sizeof(bool));
    //set default to true
    for (int i=0;i<NCOLOURS;i++) available_colours[i] = true;
    for (int i=0;i<n->nneighbours;i++) {
        if ((n->neighbours[i]->colour != -1) && (!can_change_colour(n->neighbours[i]))) {
            available_colours[n->neighbours[i]->colour] = false;
        }
    }
    for (int i=0;i<NCOLOURS;i++) {
        if (available_colours[i]) {
            //success - propagate change to each neighbour with chosen colour
            n->next_colour = i;
            for (int j=0;j<n->nneighbours;j++) {
                if (n->neighbours[j]->colour == i)  update_node_colour(n->neighbours[j]);
            }
            update_node_colour(n);
            return;
        }
    }
    //no luck, so increase ncolours
    NCOLOURS++;
    if (VERBOSE) printf("Failed - incresing ncolours to %d\n", NCOLOURS);
    n->next_colour = NCOLOURS - 1;
    update_node_colour(n);
}

void choose_next(Node **next_node, int *next_colour, Graph *g) {
    int colourind = 0;
    // choose next node
    if (DEBUG) puts("Finding next node");
    *next_node = choose_next_node(g);
    //choose  next colour
    if (DEBUG) puts("Finding next colour");
    // make sure node colour is possible - if not, return (colour is set to default of -1)
    if (nposscolours(*next_node) <= 0) return;
    int *rarest_colours = find_rarest_colours(g);
    while ((*next_node)->neighbour_colours[rarest_colours[colourind]]!=0) {
        colourind++;
    }
    *next_colour = rarest_colours[colourind];
    free(rarest_colours);
}

void solver(Node *next_node, int next_node_colour, Graph *g, int ncoloured) {
    set_and_propagate(next_node, next_node_colour, g);
    reset_next_colour(g);
    save_colourstate(ncoloured, g);
    if (DEBUG) print_colour_state(g);
    if (ncoloured == NNODES) return;
    else {
        Node *next_node;
        int next_colour = -1;
        choose_next(&next_node, &next_colour, g);
        solver(next_node, next_colour, g, ncoloured +1);
    }
    return;
}

struct Result solve(int *n1arr, int *n2arr, int nnodes, int nedges) {
    NNODES = nnodes; NEDGES = nedges;
    Graph graph = construct_graph(n1arr, n2arr);
    Node *firstnode = choose_next_node(&graph);
    solver(firstnode, 0, &graph, 1);
    if (DETAILEDRES) print_colour_state(&graph);
    if (PRINTSTATE) print_state(&graph);
    if (!check_valid(&graph)) puts("ERROR INVALID SOLUTION");
    int *colours = malloc(NNODES * sizeof(int));
    for (int i=0;i<NNODES;i++) {
        colours[i] = graph.nodes[i].colour;
    }

    //Kemp chain thing
    int *chain= malloc(NNODES * sizeof(int));
    for (int i=0;i<NNODES;i++) chain[i] = -1;
    int othercolour= 8;
    int initnode = 0;
    find_kemp_chain(&graph.nodes[initnode], othercolour, chain);
    print_kemp_chain(chain);






    struct Result res = {NCOLOURS, NNODES, colours};
    free_graph(&graph);
    return res;
}

void save_colourstate(int ncoloured, Graph *g) {
    for (int j=0;j<NNODES;j++) {
        g->colourstate[ncoloured-1][j] = g->nodes[j].colour;
    }
}

void print_colour_state(Graph *g) {
    int *counts = colour_counts(g);
    printf("Colours: ");
    for (int i=0;i<NCOLOURS;i++) printf("%02d ", i);
    printf("\nCCounts: ");
    for (int i=0;i<NCOLOURS;i++) printf("%02d ", counts[i]);
    puts("");
    printf("Colour state:\n");
    for (int i=0;i<NNODES;i++) {
        printf("Node %02d: | ", i);
        for (int j=0;j<NCOLOURS;j++) {
            if (g->nodes[i].colour == -1) printf("_ ");
            else if (g->nodes[i].neighbour_colours[j]==0) printf("0 ");
            else if (g->nodes[i].neighbour_colours[j]==1) printf("1 ");
            else printf(". ");
        }
        if (g->nodes[i].colour == -1) printf("|XX");
        else printf("|%02d", g->nodes[i].colour);
        if (g->nodes[i].next_colour == -1) printf("|\n");
        else printf("|%02d|\n", g->nodes[i].next_colour);
    }
}

void print_kemp_chain(int *chain) {
    for (int i=0;i<NNODES;i++) {
        printf("%02d ", i);
    }
    puts("");
    for (int i=0;i<NNODES;i++) {
        if (chain[i] != -1) printf("%02d ",chain[i]);
        else printf(".  ");
    }
    puts("");
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

void print_state(Graph *g) {
    for (int i=0;i<NNODES;i++) {
        for (int j=0;j<NNODES;j++) {
            printf("%02d ", g->colourstate[i][j]);
        }
        puts("");
    }
}

void print_output(struct Result res) {
    printf("%d 0\n", NCOLOURS);
    for (int i=0;i<NNODES;i++) printf("%d ",res.colours[i]);
    puts("");
}

struct Tuple parse_file(char *fpath) {
    FILE *myfile = fopen (fpath,"r");
    if (myfile == NULL) {
        perror ("Error opening file"); //or return 1;
        exit(1);
    }
    int nnodes; int nedges;
    fscanf(myfile, "%d %d", &nnodes, &nedges);
    int *n1ind = malloc(nedges * sizeof(int));
    int *n2ind = malloc(nedges * sizeof(int));
    for (int i=0; i<(nedges); i++) {
        fscanf(myfile, "%d %d", &n1ind[i], &n2ind[i]);
    }
    fclose(myfile);
    struct Tuple tup = {n1ind, n2ind, nnodes, nedges};
    return tup;
}

bool check_valid(Graph *g) {
    bool rtn = true;
    for (int i=0;i<NNODES;i++) {
        Node n = g->nodes[i];
        for (int j=0;j<n.nneighbours;j++) {
            if (n.colour == -1 || (n.colour == n.neighbours[j]->colour)) {
                printf("Error: nodes %d and %d are both colour %d\n", n.index, n.neighbours[j]->index, n.colour);
                rtn = false;
            }
        }
    }
    return rtn;
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
    struct Result res = solve(tup.n1arr, tup.n2arr, tup.nnodes, tup.nedges);
    print_output(res);
}
