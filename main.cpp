#include "MathUtils.h"
#include "Mesh.h"
#include "ObjLoader.h"
#include "Shader.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <filesystem>
#include <iostream>

#ifndef SHADER_DIR
#define SHADER_DIR "src"
#endif

#ifndef MODEL_DIR
#define MODEL_DIR "models"
#endif

namespace {

GLuint shaderProgram = 0;
Mesh modelMesh;
int windowWidth = 1200;
int windowHeight = 1200;

void reshape(int width, int height)
{
    windowWidth = width;
    windowHeight = height > 0 ? height : 1;
    glViewport(0, 0, windowWidth, windowHeight);
}

void update()
{
    glutPostRedisplay();
}

void display()
{
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float time = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f;
    const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const Mat4 projection = perspective(45.0f * 3.14159265f / 180.0f, aspect, 0.1f, 100.0f);
    const Mat4 view = translate(0.0f, 0.0f, -4.0f);
    const Mat4 model = multiply(rotateY(time), rotateX(0.0f));
    const Mat4 mvp = multiply(projection, multiply(view, model));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"), 1, GL_FALSE, mvp.data());
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, model.data());
    glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, modelMesh.color.data());
    glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), modelMesh.hasTexture ? 1 : 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    if (modelMesh.hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, modelMesh.texture);
    }

    glBindVertexArray(modelMesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, modelMesh.vertexCount);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glutSwapBuffers();
}

}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("OpenGL avec shaders");

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur initialisation GLEW" << std::endl;
        return 1;
    }

    shaderProgram = createShaderProgram(
        std::filesystem::path(SHADER_DIR) / "vertex.vert",
        std::filesystem::path(SHADER_DIR) / "fragment.frag"
    );
    modelMesh = loadObjModel(std::filesystem::path(MODEL_DIR) / "BigDrill3-TEST.obj");
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(update);
    glutMainLoop();

    return 0;
}
