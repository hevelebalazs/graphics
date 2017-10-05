#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "graphics/texture.h"

static int winWidth = 800;
static int winHeight = 600;

struct {
    GLuint vertexBuffer;
    GLuint elementBuffer;

    GLuint texture;

    GLuint vertexShader;
    GLuint fragmentShader;

    GLuint program;

    struct {
        GLint texture;
    } uniforms;

    struct {
        GLuint position;
    } attributes;
} Resources;

static GLfloat VertexData[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};

static GLushort ElementData[] = {0, 1, 2, 3};

// create a GL buffer of custom type
// returns ID of created buffer
static GLuint bufferId(GLenum type, void *buffer, GLsizei size) {
    GLuint index;
    glGenBuffers(1, &index);
    glBindBuffer(type, index);
    glBufferData(type, size, buffer, GL_STATIC_DRAW);
    return index;
}

// get contents of a file as a 0 terminated string
// if no such file, 0 will be returned
static char* fileLoad(char *name) {
    if (!name) return 0;

    FILE *file = fopen(name, "rb");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char*)malloc(size + 1);

    if (!content) {
        printf("Cannot allocate buffer for loading file [%s]!\n", name);
        fclose(file);
        return 0;
    }

    fread(content, 1, size, file);
    content[size] = 0;

    return content;
}

static void showInfo(
    GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
) {
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, 0, log);
    printf("  %s\n", log);
    free(log);
}

// create a GLSL shader of custom type from file
// returns ID of created shader
// returns 0 on error
static GLuint shaderId(GLenum type, char *filename) {
    char *source = fileLoad(filename);

    if (!source) return 0;

    int length = strlen(source);

    int id = glCreateShader(type);
    glShaderSource(id, 1, (const GLchar**)&source, &length);
    free(source);

    glCompileShader(id);

    int res;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    if (!res) {
        printf("Failed to compile shader [%s]!\n", filename);
        
        showInfo(id, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// create shader program from vertex shader and fragment shader
// returns ID of created program
// returns 0 on error
static GLuint programId(GLuint vertexShader, GLuint fragmentShader) {
    GLuint id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    glLinkProgram(id);

    int res;
    glGetProgramiv(id, GL_LINK_STATUS, &res);

    if (!res) {
        printf("Failed to link shader program!\n");
        showInfo(id, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(id);
        return 0;
    }

    return id;
}

// create OpenGL texture object given a texture in memory
// returns ID of created texture
// returns 0 on error
static GLuint textureId(int width, int height, void *data) {
    GLuint id;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexImage2D(
        GL_TEXTURE_2D, 0,
        GL_RGB8,
        width, height, 0,
        GL_BGR, GL_UNSIGNED_BYTE,
        data
    );

    return id;
}

// init graphics resources
// returns 0 on failure
// returns 1 on success
char initResources() {
    // buffers
    Resources.vertexBuffer = bufferId(GL_ARRAY_BUFFER, VertexData, sizeof(VertexData));
    Resources.elementBuffer = bufferId(GL_ELEMENT_ARRAY_BUFFER, ElementData, sizeof(ElementData));

    // shaders
    Resources.vertexShader = shaderId(GL_VERTEX_SHADER, "shader/vertex.glsl");
    Resources.fragmentShader = shaderId(GL_FRAGMENT_SHADER, "shader/fragment.glsl");

    if (!Resources.vertexShader) return 0;
    if (!Resources.fragmentShader) return 0;

    // program
    Resources.program = programId(Resources.vertexShader, Resources.fragmentShader); 

    if (!Resources.program) return 0;

    // texture
    int texwidth = 400;
    int texheight = 400;

    char *texdata = calloc(3 * texwidth * texheight, sizeof(unsigned char)); 
    texPerlinGrad2d(texwidth, texheight, texdata);
    texMarble2d(texwidth, texheight, texdata);
    // texFireGradient2d(texwidth, texheight, texdata);

    Resources.texture = textureId(texwidth, texheight, texdata); 
    
    if (!Resources.texture) return 0;

    // uniforms
    Resources.uniforms.texture = glGetUniformLocation(Resources.program, "texture");

    // attributes
    Resources.attributes.position = glGetAttribLocation(Resources.program, "position");

    return 1;
}

// update things
void update() {
}

// draw things
void draw(SDL_Window *window) {
    glUseProgram(Resources.program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Resources.texture);
    glUniform1i(Resources.uniforms.texture, 0);

    glBindBuffer(GL_ARRAY_BUFFER, Resources.vertexBuffer);
    glVertexAttribPointer(
        Resources.attributes.position,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GLfloat) * 2,
        0
    );
    glEnableVertexAttribArray(Resources.attributes.position);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Resources.elementBuffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,
        4,
        GL_UNSIGNED_SHORT,
        0
    );

    glDisableVertexAttribArray(Resources.attributes.position);

    SDL_GL_SwapWindow(window);
}

int main(int argc, char **argv) {
    setbuf(stdout, 0);

    // init SDL
    int res;
    res = SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    
    if (res != 0) {
        printf("Unable to init SDL!\n");
        return 1;
    }

    // create an SDL window
    SDL_Window *window;

    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow(
        "SDL Window",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        winWidth, winHeight,
        flags
    );

    SDL_GL_CreateContext(window);

    // init GLEW
    glewExperimental = 0;
    res = glewInit();

    if (res != 0) {
        printf("GLEW error: %s\n", glewGetErrorString(res));
        return 1;
    }

    if (!GLEW_VERSION_2_0) {
        printf("OpenGL 2.0 not available!\n");
        return 1;
    }

    char quit = 0;

    // parse events
    void events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }
    }

    res = initResources();

    if (!res) {
        printf("Cannot init resources!\n");
        return 1;
    }
    
    // main loop
    while (!quit) {
        events();
        update();
        draw(window);
    }

    return 0;
}
