#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
    int *best;
} Knapsack;

typedef struct {
    int best_value;
    bool *in_sack;
} Result;

typedef struct {
    bool success;
    int value;
    bool *route;
} RtnNode;

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
    if (isinf(rlx)) {
        exit(1);
    }

    return rlx;
}


RtnNode recurse(int value, int room, double estimate, Knapsack *ks, int depth) {
    //check for endpoint
    if (estimate < (double)*(ks->best)) {
        //fail
        RtnNode rtn = {false, 0, NULL};
        return rtn;
    } else if (depth >= ks->nitems) {
        if (value > *(ks->best)) {
            //win!
            *(ks->best) = value;
            //printf("New best: %d, room: %d\n", value, room);
            bool *route = calloc(depth, sizeof(bool));
            RtnNode win =  {true, value, route};
            return win;
        } else {
            //else fail
            RtnNode rtn = {false, 0, NULL};
            return rtn;
        }
    }
    int v = ks->values[depth];
    int w = ks->weights[depth];
    RtnNode rtn1 = {false, 0, NULL};
    if (w <= room) {
        double est_n1 = value + v + relax(ks->weights, ks->values, depth+1, ks->nitems, room - w);
        rtn1 = recurse(value + v, room - w, est_n1, ks, depth+1);
    }
    double est_n2 = value + relax(ks->weights, ks->values, depth+1, ks->nitems, room);
    RtnNode rtn2 = recurse(value, room, est_n2, ks, depth+1);
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
    } else if (rtn2.success) {
        return rtn2;
    } else { 
        //fail
        RtnNode rtn = {false, 0, NULL};
        return rtn;
    }
}
    
Result branch_bound(Knapsack ks) {
    RtnNode rtn = recurse(0, ks.capacity, ks.relaxation, &ks, 0);
    Result res = {rtn.value, rtn.route};
    return res;
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

Result score(int **array, Knapsack ks) {
    int i = ks.capacity;
    int j = ks.nitems-1;
    bool *resarr = calloc(ks.nitems, sizeof(bool *));
    while (i>0 && j>=0) {
        if (array[i][j] == array[i-1][j]) {
                i--;
        }
        if (array[i][j] == array[i][j-1]) {
            resarr[j] = false;
            j--;
        } else {
            resarr[j] = true;
            i -= ks.items[j].weight;
            j--;
        }
    }
    Result res = {array[ks.capacity][ks.nitems-1], resarr};
    return res;
}

bool checkscore(Result res, Knapsack ks) {
    int sum = 0;
    int weight = 0;
    for (int i=0;i<ks.nitems;i++) {
        if (res.in_sack[i]) {
            weight += ks.items[i].weight;
            sum += ks.items[i].value;
        }
    }
    if (weight <= ks.capacity && sum == res.best_value) {
        return true;
    } else {
        return false;
    }
}



Result dynprog(Knapsack ks) {
    int **array = make2Darray(ks.capacity+1, ks.nitems);
    // init for i=0
    int weight = ks.items[0].weight;
    int value = ks.items[0].value;
    for (int i=0;i<=ks.capacity;i++) {
        if (weight <= i) {
            array[i][0] = value;
        } else {
            array[i][0] = 0;
        }
    }
    for (int j=1;j<ks.nitems;j++) {
        int weight = ks.items[j].weight;
        int value = ks.items[j].value;
        for (int i=0;i<=ks.capacity;i++) {
            int prev = array[i][j-1];
            if (i >= weight) {
                int ifremove = array[i-weight][j-1];
                if (value + ifremove > prev) {
                    array[i][j] = value + ifremove;
                } else {
                    array[i][j] = prev;
                }
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
    int *best = calloc(1, sizeof(int));
    ks->weights = weights;
    ks->values = vals;
    ks->relaxation = relax(weights, vals, 0, ks->nitems, ks->capacity);
    ks->best = best;
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
    if (argc != 2) {
        printf("No file name specified\n");
        exit(1);
    }
    Knapsack ks = parse_file(argv[1]);
    printf("Items: %d  Capacity: %d\n", ks.nitems, ks.capacity);
    for (int i=0;i<ks.nitems;i++) {
        printf("item %d: weight %d, value: %d, relweight: %0.2f\n", i, ks.items[i].weight, ks.items[i].value, ks.items[i].relweight);
    }
    //Result res = dynprog(ks);
    Result res = branch_bound(ks);
    printf("Linear Relaxation: %0.2f\n", ks.relaxation);
    printf("Score: %d\n", res.best_value);
    for (int i=0;i<ks.nitems;i++) {
        if (res.in_sack[i]) {
        //printf("%i ", i);
        }
    }
    printf("\n");
    char *errck = checkscore(res, ks) ? "Passed" : "Failed";
    printf("Check sum: %s\n", errck);
    return 0;
}
