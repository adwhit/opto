#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int NNODES;
int NGENES;

typedef struct {
    uint8_t *bytes;
    double score;
} Gene;

typedef struct {
    double x;
    double y;
} Node;

struct HoF {
    Gene g;
    struct HoF *next;
};

typedef enum {false, true} bool;

void print_pos(int *pos);

int cmp_doubles(const void *v1, const void *v2) {
    const double i1 = **(const double **)v1;
    const double i2 = **(const double **)v2;
    if (i1 < i2) return -1;
    if (i1 > i2) return 1;
    return 0;
}

int cmp_genes(const void *v1, const void *v2) {
    const Gene *g1 = (const Gene *)v1;
    const Gene *g2 = (const Gene *)v2;
    if (g1->score < g2->score) return -1;
    if (g1->score > g2->score) return 1;
    return 0;
}

int cmp_bytes(const void *v1, const void *v2) {
    const uint8_t i1 = **(const uint8_t **)v1;
    const uint8_t i2 = **(const uint8_t **)v2;
    if (i1 < i2) return -1;
    if (i1 > i2) return 1;
    return 0;
}

void bytes_argsort(uint8_t *array, int arraysize, int *position) {
    uint8_t **parray = malloc(arraysize * sizeof(uint8_t *));
    for(int i = 0; i<arraysize; i++) parray[i] = &array[i];
    qsort(parray, arraysize, sizeof(uint8_t *), cmp_bytes);
    for(int i = 0;i<arraysize;i++) position[i] = (parray[i] - array);
    free(parray);
}

void doubles_argsort(double *array, int arraysize, int *position) {
    double **parray = malloc(arraysize * sizeof(double *));
    for(int i = 0; i<arraysize; i++) parray[i] = &array[i];
    qsort(parray, arraysize, sizeof(double *), cmp_doubles);
    for(int i = 0;i<arraysize;i++) position[i] = (parray[i] - array);
    free(parray);
}

double dist(Node n1, Node n2) {
    return sqrt((n1.x-n2.x)*(n1.x-n2.x) + (n1.y-n2.y)*(n1.y-n2.y));
}

void score(Gene *g, Node *nodes) {
    //sort everything
    int *position = malloc(NNODES * sizeof *position);
    bytes_argsort(g->bytes,NNODES,position);
    //accumulate total
    double total = 0;
    for (int i=0;i<NNODES-1;i++)
        total += dist(nodes[position[i]], nodes[position[i+1]]);
    total += dist(nodes[position[NNODES-1]], nodes[position[0]]);
    g->score = total;
    free(position);
}

void score_and_sort(Gene *population, Node *nodes) {
    for (int i=0;i<NGENES;i++) {
        score(&population[i], nodes);
    }
    qsort(population, NGENES, sizeof *population, cmp_genes);
}

void gen_grid(int x, int y, int gap, Node *nodes) {
    for (int i=0;i<x;i++) {
        for (int j=0;j<y;j++) {
            nodes[y*i + j] = (Node){i*gap,j*gap};
        }
    }
}

Gene random_gene() {
    Gene g = (Gene){malloc(NNODES*sizeof(uint8_t)), 0};
    for (int i=0;i<NNODES;i++)
        g.bytes[i] = (uint8_t)rand();
    return g;
}

Gene zero_gene() {
    return (Gene){calloc(NNODES, sizeof(uint8_t)), 0};
}

Gene clone(Gene *g) {
    Gene clone = (Gene){malloc(NNODES * sizeof(uint8_t)), g->score};
    for (int i=0;i<NNODES;i++) 
        clone.bytes[i] = g->bytes[i];
    return clone;
}

Gene bit_cross(Gene *g1, Gene *g2) {
    Gene g = (Gene){malloc(NNODES * sizeof(uint8_t)),0};
    uint8_t b;
    for (int i=0;i<NNODES;i++) {
        b = (uint8_t)rand();
        g.bytes[i] = (g1->bytes[i] & b) | (g2->bytes[i] & !b);
    }
    return g;
}

Gene byte_cross(Gene *g1, Gene *g2) {
    Gene g = (Gene){malloc(NNODES * sizeof(uint8_t)),0};
    for (int i=0;i<NNODES;i++) {
        if (rand()%2) {
            g.bytes[i] = g1->bytes[i];
        } else {
            g.bytes[i] = g2->bytes[i];
        }
    }
    return g;
}

Gene mutate(Gene *g1, double p) {
    Gene g = clone(g1);
    for (int i=0;i<NNODES;i++) {
        if ((double)rand()/(double)RAND_MAX < p) {
            g.bytes[i] = g.bytes[i] ^ (uint8_t)rand();
        }
    }
    return g;
}

void gen_genes(Gene *genes, int n) {
    for (int i=0;i<n;i++) {
        genes[i] = random_gene();
    }
}

void print_gene(Gene *g) {
    /*for (int i=0;i<NNODES;i++) {*/
        /*printf("%d ", g->bytes[i]);*/
    /*}*/
    printf("Score: %0.2f\n", g->score);
}

void print_population(Gene *genes) {
    for (int i=0;i<NGENES;i++) {
        print_gene(&genes[i]);
    }
}

void print_population_stats(Gene *population, Gene *hof, int generation) {
    double scr = population[0].score;
    double max = scr;
    double min = scr;
    double sigma_x = scr;
    double sigma_xsqr = scr*scr;
    for (int i=1;i<NGENES;i++) {
        scr = population[i].score;
        if (scr > max) max = scr;
        if (scr < max) max = scr;
        sigma_x += scr;
        sigma_xsqr += scr*scr;
    }
    double std = sqrt(sigma_xsqr/NGENES - (sigma_x/NGENES * sigma_x/NGENES));
    printf("--- Generation %d ---\n", generation);
    printf("Max: %.2f\nMin: %.2f\nMean: %.2f\nStd: %.2f\nBest: %.2f\n",
            max, min, sigma_x/NGENES, std, hof[0].score);
}

void print_pos(int *pos) {
    for (int i=0;i<NNODES;i++) {
        printf("%d ", pos[i]);
    }
    puts("");
}

void free_population(Gene *population) {
    for (int i=0;i<NGENES;i++) {
        free(population[i].bytes);
    }
    free(population);
}

Gene *step(Gene *population) {
    //evolutionary step. Create new genes according to some strategy
    //free the old ones
    double mutate_prob = 0.05;
    Gene *genes = malloc(NGENES*sizeof *genes);
    for (int i=0;i<NGENES;i++) {
        if (i<NGENES/2) {
            genes[i] = mutate(&population[i%(NGENES/2)],mutate_prob);
        } else {
            int gi1 = rand() % (NGENES/2);
            int gi2 = rand() % (NGENES/2);
            genes[i] = byte_cross(&population[gi1], &population[gi2]);
        }
    }
    free_population(population);
    return genes;
}

bool is_same(Gene *g1, Gene *g2) {
}

bool add_to_hof(Gene g, Gene *hof) {
    int hofsize = 10;
    for (int i=0;i<hofsize;i++) {
        if (g.score < hof[i].score) {
            free(hof[hofsize-1].bytes);
            for (int j=hofsize-1;j>i;j--) {
                hof[j-1] = hof[j];
            }
            hof[i] = clone(&g);
            return true;
        }
    }
    return false;
}

void evolve(Gene *population, Node *nodes) {
    int ngens = 10000;
    Gene *hof = malloc(10*sizeof *hof);
    //init stuff
    score_and_sort(population, nodes);
    for (int i=0;i<10;i++) {
        hof[i] = clone(&population[i]);
        printf("%.2f %d\n", hof[i].score, i);
    }
    print_population_stats(population, hof, 0);
    //evolve
    for (int i=0;i<ngens;i++) {
        population = step(population);
        score_and_sort(population, nodes);
        int j=0;
        bool added=true;
        while (added) {
            added = add_to_hof(population[j],hof);
            j++;
        }
        print_population_stats(population, hof, i+1);
    }
}

void init() {
    //srand(time(NULL));   // random seed
    int x=10;
    int y=10;
    int gap=10;
    NNODES = x*y;
    Node *nodes = malloc(NNODES*sizeof *nodes);
    gen_grid(x, y, gap, nodes);
    NGENES = 2000;
    Gene *genes = malloc(NGENES*sizeof *genes);
    gen_genes(genes, NGENES);
    evolve(genes, nodes);
}

int main(int argc, char*argv[]) {
    init();
}
