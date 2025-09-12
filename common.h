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
};

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

#define NUM_CUBIES 27
struct vector_model {
    vec3 cubies[27];
    vec4 transforms[27];
};
int on_face(vec3, int);
void vector_model_move(struct vector_model *, int); // TODO rename some symbols to be more consistent
struct vector_model vector_model_new();
int get_cubie_type(struct vector_model *, int);
int get_cubie_permutation(struct vector_model *, int);
int get_cubie_orientation(struct vector_model *, int);

// cubie-model.c

struct cubie_model {
    char co[8], eo[12], cp[8], ep[8];
};
