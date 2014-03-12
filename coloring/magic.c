#include <stdlib.h>
#include <stdio.h>

#define SIDELEN 5
#define NITEMS (SIDELEN * SIDELEN)
#define TARGET (SIDELEN * (NITEMS +1)/2)
int count = 0;

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

int takelower(int ind, Square *square) {
    bool *possvals = square->possvalarr[ind].possvals;
    for (int i=0;i<NITEMS;i++) {
        //printf("sqr %d ix %d val %d\n",ind, i, possvals[i]);
        if (possvals[i]) { return square->valset[i]; }
    }
    return -1;
}

Square deepcopy(Square *square) {
    Square newsquare;
    newsquare.valset = square->valset; // no need to deep copy, make sure you don't free
    newsquare.possvalarr = malloc(NITEMS* sizeof(PossVals));
    newsquare.board = malloc(NITEMS * sizeof(int));
    for (int i=0;i<NITEMS;i++) {
        newsquare.board[i] = square->board[i];
        bool *tmp = malloc(NITEMS * sizeof(int));
        for (int j=0;j<NITEMS;j++){ //init to true
            tmp[j] = square->possvalarr[i].possvals[j];
        }
        PossVals newpv = {tmp, square->possvalarr[i].tcount};
        newsquare.possvalarr[i] = newpv;
    }
    return newsquare;
}

bool group_is_feasible(int arr[SIDELEN], Square *square) {
    int uppersum = 0;
    int lowersum = 0;
    for (int i=0;i<SIDELEN;i++) {
        //printf("arr[%d] = %d\n", i, arr[i]);
        int upval = takeupper(arr[i], square);
        int lowval = takelower(arr[i], square);
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

bool check_and_constrain(Square *square) {
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

int *ind2col(int ind) {
    int *col = malloc(SIDELEN * sizeof(int));
    int coln = ind % SIDELEN;
    for (int i=0;i<SIDELEN;i++) 
        col[i] = coln + i*SIDELEN;
    return col;
}

int *ind2row(int ind) {
    int *row = malloc(SIDELEN * sizeof(int));
    int rown = ind / SIDELEN;
    for (int i=0;i<SIDELEN;i++) 
        row[i] = rown*SIDELEN + i;
    return row;
}

int *diagul(int ind) {
    int *diagindul = malloc(SIDELEN * sizeof(int));
    for (int i=0;i<SIDELEN;i++)
        diagindul[i] = SIDELEN * (i+1);
    return diagindul;
}

int *diagur(int ind) {
    int *diagindur = malloc(SIDELEN * sizeof(int));
    for (int i=0;i<SIDELEN;i++)
        diagindur[i] = (SIDELEN-1) + SIDELEN -1;
    
    return diagindur;
}

bool ondiagonal_ul(int ind) {
    if (ind % SIDELEN == ind / SIDELEN)
        return true;
    return false;
}

bool ondiagonal_ur(int ind) {
    if (ind % SIDELEN == (ind / SIDELEN) + SIDELEN - 1)
        return true;
    return false;
}

void set_value(int boardind, int val, Square *square) {
    // index lookup
    int valind = val2index(val, square->valset);
    //printf("Setting index %d to value %d (value index %d)\n", boardind,val,valind);
    // set alternative to false
    for (int i=0;i<NITEMS;i++) {
        if (i != valind && square->possvalarr[boardind].possvals[i]) {
            square->possvalarr[boardind].possvals[i] = false;
            square->possvalarr[boardind].tcount--;
        }
        if (i!=boardind && square->possvalarr[i].possvals[valind]) {
            square->possvalarr[i].possvals[valind] = false;
            square->possvalarr[i].tcount--;
        }
    }
    square->board[boardind] = val;
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

Result make_magic(Square square, int nplaced) {
    // recursively solve
    //print_possvalarr(square->possvalarr);
    //print_board(square.board);
    count += 1;
    //printf("Count: %d\n", count);
    if (!check_and_constrain(&square)) {
        Result reslose = {false, NULL};
        return reslose;
    } else if (nplaced == NITEMS) {
        //win!
        Result reswin = {true, square};
        return reswin;
    }
    int *order_difficulty = find_most_constrained(square.possvalarr);
    for (int i=0;i<NITEMS-nplaced;i++) {
        Square s2 = deepcopy(&square);
        int next_index = order_difficulty[i];
        while (s2.board[next_index] != -1) next_index = order_difficulty[++i];     //hack to allow it to finish when nplaced = 8
        set_value(next_index, takelower(next_index, &s2), &s2);
        Result res = make_magic(s2, nplaced+1);
        if (res.success) {
            print_board(res.square.board);
            //return res;
        } else {
            free_square(s2);
        }
    }
    Result res = {false, NULL};
    return res;
}

int main(int argc, char *argv[]) {
    Square square = initsquare(SIDELEN);
    Result res = make_magic(square, 0);
    if (res.success)
        print_board(res.square.board);
    return 0;
}
