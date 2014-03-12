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

int cmp_node_nlinks(const void *v1, const void *v2) {
    const Node *i1 = *(const Node **)v1;
    const Node *i2 = *(const Node **)v2;
    return i1->nlinks < i2->nlinks ? -1 : (i1->nlinks > i2->nlinks);
}

int *node_argsort_nlinks(Node *array) {
    Node **parray = malloc(NNODES * sizeof(Node));
    int *position = malloc(NNODES * sizeof(int));
    for(int i = 0; i < NNODES; i++) parray[i] = &array[i];
    qsort(parray, NNODES, sizeof *parray, cmp_node_nlinks);
    for(int i = 0;i < NNODES;i++) position[i] = (parray[i] - array);
    free(parray);
    return position;
}

int *find_most_constrained(Graph *g) {
    int *position = node_argsort_nlinks(g->nodes);
    return position;
}

int cmp_colour_freq(const void *v1, const void *v2) {
    const int *i1 = *(const int **)v1;
    const int *i2 = *(const int **)v2;
    return i1 < i2 ? -1 : (i1 > i2);
}

int *colour_freq_sort(int *array) {
    int **parray = malloc(NNODES * sizeof(int));
    int *position = malloc(NNODES * sizeof(int));
    for(int i = 0; i < NNODES; i++) parray[i] = &array[i];
    qsort(parray, NNODES, sizeof *parray, cmp_colour_freq);
    for(int i = 0;i < NNODES;i++) position[i] = (parray[i] - array);
    free(parray);
    return position;
}

int *find_rarest_colour(Graph *g) {
    //implicitly takes "first" rarest colour
    int *counts = calloc(g->ncolours, sizeof(int));
    for (int i=0;i<NNODES;i++) {
        for (int j=0;j<g->ncolours;j++) {
            if (g->nodes[i].posscolours[j]) counts[j]++;
        }
    }
    int *positions = colour_freq_sort(counts);
    return positions;
}

Graph construct_graph(int nitems, int *n1inds,int *n2inds) {
    Graph g = {0, calloc(NNODES, sizeof(int)), calloc(NNODES,sizeof(Node))};
    for (int i=0;i<NNODES;i++) {
        Node n = { i, 0,
            calloc(NNODES,sizeof(bool)),
            calloc(NNODES,sizeof(Node *))
        };
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
    return true;
}



bool solver(int next_node_ix, int next_node_colour, Graph g, int ncoloured) {
    bool success = set_and_propagate(next_node_ix, next_node_colour, &g);
    if (!success) return false;
    if (ncoloured == NNODES) return true;
    return true;
        

}

void solve(Graph g) {
    int *constrained_order = find_most_constrained(&g);
    solver(constrained_order[0], 0, g, 1);
}

int main() {
    NNODES = 4;
    int n1ind[3] = {0,1,1};
    int n2ind[3] = {1,2,3};
    Graph graph = construct_graph(3, n1ind, n2ind);
    for (int i=0;i<NNODES;i++) {
        printf("Node %d: ", i);
        Node n = graph.nodes[i];
        for (int j=0;j<n.nlinks;j++)
            printf("%d ", n.links[j]->index);
        puts("");
    }
}

