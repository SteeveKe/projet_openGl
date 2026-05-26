#include <GL/glew.h>
#include <GL/freeglut.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifndef SHADER_DIR
#define SHADER_DIR "src"
#endif

GLuint shaderProgram = 0;
GLuint vao = 0;
GLuint vbo = 0;

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Impossible d'ouvrir le shader : " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Compile un shader
GLuint compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();

    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Erreur compilation shader :\n" << log << std::endl;
    }

    return shader;
}

// Assemble le vertex shader et le fragment shader dans un programme
GLuint createShaderProgram()
{
    const std::string vertexSource = readFile(std::string(SHADER_DIR) + "/vertex.vert");
    const std::string fragmentSource = readFile(std::string(SHADER_DIR) + "/fragment.frag");

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Erreur linkage programme shader :\n" << log << std::endl;
    }

    // Apres le linkage, le programme garde le resultat compile.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void createTriangle()
{
    const float vertices[] = {
        // x, y
        -0.6f, -0.4f,
         0.6f, -0.4f,
         0.0f,  0.6f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // layout(location = 0) dans vertex.vert correspond a deux floats : x et y.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void display()
{
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // On active les shaders, puis on dessine les 3 sommets du triangle.
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 1200);
    glutCreateWindow("OpenGL avec shaders");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur initialisation GLEW" << std::endl;
        return 1;
    }

    shaderProgram = createShaderProgram();
    createTriangle();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
