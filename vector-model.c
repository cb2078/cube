#include "common.h"

int on_face(vec3 cubie, int face)
{
    return round(cubie[face%3])==(int)(face/3)*2-1;
}

int can_move_cubie(vec3 cubie, int face, int type)
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

void move(struct vector_cube *cube, int move)
{
    int face=move%6;
    int type=get_move_type(move);
    int dim=face%3;
    int amount=move/U2+1;
    int sign=1-face/3*2;

    for (int i=0; i<27; ++i) {
        vec3 v;
        glm_quat_rotatev(cube->transforms[i], cube->cubies[i], v);
        if (!can_move_cubie(v, face, type))
            continue;
        glm_vec3_zero(v);
        v[dim]=1;
        vec4 q;
        glm_quatv(q, sign*amount*glm_rad(90), v);
        glm_quat_mul(q, cube->transforms[i], cube->transforms[i]);
    }
}

void vector_cube_init(struct vector_cube *cube)
{
    int i=0;
    LOOP(x, y, z) glm_vec3_copy((vec3){x-1, y-1, z-1}, cube->cubies[i++]);
    glm_quat_identity_array(cube->transforms, 27);
}
