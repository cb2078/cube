#include "common.h"

int on_face(vec3 cubie, int face)
{
    return round(cubie[face%3])==(int)(face/3)*2-1;
}

static int can_move_cubie(vec3 cubie, int face, int type)
{
    int opposite_face=(face+3)%6;
    switch (type) {
        case FACE_TURN:
            return on_face(cubie, face);
        case WIDE_MOVE:
            return !on_face(cubie, opposite_face);
        case ROTATION:
            return 1;
        case SLICE_MOVE:
            return !on_face(cubie, face) && !on_face(cubie, opposite_face);
        default:
            exit(1);
    }
}

static void get_cubie_position(struct vector_model *cube, int i, vec3 v)
{
    glm_quat_rotatev(cube->transforms[i], cube->cubies[i], v);
}

void vector_model_move(struct vector_model *cube, int move)
{
    int face=move%6;
    int type=get_move_type(move);
    int dim=face%3;
    int amount=move/U2+1;
    int sign=1-face/3*2;

    for (int i=0; i<27; ++i) {
        vec3 v;
        get_cubie_position(cube, i, v);
        if (!can_move_cubie(v, face, type)) continue;
        glm_vec3_zero(v);
        v[dim]=1;
        vec4 q;
        glm_quatv(q, sign*amount*glm_rad(90), v);
        glm_quat_mul(q, cube->transforms[i], cube->transforms[i]);
    }
}

// todo: turn this into a 'new' function
struct vector_model vector_model_new(void)
{
    struct vector_model cube;
    int i=0;
    LOOP(x, y, z) glm_vec3_copy((vec3){x-1, y-1, z-1}, cube.cubies[i++]);
    glm_quat_identity_array(cube.transforms, 27);
    return cube;
}

int get_cubie_type(struct vector_model *cube, int i)
{
    vec3 v;
    get_cubie_position(cube, i, v);
    int num_zeros = 0;
    for (int i=0; i<3; ++i) num_zeros += v[i];
    return (enum cubie_type)num_zeros;
}

int get_cubie_permutation(struct vector_model *cube, int i)
{
    struct vector_model solved = vector_model_new();
    vec3 v;
    get_cubie_position(cube, i, v);
    int type = get_cubie_type(cube, i);

    for (int i=0, j=0; i<27; ++i) {
        if (glm_vec3_eqv_eps(v, solved.cubies[i])) return j;
        j += get_cubie_type(cube, i)==type;
    }
    assert(0); // todo: unreachable
}

int get_cubie_orientation(struct vector_model *cube, int i)
{
    struct vector_model solved = vector_model_new();
    int target = get_cubie_permutation(&solved, i);
    int type = get_cubie_type(cube, i);
    assert(type==CORNER || type==EDGE);
    int corner_move_set[] = {U, R2, F2, D, L2, B2};
    int edge_move_set[] = {U, R, F2, D, L, B2};
    int *move_set = type==CORNER ? corner_move_set : edge_move_set;

    int visited[12] = {0};
    struct vector_model stack[12] = {*cube};
    struct vector_model *top = 1+stack;
    while (top>stack) {
        cube = --top;

        int perm = get_cubie_permutation(cube, i);
        if (perm==target) {
            float theta = glm_quat_angle(cube->transforms[i]);
            float order = type==CORNER ? 3 : 2;
            return lroundf(theta / M_2_PI * order);
        }
        if (visited[perm]) continue;
        visited[perm] = 1;

        for (int i=0; i<6; ++i) {
            struct vector_model next = *cube;
            vector_model_move(&next, move_set[i]);
            *++top = next;
        }
    }
    assert(0);
}
