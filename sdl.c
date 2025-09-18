#include "common.h"

GLuint new_shader(char *filename, GLuint type)
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

void set_mat4(GLuint program, char *location, mat4 m)
{
   	glUniformMatrix4fv(glGetUniformLocation(program, location), 1, GL_FALSE, m[0]);
}

void set_mat4s(GLuint program, char *location, mat4 *m, int count)
{
   	glUniformMatrix4fv(glGetUniformLocation(program, location), count, GL_FALSE, *m[0]);
}

int main(void)
{
    struct vector_model cube;
    vector_model_new(&cube);

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
    LOOP(x, y, z) {
        for (int i=0; i<6; ++i)
            glm_vec3_copy(colours[on_face((vec3){x-1, y-1, z-1}, i) ? i+1 : 0], facelet_colours[x][y][z][i]);
    }

    vec4 current_transforms[NUM_CUBIES];
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube.cubies), cube.cubies, GL_STATIC_DRAW);
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
    for (int i=0; i<LENGTH(facelet_transforms); ++i) {
        int dim=i%3;
        vec3 v={0, 0, 0};
        v[dim]=(int)(i/3)-0.5f;
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

    for (;;) {
        // poll events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    goto exit;
                case SDL_EVENT_KEY_DOWN:
                    switch (event.key.key) {
                        #define CASE(x, y) case SDLK_##x: vector_model_move(&cube, y); break
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

        mat4 cubie_transforms[NUM_CUBIES];
        for (int i=0; i<LENGTH(cubie_transforms); ++i) {
            float cos_theta=glm_quat_dot(current_transforms[i], cube.transforms[i]);
            if (ABS(cos_theta)<EPSILON)
                continue;
            float speed = 1.0f / 144 * 10; // todo: time between frames
            glm_quat_nlerp(current_transforms[i], cube.transforms[i], speed, current_transforms[i]);
            glm_quat_mat4(current_transforms[i], cubie_transforms[i]);
        }
        set_mat4s(sp, "cubie_transforms", cubie_transforms, LENGTH(cubie_transforms));

        // rendering
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 6*NUM_CUBIES);
        SDL_GL_SwapWindow(window);
    }

    exit:
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
