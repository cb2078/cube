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

    for (int i=0; i<NUM_CUBIES; ++i) {
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

void vector_model_new(struct vector_model *cube)
{
    int i=0;
    LOOP(x, y, z) glm_vec3_copy((vec3){x-1, y-1, z-1}, cube->cubies[i++]);
    glm_quat_identity_array(cube->transforms, NUM_CUBIES);
}

int get_cubie_type(struct vector_model *cube, int i)
{
    int num_zeros = 0;
    for (int j=0; j<3; ++j) num_zeros += cube->cubies[i][j]==0;
    return num_zeros;
}

int get_cubie_permutation(struct vector_model *cube, int i)
{
    struct vector_model solved;
    vector_model_new(&solved);
    vec3 v;
    get_cubie_position(cube, i, v);
    int type = get_cubie_type(cube, i);

    for (int j=0, count=0; j<NUM_CUBIES; ++j)
    {
        if (get_cubie_type(cube, j)!= type)
            continue;
        if (glm_vec3_eqv_eps(v, solved.cubies[j]))
            return count;
        ++count;
    }
    assert(0); // todo: unreachable
}

int get_cubie_orientation(struct vector_model *cube, int i)
{
    struct vector_model solved;
    vector_model_new(&solved);
    int target = get_cubie_permutation(&solved, i);
    int type = get_cubie_type(cube, i);
    assert(type==CORNER || type==EDGE);
    int corner_move_set[] = {U, R2, F2, D, L2, B2};
    int edge_move_set[] = {U, R, F2, D, L, B2};
    int *move_set = type==CORNER ? corner_move_set : edge_move_set;

    int visited[NUM_EDGES] = {0};
    visited[get_cubie_permutation(cube, i)] = 1;
    struct vector_model stack[NUM_EDGES] = {*cube};
    struct vector_model *top = 1+stack;
    while (top>stack)
    {
        struct vector_model cube = *--top;

        int perm = get_cubie_permutation(&cube, i);
        if (perm==target)
        {
            float theta = glm_quat_angle(cube.transforms[i]);
            vec3 axis;
            glm_quat_axis(cube.transforms[i], axis);
            // adjust theta if the axis of rotation points in the opposite direction
            if (glm_vec3_dot(cube.cubies[i], axis) < 0)
                theta = 2*M_PI-theta;
            int order = type==CORNER ? 3 : 2;
            int result = lroundf(theta / M_PI / 2 * order);
            return result % order;
        }

        for (int j=0; j<6; ++j)
        {
            struct vector_model next = cube;
            vector_model_move(&next, move_set[j]);
            int next_perm = get_cubie_permutation(&next, i);
            if (visited[next_perm])
                continue;
            else
                visited[next_perm] = 1;
            assert(top-stack<LENGTH(stack));
            *top++ = next;
        }
    }
    assert(0 && "exhausted search");
}
