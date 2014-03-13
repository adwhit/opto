//TODO:
//Come up with a decentbacktracking/optimisation algorithm
//Make python ffi
//Implement debug/verbose mode

#include <stdio.h>
#include <stdlib.h>

int NNODES = 0;

typedef enum {false, true} bool;

typedef struct Node Node;

struct Node {
    int index;
    int nlinks;
    int nposscolours;
    bool *posscolours;
    Node **links;
};

typedef struct {
    int ncolours;
    int *node_colours;
    Node *nodes;
} Graph;

typedef struct {
    int itercount;
} SolveState;

void print_colours(Graph *g);
void print_colour_state(Graph *g);

int cmp_node_nlinks(const void *v1, const void *v2) {
    const Node *i1 = *(const Node **)v1;
    const Node *i2 = *(const Node **)v2;
    return i1->nlinks > i2->nlinks ? -1 : (i1->nlinks < i2->nlinks);
}

int *node_argsort_nlinks(Node *array) {
    Node **parray = malloc(NNODES * sizeof(Node));
    int *position = malloc(NNODES * sizeof(int));
    for(int i = 0; i < NNODES; i++) parray[i] = &array[i];
    qsort(parray, NNODES, sizeof *parray, cmp_node_nlinks);
    for(int i = 0;i < NNODES;i++) position[i] = (parray[i] - array);
    return position;
}

int *find_most_constrained(Graph *g) {
    int *position = node_argsort_nlinks(g->nodes);
    /*puts("Node constraint order");*/
    /*for (int i=0;i < NNODES; i++) {*/
        /*printf("index: %d  Rank %d\n", i, position[i]);*/
    /*}*/
    return position;
}

int cmp_colour_freq(const void *v1, const void *v2) {
    const int *i1 = *(const int **)v1;
    const int *i2 = *(const int **)v2;
    return i1 > i2 ? -1 : (i1 < i2);
}

int *colour_freq_sort(int *array, int ncolours) {
    int **parray = malloc(ncolours * sizeof(int));
    int *position = malloc(ncolours * sizeof(int));
    for(int i = 0; i < ncolours; i++) parray[i] = &array[i];
    qsort(parray, ncolours, sizeof *parray, cmp_colour_freq);
    for(int i = 0;i < ncolours;i++) position[i] = (parray[i] - array);
    return position;
}

int *find_rarest_colours(Graph *g) {
    int *counts = calloc(g->ncolours, sizeof(int));
    for (int i=0; i<g->ncolours; i++) {
        for (int j=0; j<g->ncolours; j++) { 
            if (g->nodes[i].posscolours[j]) counts[j]++;
        }
    }
    int *positions = colour_freq_sort(counts, g->ncolours);
    /*puts("color count ranking");*/
    /*for (int i=0;i < g->ncolours; i++) {*/
        /*printf("Index: %d Count: %d  Rank %d\n",i, counts[i], positions[i]);*/
    /*}*/
    return positions;
}

Graph construct_graph(int ncolours, int nitems, int *n1inds,int *n2inds) {
    //initialise graph and nodes. Possible cffi entry point
    Graph g = {ncolours, calloc(NNODES, sizeof(int)), calloc(NNODES,sizeof(Node))};
    for (int i=0;i<NNODES;i++) {
        g.node_colours[i] = -1;
        Node n = { i, 0, g.ncolours,
            calloc(NNODES,sizeof(bool)),
            calloc(NNODES,sizeof(Node *))
        };
        for (int j=0;j<NNODES;j++) n.posscolours[j] = true;
        g.nodes[i] = n;
    };
    for (int i=0;i<nitems;i++) {
        int n1ind = n1inds[i];
        int n2ind = n2inds[i];
        Node *n1 = &g.nodes[n1ind];
        Node *n2 = &g.nodes[n2ind];
        n1->links[n1->nlinks] = n2;
        n2->links[n2->nlinks] = n1;
        n1->nlinks++;
        n2->nlinks++;
    }
    return g;
}

bool set_and_propagate(int next_node_ix, int next_node_colour, Graph *g) {
    printf("Setting node %d to colour %d\n", next_node_ix, next_node_colour);
    //set
    Node node = g->nodes[next_node_ix];
    g->node_colours[next_node_ix] = next_node_colour;
    //propagate - remove colour from nearby nodes
    for (int j=0;j<node.nlinks;j++) {
        Node *linkednode = node.links[j];
        if (linkednode->posscolours[next_node_colour]) {
            linkednode->posscolours[next_node_colour] = false;
            linkednode->nposscolours--;
            if (linkednode->nposscolours <= 0) return false;
        }
    }
    print_colour_state(g);
    return true;
}

void choose_next(int *next_node, int *next_colour, Graph *g) {
    int *constrained_order = find_most_constrained(g);
    int *rarest_colours = find_rarest_colours(g);
    int nodeind = 0;
    int colourind = 0;
    // make sure node hasn't already been set
    while (g->node_colours[constrained_order[nodeind]] != -1) nodeind ++;
    // make sure node colour is possible
    while (!g->nodes[constrained_order[nodeind]].posscolours[rarest_colours[colourind]]) colourind++;
    *next_node = constrained_order[nodeind];
    *next_colour = rarest_colours[colourind];
}

bool solver(int next_node_ix, int next_node_colour, Graph *g, int ncoloured) {
    bool success = set_and_propagate(next_node_ix, next_node_colour, g);
    if (!success) return false;
    else if (ncoloured == NNODES) return true;
    else {
        int next_node;
        int next_colour;
        choose_next(&next_node, &next_colour, g);
        return solver(next_node, next_colour, g, ncoloured +1);
    }
}

bool start_solver(Graph *g) {
    printf("Attempting to solve with %d colours\n", g->ncolours);
    int *constrained_order = find_most_constrained(g);
    bool success = solver(constrained_order[0], 0, g, 1);
    if (success) puts("Success!");
    else puts("Fail");
    return success;
}

void solve(int nitems, int *n1ind, int *n2ind) {
    // TODO: free graph on failure
    int ncolours = 2;
    bool success = false;
    while (!success) {
        Graph graph = construct_graph(ncolours, nitems, n1ind, n2ind);
        success = start_solver(&graph);
        ncolours++;
    }
}


void print_colours(Graph *g) {
    printf("Colours: ");
    for (int i=0;i<NNODES;i++) {
        if (g->node_colours[i]==-1) printf(". ");
        else printf("%d ", g->node_colours[i]);
    }
    puts("");
}

void print_colour_state(Graph *g) {
    printf("Colour state:\n");
    for (int i=0;i<NNODES;i++) {
        printf("Node %d: | ", i);
        for (int j=0;j<g->ncolours;j++) {
            if (g->nodes[i].posscolours[j]) printf("0 ");
            else printf(". ");
        }
        if (g->node_colours[i] == -1) printf("| X\n");
        else printf("| %d\n", g->node_colours[i]);
    }
}

void print_links(Graph *g) {
    for (int i=0;i<NNODES;i++) {
        printf("Node %d: ", i);
        Node n = g->nodes[i];
        for (int j=0;j<n.nlinks;j++)
            printf("%d ", n.links[j]->index);
        puts("");
    }
}

int main() {
    NNODES = 10;
    int n1ind[13] = {0,1,1,1,3,5,6,6,5,9,9,9,9};
    int n2ind[13] = {1,2,3,4,4,6,7,8,4,1,2,3,4};
    solve(13, n1ind, n2ind);
}

