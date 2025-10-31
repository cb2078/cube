extern int move_set[18];
int prune_move(int, int);
void possible_moves(int *, int *, int, int[6]);
void print_moves(int *, int);
void make_scramble(int *, int);
void read_moves(char *s, int *, int *);
int apply_cancellations(int *, int *);
void inverse_moves(int *, int);
