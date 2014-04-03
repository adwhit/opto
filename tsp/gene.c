#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <libconfig.h>

#define NORM 4

int NNODES;
int NGENES;
int NGENS;
int HOFSIZE;
int VERBOSE;
int SNAPSHOTS;
int NORMALIZER;
double MUTATE_PROB;
double MUTATE_FRAC;
double CROSS_FRAC;
double SWAP_FRAC;
double TAKE_FRAC;
int TAKE_BEST;

typedef struct {
    short *bytes;
    double score;
} Gene;

typedef struct {
    double x;
    double y;
} Node;

typedef enum {false, true} bool;

void print_pos(int *pos);
int *pyffi(int nnodes, double *xs, double *ys, int verbose, int snapshots);

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
    const short i1 = **(const short **)v1;
    const short i2 = **(const short **)v2;
    if (i1 /16 < i2 /16) return -1;
    if (i1 /16 > i2 /16) return 1;
    return 0;
}

void bytes_argsort(short *array, int arraysize, int *position) {
    short **parray = malloc(arraysize * sizeof(short *));
    for(int i = 0; i<arraysize; i++) parray[i] = &array[i];
    qsort(parray, arraysize, sizeof(short *), cmp_bytes);
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
    Gene g = (Gene){malloc(NNODES*sizeof(short)), 0};
    for (int i=0;i<NNODES;i++)
        g.bytes[i] = (short)rand();
    return g;
}

Gene zero_gene() {
    return (Gene){calloc(NNODES, sizeof(short)), 0};
}

Gene cloner(Gene *g) {
    Gene clone = (Gene){malloc(NNODES * sizeof(short)), g->score};
    for (int i=0;i<NNODES;i++) 
        clone.bytes[i] = g->bytes[i];
    return clone;
}

Gene bit_cross(Gene *g1, Gene *g2) {
    Gene g = (Gene){malloc(NNODES * sizeof(short)),0};
    short b;
    for (int i=0;i<NNODES;i++) {
        b = (short)rand();
        g.bytes[i] = (g1->bytes[i] & b) | (g2->bytes[i] & !b);
    }
    return g;
}

Gene byte_cross(Gene *g1, Gene *g2) {
    Gene g = (Gene){malloc(NNODES * sizeof(short)),0};
    for (int i=0;i<NNODES;i++) {
        if (rand()%2) {
            g.bytes[i] = g1->bytes[i];
        } else {
            g.bytes[i] = g2->bytes[i];
        }
    }
    return g;
}

Gene bit_mutate(Gene *g1, double p) {
    Gene g = cloner(g1);
    for (int i=0;i<NNODES;i++) {
        if ((double)rand()/(double)RAND_MAX < p) {
            g.bytes[i] = g.bytes[i] ^ (short)rand();
        }
    }
    return g;
}

Gene byte_mutate(Gene *g1, double p) {
    Gene g = cloner(g1);
    for (int i=0;i<NNODES;i++) {
        if ((double)rand()/(double)RAND_MAX < p) {
            g.bytes[i] = (short)rand();
        }
    }
    return g;
}

Gene byte_swap(Gene *g1, double p) {
    Gene g = cloner(g1);
    short b1;
    int b2ix;
    for (int i=0;i<NNODES;i++) {
        if ((double)rand()/(double)RAND_MAX < p) {
            b1 = g.bytes[i];
            b2ix = rand() % NNODES;
            g.bytes[i] = g.bytes[b2ix];
            g.bytes[b2ix] = b1;
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
    for (int i=0;i<NNODES;i++) {
        printf("%d,", g->bytes[i]);
    }
    printf("\nScore: %f\n", g->score);
}

void print_result(Gene *g) {
    int *position = malloc(NNODES * sizeof *position);
    bytes_argsort(g->bytes,NNODES,position);
    printf("res = [");
    for (int i=0;i<NNODES;i++) {
        printf("%d,",position[i]);
        if ((i+1)%15 == 0) puts("");
    }
    puts("]");
    printf("Score: %.2f\n", g->score);
    free(position);
}

void print_snapshot(Gene *g) {
    int *position = malloc(NNODES * sizeof *position);
    bytes_argsort(g->bytes,NNODES,position);
    for (int i=0;i<NNODES-1;i++) {
        printf("%d ",position[i]);
    }
    printf("%d\n",position[NNODES-1]);
    printf("%.2f\n", g->score);
    free(position);
}

void print_population(Gene *genes, int ngenes) {
    printf("---Population---\n");
    for (int i=0;i<ngenes;i++) {
        printf("Gene %d:\n", i);
        print_result(&genes[i]);
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

void free_population(Gene *population, int popsize) {
    for (int i=0;i<popsize;i++) {
        free(population[i].bytes);
    }
    free(population);
}

void wildcard(Gene *population) {
    //randomly destroy some of the genes and replace
}

Gene *next_generation(Gene *population) {
    //evolutionary step. Create new genes according to some strategy
    //free the old ones
    int nbytemut = (int)(NGENES*MUTATE_FRAC);
    int ncross = (int)(NGENES*CROSS_FRAC);
    int nswap = (int)(NGENES*SWAP_FRAC);
    int take_best = (int)(TAKE_FRAC*NGENES);

    Gene *genes = malloc(NGENES*sizeof *genes);
    for (int i=0;i<NGENES;i++) {
        // possibility of mutate, cross, swap or clone
        if (i < nbytemut) {
            //genes[i] = byte_mutate(&population[rand()%take_best],MUTATE_PROB);
            genes[i] = byte_mutate(&population[i],MUTATE_PROB);
        } else if (i < nbytemut + ncross) {
            int gi1 = rand() % take_best;
            int gi2 = rand() % take_best;
            genes[i] = byte_cross(&population[gi1], &population[gi2]);
        } else if (i < nbytemut + ncross + nswap) {
            genes[i] = byte_swap(&population[rand() % take_best], MUTATE_PROB);
        } else {
            genes[i] = cloner(&population[rand() % take_best]);
        }
    }
    free_population(population, NGENES);
    return genes;
}

bool is_same(Gene *g1, Gene *g2) {
    if ((g1->score - g2->score < 0.001) && (g1->score - g2->score > -0.001))
        return true;
    return false;
}

bool add_to_hof(Gene g, Gene *hof) {
    for (int i=0;i<HOFSIZE;i++) {
        if (is_same(&g, &hof[i])) return false;
        if (g.score < hof[i].score) {
            //we have a winner
            free(hof[HOFSIZE-1].bytes);
            for (int j=HOFSIZE-1;j>i;j--) {
                hof[j] = hof[j-1];
            }
            hof[i] = cloner(&g);
            return true;
        }
    }
    return false;
}

Gene *evolve(Node *nodes) {
    //init stuff
    Gene *hof = malloc(HOFSIZE* sizeof *hof);
    Gene *population = malloc(NGENES* sizeof *population);
    gen_genes(population, NGENES);
    score_and_sort(population, nodes);
    for (int i=0;i<HOFSIZE;i++) {
        hof[i] = cloner(&population[i]);
    }
    if (VERBOSE) print_population_stats(population, hof, 0);
    int lastupdate = 0;
    //evolve
    for (int i=0;i<NGENS;i++) {
        if (i - lastupdate >= 100) break;  //no improvements
        population = next_generation(population);
        score_and_sort(population, nodes);
        if (SNAPSHOTS && i%20 == 0) print_snapshot(&population[0]);
        int j=0;
        while (1) {
            if(add_to_hof(population[j],hof)) {
                lastupdate = i;
                j++;
            } else break;
        }
        if (VERBOSE) print_population_stats(population, hof, i+1);
    }
    if (SNAPSHOTS) print_snapshot(&hof[0]);
        else print_population(hof, HOFSIZE);
    free_population(population, NGENES);
    return hof;
}

void testgrid() {
    //srand(time(NULL));   // random seed
    int x=10;
    int y=10;
    int gap=10;
    NNODES = x*y;
    Node *nodes = malloc(NNODES*sizeof *nodes);
    gen_grid(x, y, gap, nodes);
    evolve(nodes);
}

void set_globals(int nnodes, int verbose, int snapshots) {
    if (verbose == 1) VERBOSE = true;
    if (snapshots == 1) SNAPSHOTS = true;
    NNODES = nnodes;
    NGENES = 5000;
    TAKE_FRAC = 0.6;
    MUTATE_PROB = 1.0/NNODES;
    MUTATE_FRAC = 0.5;
    CROSS_FRAC = 0.50;
    SWAP_FRAC = 0.0;
    NGENS = 10000;
    HOFSIZE = 10;
    NORMALIZER = 2<<4;
}

int *pyffi(int nnodes, double *xs, double *ys, int verbose, int snapshots) {
    set_globals(nnodes,verbose, snapshots);
    Node *nodes = malloc(NNODES*sizeof *nodes);
    for (int i=0;i<NNODES;i++) {
        nodes[i] = (Node){xs[i], ys[i]};
        if (VERBOSE) printf("Node %d x %.2f y %.2f\n", i, nodes[i].x, nodes[i].y);
    }
    Gene *hof = evolve(nodes);
    //return best
    free(nodes);
    int *position = malloc(NNODES * sizeof *position);
    bytes_argsort(hof[0].bytes,NNODES,position);
    return position;
}

void print_hex(Gene *g) {
    for (int i=0;i<NNODES;i++) {
        printf("%04hx ", g->bytes[i]);
    }
    puts("");
}

int main(int argc, char*argv[]) {
    // This entry point is just for tests
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-v") == 0) VERBOSE = true;
    }
    NNODES = 10;
    Gene g1 = zero_gene();
    for (int i=0;i<NNODES;i++) {
        g1.bytes[i] = 0xffff;
    }
    Gene g2 = zero_gene();
    Gene g3 = byte_cross(&g1, &g2);
    Gene g4 = bit_cross(&g1, &g2);
    print_hex(&g1);
    print_hex(&g2);
    print_hex(&g3);
    print_hex(&g4);
}
