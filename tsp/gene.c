#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int NNODES;
int NGENES;

typedef struct {
    int nbytes;
    uint8_t *bytes;
} Gene;

typedef struct {
    double x;
    double y;
} Node;

typedef enum {false, true} bool;

void print_pos(int *pos);

void setbit(Gene *g, int n) {
}

void getbit(Gene *g, int nth) {

}

int cmp_bytes(const void *v1, const void *v2) {
    const uint8_t i1 = **(const uint8_t **)v1;
    const uint8_t i2 = **(const uint8_t **)v2;
    if (i1 < i2) return -1;
    if (i1 > i2) return 1;
    return 0;
}

void bytes_argsort(uint8_t *array, int *position) {
    uint8_t **parray = malloc(NNODES * sizeof(uint8_t *));
    for(int i = 0; i<NNODES; i++) parray[i] = &array[i];
    qsort(parray, NNODES, sizeof(uint8_t *), cmp_bytes);
    for(int i = 0;i<NNODES;i++) position[i] = (parray[i] - array);
    free(parray);
}

double dist(Node n1, Node n2) {
    //printf("%f %f %f %f %f\n",n1.x,n2.x,n1.y,n2.y,sqrt((n1.x-n2.x)*(n1.x-n2.x) + (n1.y-n2.y)*(n1.y-n2.y)));
    return sqrt((n1.x-n2.x)*(n1.x-n2.x) + (n1.y-n2.y)*(n1.y-n2.y));
}

double score(Gene *g, Node *nodes) {
    //sort everything
    int *position = malloc(NNODES * sizeof *position);
    bytes_argsort(g->bytes,position);
    print_pos(position);
    double total = 0;
    for (int i=0;i<NNODES-1;i++) {
        total += dist(nodes[position[i]], nodes[position[i+1]]);
    }
    total += dist(nodes[position[NNODES-1]], nodes[position[0]]);
    return total;
}

void make_grid(int x, int y, int gap, Node *nodes) {
    for (int i=0;i<x;i++) {
        for (int j=0;j<y;j++) {
            nodes[y*i + j] = (Node){i*gap,j*gap};
        }
    }
}

Gene random_gene() {
    Gene g;
    g.bytes = malloc(NNODES*sizeof(uint8_t));
    for (int i=0;i<NNODES;i++) {
        g.bytes[i] = (uint8_t)rand();
    }
    return g;
}

Gene zero_gene() {
    Gene g;
    g.bytes = calloc(NNODES, sizeof(uint8_t));
    return g;
}

void print_gene(Gene *g) {
    for (int i=0;i<NNODES;i++) {
        printf("%d ", g->bytes[i]);
    }
    puts("");
}

void print_pos(int *pos) {
    for (int i=0;i<NNODES;i++) {
        printf("%d ", pos[i]);
    }
    puts("");
}

int main(int argc, char*argv[]) {
    int x=10;
    int y=10;
    NNODES = x*y;    //assume always leave from zeroth node
    NGENES = 2;
    int gap=10;
    Node *nodes = malloc(NNODES*sizeof *nodes);
    make_grid(x, y, gap, nodes);
    Gene *genes = malloc(NGENES*sizeof *genes);
    genes[0] = random_gene();
    genes[1] = zero_gene();
    print_gene(&genes[0]);
    print_gene(&genes[1]);
    printf("Score: %0.2f\n", score(&genes[0], nodes));
    printf("Score: %0.2f\n", score(&genes[1], nodes));
}
