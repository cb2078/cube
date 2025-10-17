extern int pow2[];
extern int pow3[];
extern int fact[];
extern int powfact4[];
int pick(int n, int k);
int choose(int n, int k);

int get_combination(char *x, int n, int k);
void set_combination(char *x, int n, int k, int r);
int get_partial_permutation(char *x, int n, int k);
void set_partial_permutation(char *x, int n, int k, int r);
int get_permutation(char *x, int n);
void set_permutation(char *x, int n, int k);
int get_parity(char *x, int n);
