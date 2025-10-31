#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <SDL3/SDL.h>
#include <cglm/cglm.h>

#define NUM_CUBIES 27
#define EPSILON 1e-5
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define LOOP(i, j, k) for (int i=0; i<3; ++i) for (int j=0; j<3; ++j) for (int k=0; k<3; ++k)

enum cubie_type
{
    CORNER,
    EDGE,
    CENTRE,
};

enum move_type
{
    FACE_TURN,
    WIDE_MOVE,
    ROTATION,
    SLICE_MOVE,
};

static SDL_Condition *condition;
static SDL_Mutex *mutex;
static SDL_Thread *thread;
static int initialised;

static vec3 cubie_offsets[NUM_CUBIES];
static vec4 current_transforms[NUM_CUBIES];
static vec4 desired_transforms[NUM_CUBIES];

static int get_move_type(int move)
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

static int on_face(vec3 cubie, int face)
{
    return round(cubie[face%3])==(int)(face/3)*2-1;
}

static int can_move_cubie(vec3 cubie, int face, int type)
{
    int opposite_face=(face+3)%6;
    switch (type)
    {
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

static void get_cubie_position(int i, vec3 v)
{
    glm_quat_rotatev(desired_transforms[i], cubie_offsets[i], v);
}

static void move(int move)
{
    int face=move%6;
    int type=get_move_type(move);
    int dim=face%3;
    int amount=move/U2+1;
    int sign=1-face/3*2;

    SDL_LockMutex(mutex);
    for (int i=0; i<NUM_CUBIES; ++i)
    {
        vec3 v;
        get_cubie_position(i, v);
        if (!can_move_cubie(v, face, type)) continue;
        glm_vec3_zero(v);
        v[dim]=1;
        vec4 q;
        glm_quatv(q, sign*amount*glm_rad(90), v);
        glm_quat_mul(q, desired_transforms[i], desired_transforms[i]);
    }
    SDL_UnlockMutex(mutex);
}

static void scramble(void)
{
    int moves[100];
    make_scramble(moves, LENGTH(moves));
    for (int i=0; i<LENGTH(moves); ++i) move(moves[i]);
}

static void reset(void)
{
    glm_quat_identity_array(desired_transforms, NUM_CUBIES);
}

static GLuint new_shader(char *filename, GLuint type)
{
    FILE *f=fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    int length=ftell(f);
    char *buf=malloc(length+1);
    rewind(f);
    fread(buf, 1, length, f);
    buf[length]='\0';
    fclose(f);

    GLuint shader=glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar **)&buf, 0);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, buf);
        fprintf(stderr, "shader compilation error:\n%s\n", buf);
        exit(1);
    }

    free(buf);
    return shader;
}

static void set_mat4(GLuint program, char *location, mat4 m)
{
   	glUniformMatrix4fv(glGetUniformLocation(program, location), 1, GL_FALSE, m[0]);
}

static void set_mat4s(GLuint program, char *location, mat4 *m, int count)
{
   	glUniformMatrix4fv(glGetUniformLocation(program, location), count, GL_FALSE, *m[0]);
}

static int gui_thread(void *data)
{
    (void)data;

    SDL_LockMutex(mutex);

    LOOP(x, y, z) glm_vec3_copy((vec3){x-1, y-1, z-1}, cubie_offsets[x*9+y*3+z]);
    reset();

    vec2 vertices[] = {
        {-0.5f, -0.5f},
        { 0.5f, -0.5f},
        { 0.5f,  0.5f},
        {-0.5f,  0.5f},
    };

    vec3 colours[7] = {
        {0, 0, 0},
        {255, 255, 255}, // U
        {255, 0, 0}, // R
        {0, 128, 0}, // F
        {255, 255, 0}, // D
        {255, 165, 0}, // L
        {0, 0, 255}, // B
    };

    vec3 facelet_colours[3][3][3][6];
    LOOP(x, y, z)
    {
        for (int i=0; i<6; ++i)
            glm_vec3_copy(colours[on_face((vec3){x-1, y-1, z-1}, i) ? i+1 : 0], facelet_colours[x][y][z][i]);
    }

    glm_quat_identity_array(current_transforms, NUM_CUBIES);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window *window=SDL_CreateWindow("cube_solve", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context=SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // enable vsync

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint vs=new_shader("cube.vert", GL_VERTEX_SHADER);
    GLuint fs=new_shader("cube.frag", GL_FRAGMENT_SHADER);
    GLuint sp=glCreateProgram();
    glAttachShader(sp, vs);
    glAttachShader(sp, fs);
    glLinkProgram(sp);
    glUseProgram(sp);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void *)0);

    GLuint cubie_translation_vbo;
    glGenBuffers(1, &cubie_translation_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cubie_translation_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubie_offsets), cubie_offsets, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
    glVertexAttribDivisor(1, 6);

    GLuint facelet_colour_vbo;
    glGenBuffers(1, &facelet_colour_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, facelet_colour_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(facelet_colours), facelet_colours, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
    glVertexAttribDivisor(2, 1);

    mat4 facelet_transforms[6];
    for (int i=0; i<LENGTH(facelet_transforms); ++i)
    {
        int dim=i%3;
        vec3 v={0, 0, 0};
        v[dim]=(float)(i/3)-0.5f;
        glm_mat4_identity(facelet_transforms[i]);
        glm_translate(facelet_transforms[i], v);
        if (dim==2)
            continue;
        v[dim]=0, v[!dim]=1;
        glm_rotate(facelet_transforms[i], glm_rad(90), v);
    }
    set_mat4s(sp, "facelet_transforms", facelet_transforms, LENGTH(facelet_transforms));

    mat4 model=GLM_MAT4_IDENTITY_INIT;
    glm_rotate_x(model, glm_rad(30), model);
    glm_rotate_y(model, glm_rad(-30), model);
    glm_rotate_y(model, glm_rad(180), model);
    glm_rotate_z(model, glm_rad(-90), model);
    set_mat4(sp, "model", model);

    mat4 view=GLM_MAT4_IDENTITY_INIT;
    glm_translate(view, (vec3){0.0f, 0.0f, -4.0f});
    set_mat4(sp, "view", view);

    mat4 projection=GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(45.0f), (float)WINDOW_WIDTH/WINDOW_HEIGHT, 0.1f, 100.0f, projection);
    set_mat4(sp, "projection", projection);

    initialised = 1;
    SDL_BroadcastCondition(condition);
    SDL_UnlockMutex(mutex);
    for (;;)
    {
        // poll events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    goto exit;
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key)
                    {
                        case SDLK_F1: scramble(); break;
                        case SDLK_F2: reset(); break;
                        #define CASE(x, y) case SDLK_##x: move(y); break
                        CASE(1, S);
                        CASE(2, E);
                        // CASE(3, <);
                        // CASE(4, >);
                        CASE(5, M);
                        CASE(6, M);
                        // CASE(7, <);
                        // CASE(8, >);
                        CASE(9, E3);
                        CASE(0, S3);
                        CASE(Q, Z3);
                        CASE(W, B);
                        CASE(E, L3);
                        CASE(R, LW3);
                        CASE(T, X);
                        CASE(Y, X);
                        CASE(U, RW);
                        CASE(I, R);
                        CASE(O, B3);
                        CASE(P, Z);
                        CASE(A, Y3);
                        CASE(S, D);
                        CASE(D, L);
                        CASE(F, U3);
                        CASE(G, F3);
                        CASE(H, F);
                        CASE(J, U);
                        CASE(K, R3);
                        CASE(L, D3);
                        CASE(SEMICOLON, Y);
                        CASE(Z, DW);
                        CASE(X, M3);
                        CASE(C, UW3);
                        CASE(V, LW);
                        CASE(B, X3);
                        CASE(N, X3);
                        CASE(M, RW3);
                        CASE(COMMA, UW3);
                        CASE(PERIOD, M3);
                        CASE(SLASH, DW3);
                        #undef CASE
                    }
            }
        }

        SDL_LockMutex(mutex);
        mat4 cubie_transforms[NUM_CUBIES];
        for (int i=0; i<LENGTH(cubie_transforms); ++i)
        {
            float cos_theta=glm_quat_dot(current_transforms[i], desired_transforms[i]);
            if (ABS(cos_theta)>1-EPSILON)
                glm_quat_copy(desired_transforms[i], current_transforms[i]);
            float speed = 1.0f / 144 * 10; // todo: time between frames
            glm_quat_nlerp(current_transforms[i], desired_transforms[i], speed, current_transforms[i]);
            glm_quat_mat4(current_transforms[i], cubie_transforms[i]);
        }
        set_mat4s(sp, "cubie_transforms", cubie_transforms, LENGTH(cubie_transforms));
        SDL_UnlockMutex(mutex);

        // rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 6*NUM_CUBIES);
        SDL_GL_SwapWindow(window);
    }

    exit:
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    return 0;
}

static void gui(void)
{
    if (initialised) return;

    condition = SDL_CreateCondition();
    mutex = SDL_CreateMutex();
    thread = SDL_CreateThread(gui_thread, "gui", (void *)0);

    SDL_LockMutex(mutex);
    while (!initialised) SDL_WaitCondition(condition, mutex);
    SDL_UnlockMutex(mutex);
}

static void gui_show_moves(int *moves, int length)
{
    gui();
    for (int i=0; i<length; ++i) move(moves[i]), SDL_Delay(500);
}

static void gui_show_moves_fast(int *moves, int length)
{
    gui();
    for (int i=0; i<length; ++i) move(moves[i]);
}

static void gui_show_cube(cube x)
{
    gui();
    int moves[64];
    int length;
    thistlethwaite(x, moves, &length);
    reset();
    for (int i=0; i<length; ++i) move(moves[i]);
}

static void gui_wait_for_close(void)
{
    SDL_WaitThread(thread, NULL);
    SDL_DestroyMutex(mutex);
    SDL_DestroyCondition(condition);
    SDL_Quit();
    initialised = 0;
}
