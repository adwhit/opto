#include <stdlib.h>
#include <stdio.h>

#define SIDELEN 3
#define NITEMS (SIDELEN * SIDELEN)
#define TARGET (SIDELEN * (NITEMS +1)/2)
int itercount = 0;
int magiccount = 0;

typedef enum {false, true} bool;

 typedef struct PossVals {
     bool *possvals;
     int tcount;
 } PossVals;

typedef struct Square {
    int *valset;
    PossVals *possvalarr;
    int *board;
} Square;


typedef struct Result {
    bool success;
    Square square;
} Result;

struct OrdSort {
    int val;
    int ind;
};

void print_board(int *board) {
    for (int i=0;i<SIDELEN;i++) {
        for (int j=0;j<SIDELEN;j++) {
            if (board[SIDELEN*i + j] == -1) {
                printf(". ");
            } else {
                printf("%d ",board[SIDELEN*i + j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

void print_possvalarr(PossVals *possvalarr) {
    for (int i=0;i<SIDELEN;i++) {
        for (int j=0;j<SIDELEN;j++) {
            printf("r%d c%d: ", i, j);
            for (int k=0;k<NITEMS;k++) {
                if (possvalarr[SIDELEN*i + j].possvals[k]) {
                    printf(". ");
                } else {
                    printf("X ");
                }
            }
        printf("|");
        }
    printf("\n");
    }
}

void print_valset(int *valset) {
    printf("Value set: ");
    for (int i=0;i<NITEMS;i++) {
        printf("%d", valset[i]);
    }
        printf("\n");
}

Square initsquare() {
    Square square;
    square.valset = malloc(NITEMS*sizeof(int));
    square.possvalarr = malloc(NITEMS*sizeof(PossVals));
    square.board = malloc(NITEMS*sizeof(int));
    for (int i=0;i<NITEMS;i++) {
        square.valset[i] = i+1;
    }
    for (int i=0;i<NITEMS;i++) {
        square.board[i] = -1;
        bool *tmp = malloc(NITEMS*sizeof(int));
        for (int j=0;j<NITEMS;j++){ //init to true
            tmp[j] = true;
        }
        PossVals poss = {tmp, NITEMS };
        square.possvalarr[i] = poss;
    }
    return square;
}

void free_square(Square square) {
    for (int i=0;i<NITEMS;i++) {
        free(square.possvalarr[i].possvals);
    }
    free(square.possvalarr);
    free(square.board);
}


int takeupper(int ind, Square *square) {
    bool *possvals = square->possvalarr[ind].possvals;
    for (int i=NITEMS-1;i>=0;i--) {
        /*printf("sqr %d ix %d val %d\n",ind, i, possvals[i]);*/
        if (possvals[i]) { return square->valset[i]; }
    }
    return -1;
}

int take_n_lowest(int ind, int n, Square *square) {
    //use zero indexing
    bool *possvals = square->possvalarr[ind].possvals;
    for (int i=0;i<NITEMS;i++) {
        //printf("sqr %d ix %d val %d\n",ind, i, possvals[i]);
        if (possvals[i]) { 
            if (!n) return square->valset[i]; 
            n--;
        }
    }
    return -1;
}

Square deepcopy(Square square) {
    Square newsquare;
    newsquare.valset = square.valset; // no need to deep copy, make sure you don't free
    newsquare.possvalarr = malloc(NITEMS* sizeof(PossVals));
    newsquare.board = malloc(NITEMS * sizeof(int));
    for (int i=0;i<NITEMS;i++) {
        newsquare.board[i] = square.board[i];
        bool *tmp = malloc(NITEMS * sizeof(int));
        for (int j=0;j<NITEMS;j++){ //init to true
            tmp[j] = square.possvalarr[i].possvals[j];
        }
        PossVals newpv = {tmp, square.possvalarr[i].tcount};
        newsquare.possvalarr[i] = newpv;
    }
    return newsquare;
}

bool group_is_feasible(int arr[SIDELEN], Square square) {
    int uppersum = 0;
    int lowersum = 0;
    for (int i=0;i<SIDELEN;i++) {
        //printf("arr[%d] = %d\n", i, arr[i]);
        int upval = takeupper(arr[i], &square);
        int lowval = take_n_lowest(arr[i],0, &square);
        //printf("ind %d lowval %d upval %d\n", i, lowval, upval);
        if (upval == -1 || lowval == -1) {
            return false;
        }
        uppersum += upval;
        lowersum += lowval;
    }
    //printf("lowersum %d uppersum %d\n", lowersum, uppersum );
    return uppersum >= TARGET && lowersum <= TARGET;
}

bool enforce_ordering(int *board) {
    int tl = board[0];
    int tr = board[SIDELEN-1];
    int bl = board[NITEMS-SIDELEN];
    int br = board[NITEMS-1];
    if (br != -1 && br < tl) return false;
    if (bl != -1 && (bl < tr || bl < tl)) return false;
    if (tr != -1 && tr < tl) return false;
    return true;
}

bool check_and_constrain(Square square) {
    //set ordering constraints
    if (!enforce_ordering(square.board)) return false;
    int rowind[SIDELEN];
    int colind[SIDELEN];
    int diagindul[SIDELEN];
    int diagindur[SIDELEN];
    for (int i=0;i<SIDELEN;i++) {
        for (int j=0;j<SIDELEN;j++) {
            rowind[j] = SIDELEN * i + j;
            colind[j] = i + SIDELEN * j;
        }
        diagindul[i] = (SIDELEN +1) * i;
        diagindur[i] = (SIDELEN-1) * i + SIDELEN -1;
        if (!(group_is_feasible(rowind, square) && group_is_feasible(colind, square)))
            return false;
    }
    if (!(group_is_feasible(diagindul, square) && group_is_feasible(diagindur, square)))
            return false;
    return true;
}

int val2index(int val, int *valset) {
    for (int i=0;i<NITEMS;i++) {
        if (val == valset[i])
            return i;
        }
    return -1;
}

bool set_value_and_propagate(int boardind, int val, Square square) {
    // index lookup
    int valind = val2index(val, square.valset);
    //printf("Setting index %d to value %d (value index %d)\n", boardind,val,valind);
    // set alternative to false
    for (int i=0;i<NITEMS;i++) {
        if (i != valind && square.possvalarr[boardind].possvals[i]) {
            square.possvalarr[boardind].possvals[i] = false;
            square.possvalarr[boardind].tcount--;
        }
        if (i!=boardind && square.possvalarr[i].possvals[valind]) {
            square.possvalarr[i].possvals[valind] = false;
            square.possvalarr[i].tcount--;
            if (square.possvalarr[i].tcount == 0) return false;
        }
    }
    square.board[boardind] = val;
    return check_and_constrain(square);
}

int cmp(const void *v1, const void *v2) {
    const int i1 = **(const int **)v1;
    const int i2 = **(const int **)v2;
    if (i1 == 1) return 1;
    if (i2 == 1) return -1;
    return i1<i2?-1:(i1>i2);
}

int *sortorder(int array[NITEMS]) {
    int *parray[NITEMS],i;
    int *position = malloc(NITEMS * sizeof(int));
    for(i = 0; i < NITEMS;i++) parray[i] = &array[i];
    qsort(parray,NITEMS, sizeof *parray, cmp);
    for(i = 0;i < NITEMS;i++) position[i] = (parray[i] - array);
    return position;
}

int *find_most_constrained(PossVals *possvalarr) {
    int tcounts[NITEMS];
    for (int i=0;i<NITEMS;i++) (tcounts[i] = possvalarr[i].tcount);
    return sortorder(tcounts);
}

void make_magic(int next_index, int next_value, Square square, int nplaced) {
    itercount += 1;
    //printf("Count: %d\n", itercount);
    if (!set_value_and_propagate(next_index, next_value, square)) {
        free_square(square);
        return;
    } else if (nplaced == NITEMS) {
        //win!
        //print_board(res.square.board);
        magiccount++;
        printf("Magic count: %d\n", magiccount);
        free_square(square);
        return;
    }
    //print_possvalarr(square.possvalarr);
    //print_board(square.board);
    int *order_difficulty = find_most_constrained(square.possvalarr);
    //for (int i=0;i<NITEMS;i++) printf("ind %i order: %d\n", i, order_difficulty[i]);
    for (int i=0;i<NITEMS-nplaced;i++) {
        //next value
        int next_index = order_difficulty[i];
        while (square.board[next_index] != -1) {
            i++; 
            if (i>=NITEMS) return;
            //printf("i: %d \n", i);
            next_index = order_difficulty[i];     //hack to allow it to finish when nplaced = 8
        }
        int next_value = NITEMS + 1;
        int nthitem = 0;
        while (next_value != -1) {
            next_value = take_n_lowest(next_index, nthitem, &square);
            nthitem++;
            make_magic(next_index, next_value, deepcopy(square), nplaced+1);
        }
    }
    free_square(square);
    return;
}

int main(int argc, char *argv[]) {
    Square square = initsquare(SIDELEN);
    make_magic(0,1,square, 8);
    return 0;
}
