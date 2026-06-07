#include "MathUtils.h"
#include "Mesh.h"
#include "ObjLoader.h"
#include "Shader.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <filesystem>
#include <iostream>

#include "FrameBuffer.h"

#ifndef SHADER_DIR
#define SHADER_DIR "src/shader"
#endif

#ifndef MODEL_DIR
#define MODEL_DIR "models"
#endif

namespace {

GLuint screenVao = 0;
GLuint screenShaderProgram = 0;

GLuint shaderProgram = 0;
Mesh modelMesh;

int windowWidth = 2000;
int windowHeight = 2000;

// Position de la souris en UV [0, 1]. Elle sert a deplacer la lumiere.
float mouseX = 0.5f;
float mouseY = 0.5f;

constexpr int minDebugMode = 0;
constexpr int maxDebugMode = 9;
int debugMode = 9;

// Framebuffer custom : on rend le modele dedans avant le post-process.
GLuint framebuffer = 0;
GLuint framebufferColorTexture = 0;
GLuint framebufferNormalTexture = 0;
GLuint framebufferDepthTexture = 0;

const GLfloat clearColor[] = {0.10f, 0.10f, 0.12f, 1.0f};
const GLfloat clearNormal[] = {0.5f, 0.5f, 1.0f, 1.0f};

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

void mouseMove(int x, int y)
{
    mouseX = static_cast<float>(x) / static_cast<float>(windowWidth);
    mouseY = 1.0f - static_cast<float>(y) / static_cast<float>(windowHeight);
}

void specialKey(int key, int, int)
{
    if (key == GLUT_KEY_UP) {
        debugMode = debugMode == maxDebugMode ? minDebugMode : debugMode + 1;
        std::cout << "debugMode = " << debugMode << std::endl;
        glutPostRedisplay();
    } else if (key == GLUT_KEY_DOWN) {
        debugMode = debugMode == minDebugMode ? maxDebugMode : debugMode - 1;
        std::cout << "debugMode = " << debugMode << std::endl;
        glutPostRedisplay();
    }
}

void createScreenTriangle()
{
    glGenVertexArrays(1, &screenVao);
}

void display()
{
    const float time = static_cast<float>(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f;

    //ecrit dans 3 textures
    //colorTexture
    //normalTexture
    //depthTexture

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);

    glClearBufferfv(GL_COLOR, 0, clearColor);
    glClearBufferfv(GL_COLOR, 1, clearNormal);
    glClear(GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const Mat4 projection = perspective(45.0f * 3.14159265f / 180.0f, aspect, 0.1f, 100.0f);
    const Mat4 view = translate(0.0f, 0.0f, -3.0f);
    const Mat4 model = multiply(rotateY(time * 1.0f), rotateX(time * 0.0f));
    const Mat4 mvp = multiply(projection, multiply(view, model));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"), 1, GL_FALSE, mvp.data());
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, model.data());
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    glBindVertexArray(modelMesh.vao);
    for (const SubMesh& subMesh : modelMesh.subMeshes) {
        glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, subMesh.color.data());
        glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), subMesh.hasTexture ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        if (subMesh.hasTexture) {
            glBindTexture(GL_TEXTURE_2D, subMesh.texture);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glDrawArrays(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);

    glDisable(GL_DEPTH_TEST);
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(screenShaderProgram);

    //couleur brute du modele
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
    glUniform1i(glGetUniformLocation(screenShaderProgram, "colorTexture"), 0);

    //normales encodees en [0, 1]
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebufferNormalTexture);
    glUniform1i(glGetUniformLocation(screenShaderProgram, "normalTexture"), 1);

    //profondeur
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, framebufferDepthTexture);
    glUniform1i(glGetUniformLocation(screenShaderProgram, "depthTexture"), 2);

    //Debug modes fleche haut/bas pour de 0 a 9
    glUniform1i(glGetUniformLocation(screenShaderProgram, "debugMode"), debugMode);

    //Lumiere controlee par la souris
    glUniform2f(glGetUniformLocation(screenShaderProgram, "uLightPosition"), mouseX, mouseY);
    glUniform1f(glGetUniformLocation(screenShaderProgram, "uLightHeight"), 0.35f);

    glBindVertexArray(screenVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
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

    screenShaderProgram = createShaderProgram(
        std::filesystem::path(SHADER_DIR) / "screen.vert",
        std::filesystem::path(SHADER_DIR) / "screen.frag"
    );
    createScreenTriangle();

    modelMesh = loadObjModel(std::filesystem::path(MODEL_DIR) / "IronMan.obj");

    glEnable(GL_DEPTH_TEST);

    generateFrameBuffer(
        windowWidth,
        windowHeight,
        framebufferColorTexture,
        framebufferNormalTexture,
        framebufferDepthTexture,
        framebuffer
    );

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(update);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);
    glutSpecialFunc(specialKey);
    glutMainLoop();

    return 0;
}
