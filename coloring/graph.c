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
    int *neighbour_colours;
    Node **neighbours;
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
int NCOLOURS = 2;
int MAXSEARCHDEPTH = 2;

bool VERBOSE = false;
bool DEBUG = false;
bool DETAILEDRES = false;
bool PRINTSTATE = false;

void print_colours(Graph *g);
void print_colour_state(Graph *g);
void print_output(Graph *g);
void save_colourstate(int iter, Graph *g);
int nposscolours(Node *n);
bool *find_available_colours(Node *n, int from_node_index, int depth);
bool can_change_colour(Node *n, int from_node, int depth);
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

int *find_rarest_colours(Graph *g) {
    //XXX this will now stackoverflow since some nodes have no possible colours
    int *counts = calloc(NCOLOURS, sizeof(int));
    for (int i=0;i<NNODES;i++) {
        for (int j=0;j<NCOLOURS;j++) {
            if (g->nodes[i].neighbour_colours[j]==0) counts[j]++;
        }
    }
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
        free(g->nodes[i].neighbour_colours);
        free(g->nodes[i].neighbours);
    };
    free(g->nodes);
};

bool *find_available_colours(Node *n, int from_node_index, int depth) {
    bool *available_colours = malloc(NCOLOURS * sizeof(bool));
    //set default to true
    for (int i=0;i<NCOLOURS;i++) available_colours[i] = true;
    //set present colour to false
    if (n->colour != -1) available_colours[n->colour] = false;
    for (int i=0;i<n->nneighbours;i++) {
        // make sure to exlude from_node from checks
        Node *nodetocheck = n->neighbours[i];
        if (nodetocheck->index != from_node_index &&
           !can_change_colour(nodetocheck, n->index, depth -1)) 
                available_colours[nodetocheck->colour] = false;
    }
    return available_colours;
}

bool can_change_colour(Node *n, int from_node_index, int depth) {
    //see if a node can change colour immediately
    n->next_colour = -1;
    if (nposscolours(n) > 1) {
        for (int i=0;i<NCOLOURS;i++) {
            if ((i != n->colour) && (n->neighbour_colours[i] == 0)) {
                n->next_colour = i;
                return true;
            }
        }
        puts("FAIL");
    }
    if (depth <= 0) {
        return false;
    }
    //see if children can change
    //Assume all colours are available except node's current colour
    bool *available_colours = find_available_colours(n, from_node_index, depth-1);
    for (int i=0;i<NCOLOURS;i++) {
        if (available_colours[i]) {
            n->next_colour = i;
            return true;
        }
    }
    return false;
}

int nposscolours(Node *n) {
    int nposs = 0;
    for (int i=0;i<NCOLOURS;i++) {
        if (!n->neighbour_colours[i]) nposs++;
    }
    return nposs;
}

bool change_and_propagate(Node *n, Graph *g) {
    int nposs = nposscolours(n);
    if ((n->colour==-1 && nposs == 0)
        || (n->colour != -1 && nposs == 1)) {
        //ask further along chain
        for (int i=0;i<n->nneighbours;i++) {
            if (n->neighbours[i]->colour == n->next_colour) {
                change_and_propagate(n->neighbours[i], g);
            }
        }
    }
    //make change!
    for (int i=0;i<n->nneighbours;i++) {
        n->neighbours[i]->neighbour_colours[n->next_colour]++;
        n->neighbours[i]->neighbour_colours[n->colour]--;
    }
    n->colour = n->next_colour;
    n->next_colour = -1;
    return true;
}

void set_and_propagate(Node *next_node, int next_node_colour, Graph *g) {
    if (next_node_colour == -1) {  
        // no free colours - try to change surrounding nodes
        if (VERBOSE) printf("Attempting to change neighbours of %d\n",next_node->index);
        if (can_change_colour(next_node, -1, MAXSEARCHDEPTH)) {
            if (VERBOSE)  {
                printf("Propagating change from node %d\n", next_node->index);
                print_colour_state(g);
            }
            change_and_propagate(next_node, g);
            return;
        } else {
            //increase ncolours
            NCOLOURS++;
            if (VERBOSE) printf("Failed - incresing ncolours to %d\n", NCOLOURS);
            next_node_colour = NCOLOURS - 1;
        }
    }
    if (VERBOSE) printf("Setting node %d to colour %d\n", next_node->index, next_node_colour);
    //set
    next_node->colour = next_node_colour;
    //propagate - remove colour from nearby nodes
    for (int j=0;j<next_node->nneighbours;j++) {
        next_node->neighbours[j]->neighbour_colours[next_node_colour]++;
    }
}

void choose_next(Node **next_node, int *next_colour, Graph *g) {
    int colourind = 0;
    // choose next node
    if (DEBUG) puts("Finding next node");
    int *constrained_order = find_most_constrained(g);
    *next_node = &g->nodes[constrained_order[0]];
    //choose  next colour
    if (DEBUG) puts("Finding next colour");
    // make sure node colour is possible - if not, return (colour is set to default of -1)
    if (nposscolours(*next_node) <= 0) return;
    int *rarest_colours = find_rarest_colours(g);
    while ((*next_node)->neighbour_colours[rarest_colours[colourind]]!=0) {
        colourind++;
    }
    *next_colour = rarest_colours[colourind];
    free(constrained_order);
    free(rarest_colours);
}

void solver(Node *next_node, int next_node_colour, Graph *g, int ncoloured) {
    set_and_propagate(next_node, next_node_colour, g);
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

void solve(int *n1ind, int *n2ind) {
    // TODO: free graph on failure
    Graph graph = construct_graph(n1ind, n2ind);
    Node *firstnode = &graph.nodes[find_most_constrained(&graph)[0]];
    solver(firstnode, 0, &graph, 1);
    if (DETAILEDRES) print_colour_state(&graph);
    print_output(&graph);
    if (!check_valid(&graph)) puts("ERROR INVALID SOLUTION");
}

void save_colourstate(int ncoloured, Graph *g) {
    for (int j=0;j<NNODES;j++) {
        g->colourstate[ncoloured-1][j] = g->nodes[j].colour;
    }
}

void print_colour_state(Graph *g) {
    printf("Colour state:\n");
    for (int i=0;i<NNODES;i++) {
        printf("Node %02d: | ", i);
        for (int j=0;j<NCOLOURS;j++) {
            if (g->nodes[i].neighbour_colours[j]==0) printf("0 ");
            else printf(". ");
        }
        if (g->nodes[i].colour == -1) printf("| X ");
        else printf("| %d ", g->nodes[i].colour);
        if (g->nodes[i].next_colour == -1) printf("| X\n");
        else printf("| %d\n", g->nodes[i].next_colour);
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
        printf("%d 0\n", NCOLOURS);
        for (int i=0;i<NNODES;i++) printf("%d ",g->nodes[i].colour);
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

bool check_valid(Graph *g) {
    for (int i=0;i<NNODES;i++) {
        Node n = g->nodes[i];
        for (int j=0;j<n.nneighbours;j++) {
            if (n.colour == n.neighbours[j]->colour) {
                return false;
            }
        }
    }
    return true;
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
