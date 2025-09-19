#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <SDL3/SDL.h>

#include <cglm/cglm.h>

#define EPSILON 1e-3
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define ABS(x) ((x)<0 ? -(x) : x)
#define LOOP(i, j, k) for (int i=0; i<3; ++i) for (int j=0; j<3; ++j) for (int k=0; k<3; ++k)
#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

#define NUM_CUBIES 27
#define NUM_CORNERS 8
#define NUM_EDGES 12

enum move_type {
    FACE_TURN,
    WIDE_MOVE,
    ROTATION,
    SLICE_MOVE,
};

enum move {
    U, R, F, D, L, B,
    UW, RW, FW, DW, LW, BW,
    Y, X, Z, E, M, S,

    U2, R2, F2, D2, L2, B2,
    UW2, RW2, FW2, DW2, LW2, BW2,
    Y2, X2, Z2, E2, M2, S2,

    U3, R3, F3, D3, L3, B3,
    UW3, RW3, FW3, DW3, LW3, BW3,
    Y3, X3, Z3, E3, M3, S3,

    NUM_MOVES,
};

enum cubie_type {CORNER, EDGE, CENTRE};

static inline int get_move_type(int move)
{
    move%=U2;
    if (move>=E)
        return SLICE_MOVE;
    if (move>=Y)
        return ROTATION;
    if (move>=UW)
        return WIDE_MOVE;
    return FACE_TURN;
}

// vector-model.c

struct vector_model {
    vec3 cubies[NUM_CUBIES];
    vec4 transforms[NUM_CUBIES];
};
int on_face(vec3, int);
void vm_move(struct vector_model *, int);
void vm_init(struct vector_model *);
int vm_get_cubie_type(struct vector_model *, int);
int vm_get_cubie_permutation(struct vector_model *, int);
int vm_get_cubie_orientation(struct vector_model *, int);

// cubie-model.c

struct cubie_model {
    char cp[NUM_CORNERS], co[NUM_CORNERS], ep[NUM_EDGES], eo[NUM_EDGES];
};
void cm_from_vm(struct vector_model *, struct cubie_model *);
void cm_print(struct cubie_model *);
void cm_init(struct cubie_model *);
void cm_move(struct cubie_model *, int);
