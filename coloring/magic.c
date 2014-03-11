// Don't really know how to do this so just hack away and see where I get to

#include <stdlib.h>
#include <stdio.h>

static const int SIDELEN = 5;
int count = 0;

typedef enum {false, true} bool;

typedef struct Square {
    int *valset;
    bool **possvalarr;
    int *board;
} Square;

typedef struct Result {
    bool success;
    Square *square;
} Result;

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

void print_possvalarr(bool **possvalarr) {
    for (int i=0;i<SIDELEN;i++) {
        for (int j=0;j<SIDELEN;j++) {
            printf("r%d c%d: ", i, j);
            for (int k=0;k<SIDELEN*SIDELEN;k++) {
                if (possvalarr[SIDELEN*i + j][k]) {
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
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        printf("%d", valset[i]);
    }
        printf("\n");
}

Square initsquare() {
    Square square;
    square.valset = malloc(SIDELEN*SIDELEN * sizeof(int));
    square.possvalarr = malloc(SIDELEN * SIDELEN * sizeof(bool *));
    square.board = malloc(SIDELEN*SIDELEN * sizeof(int));
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        square.valset[i] = i+1;
    }
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        square.board[i] = -1;
        bool *tmp = malloc(SIDELEN * SIDELEN * sizeof(int));
        for (int j=0;j<SIDELEN*SIDELEN;j++){ //init to true
            tmp[j] = true;
        }
        square.possvalarr[i] = tmp;
    }
    return square;
}

void free_square(Square square) {
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        free(square.possvalarr[i]);
    }
    free(square.possvalarr);
    free(square.board);
}


int takeupper(int ind, Square *square) {
    bool *possvals = square->possvalarr[ind];
    for (int i=SIDELEN*SIDELEN-1;i>=0;i--) {
        /*printf("sqr %d ix %d val %d\n",ind, i, possvals[i]);*/
        if (possvals[i]) { return square->valset[i]; }
    }
    return -1;
}

int takelower(int ind, Square *square) {
    bool *possvals = square->possvalarr[ind];
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        //printf("sqr %d ix %d val %d\n",ind, i, possvals[i]);
        if (possvals[i]) { return square->valset[i]; }
    }
    return -1;
}

Square deepcopy(Square *square) {
    Square newsquare;
    newsquare.valset = square->valset; // no need to deep copy, make sure you don't free
    newsquare.possvalarr = malloc(SIDELEN * SIDELEN * sizeof(bool *));
    newsquare.board = malloc(SIDELEN * SIDELEN * sizeof(int));
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        newsquare.valset[i] = square->valset[i];
        newsquare.board[i] = square->board[i];
        bool *tmp = malloc(SIDELEN*SIDELEN * sizeof(int));
        for (int j=0;j<SIDELEN*SIDELEN;j++){ //init to true
            tmp[j] = square->possvalarr[i][j];
        }
        newsquare.possvalarr[i] = tmp;
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
    int TARGET  = SIDELEN*((SIDELEN*SIDELEN)+1)/2;
    return uppersum >= TARGET && lowersum <= TARGET;
}

bool check_feasible(Square *square) {
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
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
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
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        if (i != valind)
            square->possvalarr[boardind][i] = false;
    }
    for (int j=0;j<SIDELEN*SIDELEN;j++) {
        if (j!=boardind)
            square->possvalarr[j][valind] = false;
    }
    square->board[boardind] = val;
}

Result make_magic(Square *square, int nplaced) {
    // recursively solve
    //print_possvalarr(square->possvalarr);
    //print_board(square->board);
    count += 1;
    printf("Count: %d\n", count);
    if (!check_feasible(square)) {
        Result reslose = {false, NULL};
        return reslose;
    } else if (nplaced == SIDELEN*SIDELEN) {
        //win!
        Result reswin = {true, square};
        return reswin;
    }
    for (int i=0;i<SIDELEN*SIDELEN;i++) {
        if (square->board[i] == -1) {
            Square s2 = deepcopy(square);
            set_value(i, takelower(i, &s2), &s2);
            Result res = make_magic(&s2, nplaced+1);
            if (res.success) {
                return res;
            } else {
                free_square(s2);
            }
        }
    }
    Result res = {false, NULL};
    return res;
}


/*void set_value(int boardind, int val, Square *square) {*/
    /*// index lookup*/
    /*int valind = val2index(val, square->valset);*/
    /*// set alternative to false*/
    /*for (int i=0;i<SIDELEN;i++) {*/
        /*if (i != valind)*/
            /*square->possvalarr[boardind][i] = false;*/
    /*}*/
    /*// set other board members in col, row to false*/
    /*int *rowinds = ind2row(boardind);*/
    /*int *colinds = ind2col(boardind);*/
    /*for (int j=0;j<SIDELEN;j++) {*/
        /*int rind = rowinds[j];*/
        /*int cind = colinds[j];*/
        /*if (rind != boardind)*/
            /*square->possvalarr[rind][valind] = false;*/
        /*if (cind != boardind)*/
            /*square->possvalarr[cind][valind] = false;*/
    /*}*/

    /*// do same for diagonals*/
    /*if (ondiagonal_ul(boardind)) {*/
        /*int *diagind_ul = diagul(boardind);*/
        /*for (int i=0;i<SIDELEN;i++) {*/
            /*if (diagind_ul[i]!=boardind)*/
                /*square->possvalarr[i][valind] = false;*/
        /*}*/
    /*}*/
    /*if (ondiagonal_ur(boardind)) {*/
        /*int *diagind_ur = diagur(boardind);*/
        /*for (int i=0;i<SIDELEN;i++) {*/
            /*if (diagind_ur[i]!=boardind)*/
                /*square->possvalarr[i][valind] = false;*/
        /*}*/
    /*}*/
    /*square->board[boardind] = val;*/
/*}*/


int main(int argc, char *argv[]) {
    Square square = initsquare(SIDELEN);
    Result res = make_magic(&square, 0);
    if (res.success)
        print_board(res.square->board);
    return 0;
}
