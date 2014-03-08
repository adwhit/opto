#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

typedef enum {false, true} bool;

typedef struct {
    int value;
    int weight;
} Item;

typedef struct {
    Item *items;
    int capacity;
    int nitems;
} Knapsack;

typedef struct {
    int best_value;
    bool *in_sack;
} Result;

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
    int i = ks.capacity-1;
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
    Result res = {array[ks.capacity-1][ks.nitems-1], resarr};
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
    if (weight < ks.capacity && sum == res.best_value) {
        return true;
    } else {
        return false;
    }
}



Result dynprog(Knapsack ks) {
    int **array = make2Darray(ks.capacity, ks.nitems);
    // init for i=0
    int weight = ks.items[0].weight;
    int value = ks.items[0].value;
    for (int i=0;i<ks.capacity;i++) {
        if (weight <= i) {
            array[i][0] = value;
        } else {
            array[i][0] = 0;
        }
    }
    for (int j=1;j<ks.nitems;j++) {
        int weight = ks.items[j].weight;
        int value = ks.items[j].value;
        for (int i=0;i<ks.capacity;i++) {
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
    }
    fclose(myfile);
    Knapsack ks = {items, capacity+1, nitems};
    return ks;
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        printf("No file name specified\n");
        exit(1);
    }
    Knapsack ks = parse_file(argv[1]);
    for (int i=0;i<ks.nitems;i++) {
        printf("item %d: weight %d, value: %d\n", i, ks.items[i].weight, ks.items[i].value);
    }
    Result res = dynprog(ks);
    printf("Score: %d\n", res.best_value);
    for (int i=0;i<ks.nitems;i++) {
        printf("%d ", res.in_sack[i]);
    }
    printf("\n");
    printf("success: %d\n", checkscore(res, ks));
    return 0;
}
