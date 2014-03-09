#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int VERBOSE = 0;

typedef enum {false, true} bool;

typedef struct {
    int value;
    int weight;
    int index;
    double relweight;
} Item;

typedef struct {
    Item *items;
    int capacity;
    int nitems;
    int *weights;
    int *values;
    double relaxation;
    int best_value;
} Knapsack;

typedef struct {
    bool success;
    int value;
    bool *route;
} Result;

int compare(const void *a, const void *b) {
    Item *x = (Item *)a;
    Item *y = (Item *)b;
    if ((*x).relweight > (*y).relweight) return -1;
    else if ((*x).relweight < (*y).relweight) return 1; 
    //secondary sort
    else if ((*x).weight > (*y).weight) return -1; 
    else if ((*x).weight < (*y).weight) return 1; 
    return 0;
}

double relax(int *weights_ord, int *values_ord, int startind, int endind, double capacity) {
    double rlx = 0;
    for (int i=startind;i<endind;i++) {
        if (capacity > weights_ord[i]) {
            rlx += values_ord[i];
            capacity -= (double)weights_ord[i];
        } else {
            rlx += values_ord[i] * capacity/(double)weights_ord[i];
            break;
        }
    }
    if (isinf(rlx)) exit(1);
    return rlx;
}

Result traverse(int value, int room, double estimate, Knapsack *ks, int depth) {
    //check for endpoint
    Result rtn1 = {false, 0, NULL};
    if (estimate < (double)(ks->best_value)) return rtn1; //fail
    else if (depth >= ks->nitems) {
        if (value > ks->best_value) {                     //win!
            ks->best_value = value;
            if (VERBOSE) printf("New best: %d, room: %d\n", value, room);
            bool *route = calloc(depth, sizeof(bool));
            Result win =  {true, value, route};
            return win;
        } else return rtn1;                               //fail
    }
    // else branch
    int v = ks->values[depth];
    int w = ks->weights[depth];
    if (w <= room) {
        double est_n1 = value + v + relax(ks->weights, ks->values, depth+1, ks->nitems, room - w);
        rtn1 = traverse(value + v, room - w, est_n1, ks, depth+1);
    }
    double est_n2 = value + relax(ks->weights, ks->values, depth+1, ks->nitems, room);
    Result rtn2 = traverse(value, room, est_n2, ks, depth+1);
    if (rtn1.success && rtn2.success) {
        if (rtn1.value > rtn2.value) {
            free(rtn2.route);
            rtn1.route[depth] = true;
            return rtn1;
        } else {
            free(rtn1.route);
            return rtn2;
        }
    } else if (rtn1.success) {
        rtn1.route[depth] = true;
        return rtn1;
    } else if (rtn2.success) return rtn2; 
    else return rtn1; //fail
}
    
Result branch_bound(Knapsack *ks) {
    return traverse(0, ks->capacity, ks->relaxation, ks, 0);
}

int **make2Darray(int rows, int cols) {
    int **array; 
    array = calloc(rows, sizeof(int *));
    for (int i=0;i<rows;i++) 
        array[i] = calloc(cols,sizeof(int));
    return array;
}

void printarr(int **arr, int imax, int jmax) {
    for(int i=0;i<imax;i++) {
        printf("cap: %02d | ", i); 
        for (int j=0;j<jmax;j++) {
            printf("%02d ", arr[i][j]);
        }
        printf("\n");
    }
}

Result score(int **array, Knapsack *ks) {
    int i = ks->capacity;
    int j = ks->nitems-1;
    bool *resarr = calloc(ks->nitems, sizeof(bool *));
    while (i>0 && j>=0) {
        if (array[i][j] == array[i-1][j]) {
                i--;
        }
        if (array[i][j] == array[i][j-1]) {
            resarr[j] = false;
            j--;
        } else {
            resarr[j] = true;
            i -= ks->items[j].weight;
            j--;
        }
    }
    Result res = {true, array[ks->capacity][ks->nitems-1], resarr};
    return res;
}

bool checkscore(Result res, Knapsack ks) {
    int sum = 0;
    int weight = 0;
    for (int i=0;i<ks.nitems;i++) {
        if (res.route[i]) {
            weight += ks.items[i].weight;
            sum += ks.items[i].value;
        }
    }
    if (weight <= ks.capacity && sum == ks.best_value) 
        return true;
    else 
        return false;
}



Result dynprog(Knapsack *ks) {
    int **array = make2Darray(ks->capacity+1, ks->nitems);
    // init for i=0
    int weight = ks->items[0].weight;
    int value = ks->items[0].value;
    for (int i=0;i<=ks->capacity;i++) {
        if (weight <= i)
            (array[i][0] = value);
        else 
            (array[i][0] = 0);
    }
    for (int j=1;j<ks->nitems;j++) {
        int weight = ks->items[j].weight;
        int value = ks->items[j].value;
        for (int i=0;i<=ks->capacity;i++) {
            int prev = array[i][j-1];
            if (i >= weight) {
                int ifremove = array[i-weight][j-1];
                if (value + ifremove > prev)
                    (array[i][j] = value + ifremove);
                else
                    (array[i][j] = prev);
            } else {
                array[i][j] = prev;
            }
        }
    }
    //printarr(array, ks.capacity, ks.nitems);
    return score(array, ks);
}

void knapsort(Knapsack *ks) {
    qsort(ks->items, ks->nitems, sizeof(Item), compare);
    int *weights = malloc(ks->nitems * sizeof(int));
    int *vals = malloc(ks->nitems * sizeof(int));
    for (int i=0; i<(ks->nitems); i++) {
        weights[i] = ks->items[i].weight;
        vals[i] = ks->items[i].value;
    }
    ks->weights = weights;
    ks->values = vals;
    ks->relaxation = relax(weights, vals, 0, ks->nitems, ks->capacity);
    ks->best_value = 0;
}

Knapsack parse_file(char *fpath) {
    int nitems;
    int capacity;
    FILE *myfile = fopen (fpath,"r");
    Item *items = NULL;
    if (myfile == NULL) {
        perror ("Error opening file"); //or return 1;
        exit(1);
    }
    fscanf(myfile, "%d %d", &nitems, &capacity);
    items = (Item *)calloc(nitems,sizeof(Item));
    for (int i=0;i<nitems;i++ ) {
        fscanf(myfile, "%d %d", &(items[i].value), &(items[i].weight));
        items[i].relweight = (double)items[i].value / (double)items[i].weight;
        items[i].index = i;
    }
    //sort descending
    fclose(myfile);
    Knapsack ks = {items, capacity, nitems, NULL};
    knapsort(&ks);
    return ks;
}


int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("No file name specified\n");
        exit(1);
    }
    int DYNAMIC = false;
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-v") == 0)
            VERBOSE = true;
        if (strcmp(argv[i],"-d") == 0)
            DYNAMIC = true;
    }
    Knapsack ks = parse_file(argv[1]);
    if (VERBOSE) {
        printf("Items: %d  Capacity: %d\n", ks.nitems, ks.capacity);
        for (int i=0;i<ks.nitems;i++) {
            printf("item %d: weight %d, value: %d, relweight: %0.2f\n", i, ks.items[i].weight, ks.items[i].value, ks.items[i].relweight);
        }
    }
    Result res = DYNAMIC? dynprog(&ks) : branch_bound(&ks);
    if (VERBOSE) printf("Linear Relaxation: %0.2f\n", ks.relaxation);
    printf("%d 1\n", ks.best_value);
    for (int i=0;i<ks.nitems;i++) {
        for (int j=0;j<ks.nitems;j++) {
            if (ks.items[j].index == i) {
                printf("%d ", res.route[j]);
                break;
            }
        }
    }
    printf("\n");
    if (VERBOSE) {
        char *errck = checkscore(res, ks) ? "Passed" : "Failed";
        printf("Check sum: %s\n", errck);
    }
    return 0;
}
