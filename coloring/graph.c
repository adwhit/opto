//strategy: the colours constraint is returning the list in
//the wrong order - fix
//Also there is no copying etc going on yet, fix also
//No ffi, that can be last?
//Debug/verbose mode
//Come up with some decent backtracking rules
//Code is reasonably modular and logical but could be improved

#include <stdio.h>
#include <stdlib.h>

int NNODES = 0;

typedef enum {false, true} bool;

typedef struct Node Node;

struct Node {
    int index;
    int nlinks;
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

void print_arr(int *arr, int len) {
    for (int i=0;i<len;i++) {
        printf("%d ",arr[i]);
    }
    printf("\n");
}

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
    puts("Node constraint order");
    for (int i=0;i < NNODES; i++) {
        printf("index: %d  Rank %d\n", i, position[i]);
    }
    return position;
}

int cmp_colour_freq(const void *v1, const void *v2) {
    const int *i1 = *(const int **)v1;
    const int *i2 = *(const int **)v2;
    return i1 < i2 ? -1 : (i1 > i2);
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
    for (int i=0;i<g->ncolours;i++) {
        for (int j=0;j<g->ncolours;j++) {
            if (g->nodes[i].posscolours[j]) counts[j]++;
        }
    }
    int *positions = colour_freq_sort(counts, g->ncolours);
    puts("color count ranking");
    for (int i=0;i < g->ncolours; i++) {
        printf("Index: %d Count: %d  Rank %d\n",i, counts[i], positions[i]);
    }
    return positions;
}

Graph construct_graph(int nitems, int *n1inds,int *n2inds) {
    Graph g = {4, calloc(NNODES, sizeof(int)), calloc(NNODES,sizeof(Node))};
    for (int i=0;i<NNODES;i++) {
        g.node_colours[i] = -1;
        Node n = { i, 0,
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
    Node node = g->nodes[next_node_ix];
    //set
    for (int i=0;i<g->ncolours;i++) {
        if (!(next_node_colour == i)) node.posscolours[i] = false;
    }
    g->node_colours[next_node_ix] = next_node_colour;
    //propagate
    for (int j=0;j<node.nlinks;j++) {
        Node *linkednode = node.links[j];
        linkednode->posscolours[next_node_colour] = false;
    }
    print_colours(g);
    return true;
}

void choose_next(int *next_node, int *next_colour, Graph *g) {
    int *constrained_order = find_most_constrained(g);
    int *rarest_colours = find_rarest_colours(g);
    int nodeind = 0;
    int colourind = 0;
    // make sure node hasn't already been set
    while (g->node_colours[constrained_order[nodeind]] != -1){ nodeind ++; }
    // make sure node colour is possible
    while (!g->nodes[nodeind].posscolours[rarest_colours[colourind]]) {
        colourind++;
        printf("colorind %d\n", colourind);
    }
    *next_node = constrained_order[nodeind];
    *next_colour = rarest_colours[colourind];
}

bool solver(int next_node_ix, int next_node_colour, Graph *g, int ncoloured) {
    bool success = set_and_propagate(next_node_ix, next_node_colour, g);
    if (!success) return false;
    if (ncoloured == NNODES) return true;
    int next_node;
    int next_colour;
    choose_next(&next_node, &next_colour, g);
    solver(next_node, next_colour, g, ncoloured +1);
    return false;
}

void solve(Graph *g) {
    int *constrained_order = find_most_constrained(g);
    printf("most constrained: %d \n", constrained_order[0]);
    solver(constrained_order[0], 0, g, 1);
}

void print_colours(Graph *g) {
    printf("Colours: ");
    for (int i=0;i<NNODES;i++) {
        if (g->node_colours[i]==-1) {
            printf(". ");
        } else {
            printf("%d ", g->node_colours[i]);
        }
    }
    puts("");
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
    NNODES = 4;
    int n1ind[3] = {0,1,1};
    int n2ind[3] = {1,2,3};
    Graph graph = construct_graph(3, n1ind, n2ind);
    print_links(&graph);
    print_colours(&graph);
    solve(&graph);
    print_colours(&graph);
}

