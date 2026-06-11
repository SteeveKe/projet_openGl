#include "Camera.h"
#include "GrassGenerator.h"
#include "RockGenerator.h"
#include "TreeGenerator.h"
#include "MathUtils.h"
#include "TerrainGenerator.h"
#include "Mesh.h"
#include "ObjLoader.h"
#include "Shader.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <filesystem>
#include <iostream>
#include <vector>

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

Camera camera;

GLuint shaderProgram = 0;
GLuint grassShaderProgram = 0;
Mesh modelMesh;
Mesh grassMesh;
Mesh houseMesh;
RockField rocks;
TreeField trees;
TreeField trees2;
//terrain
Mesh terrainMesh;
int windowWidth = 2000;
int windowHeight = 2000;

// Position de la souris en UV [0, 1]. Elle sert a deplacer la lumiere.
float mouseX = 0.5f;
float mouseY = 0.5f;

constexpr int minDebugMode = 0;
constexpr int maxDebugMode = 10;
int debugMode = 0;

// Framebuffer custom : on rend le modele dedans avant le post-process.
GLuint framebuffer = 0;
GLuint framebufferColorTexture = 0;
GLuint framebufferNormalTexture = 0;
GLuint framebufferSobelMaskTexture = 0;
GLuint framebufferDepthTexture = 0;

const GLfloat clearColor[] = {0.72f, 0.86f, 0.96f, 1.0f};
const GLfloat clearNormal[] = {0.5f, 0.5f, 1.0f, 1.0f};

void reshape(int width, int height)
{
    windowWidth = width > 0 ? width : 1;
    windowHeight = height > 0 ? height : 1;

    resizeFrameBuffer(
        windowWidth,
        windowHeight,
        framebufferColorTexture,
        framebufferNormalTexture,
        framebufferSobelMaskTexture,
        framebufferDepthTexture,
        framebuffer
    );

    glViewport(0, 0, windowWidth, windowHeight);
    glutPostRedisplay();
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

void specialKeys(int key, int, int)
{
    switch (key) {
        case GLUT_KEY_LEFT:  camera.rotateLeft();  break;
        case GLUT_KEY_RIGHT: camera.rotateRight(); break;
        case GLUT_KEY_UP:
            debugMode = debugMode == maxDebugMode ? minDebugMode : debugMode + 1;
            std::cout << "debugMode = " << debugMode << std::endl;
            break;
        case GLUT_KEY_DOWN:
            debugMode = debugMode == minDebugMode ? maxDebugMode : debugMode - 1;
            std::cout << "debugMode = " << debugMode << std::endl;
            break;
    }
    glutPostRedisplay();
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
    const GLfloat clearSobelMask[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 2, clearSobelMask);
    glClear(GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const Mat4 projection = perspective(45.0f * 3.14159265f / 180.0f, aspect, 0.1f, 100.0f);
    //const Mat4 view = translate(0.0f, 0.0f, -3.0f);
    const Mat4 view = camera.viewMatrix();
    //const Mat4 model = multiply(rotateY(time * 1.0f), rotateX(time * 0.0f));
    const float linkX = 0.0f, linkZ = 0.0f;
    const Mat4 model = multiply(translate(linkX, terrainHeight(linkX, linkZ) + 0.3f, linkZ),
                       multiply(rotateY(3.14159265f), scale(0.3f)));
    const Mat4 mvp = multiply(projection, multiply(view, model));

    // Link
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"), 1, GL_FALSE, mvp.data());
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, model.data());
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
    glUniform1f(glGetUniformLocation(shaderProgram, "uSobelMask"), 1.0f);

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

    // maison
    constexpr float houseX = 4.0f;
    constexpr float houseZ = 3.0f;
    constexpr float houseScale = 2.0f;
    constexpr float houseBaseOffset = 0.51f * houseScale;
    const Mat4 houseModel = multiply(
        translate(houseX, terrainHeight(houseX, houseZ) + houseBaseOffset, houseZ),
        multiply(rotateY(-0.65f + 3.14159265f), scale(houseScale))
    );
    const Mat4 houseMvp = multiply(projection, multiply(view, houseModel));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"), 1, GL_FALSE, houseMvp.data());
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, houseModel.data());
    glUniform1f(glGetUniformLocation(shaderProgram, "uSobelMask"), 1.0f);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    glBindVertexArray(houseMesh.vao);
    for (const SubMesh& subMesh : houseMesh.subMeshes) {
        glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, subMesh.color.data());
        glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), subMesh.hasTexture ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, subMesh.hasTexture ? subMesh.texture : 0);
        glDrawArrays(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //terrain
    const Mat4 terrainModel = identity();
    const Mat4 terrainMvp = multiply(projection, multiply(view, terrainModel));
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"), 1, GL_FALSE, terrainMvp.data());
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, terrainModel.data());
    glUniform1f(glGetUniformLocation(shaderProgram, "uSobelMask"), 0.0f);
    glBindVertexArray(terrainMesh.vao);
    for (const SubMesh& subMesh : terrainMesh.subMeshes) {
        glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, subMesh.color.data());
        glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), 0);
        glDrawArrays(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount);
    }
    glBindVertexArray(0);

    // pierres
    glUniform1f(glGetUniformLocation(shaderProgram, "uSobelMask"), 1.0f);
    glBindVertexArray(rocks.mesh.vao);
    for (const Mat4& rockModel : rocks.transforms) {
        const Mat4 rockMvp = multiply(projection, multiply(view, rockModel));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"),   1, GL_FALSE, rockMvp.data());
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, rockModel.data());
        for (const SubMesh& subMesh : rocks.mesh.subMeshes) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, subMesh.color.data());
            glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), subMesh.hasTexture ? 1 : 0);
            if (subMesh.hasTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, subMesh.texture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
            }
            glDrawArrays(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount);
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // arbres
    glBindVertexArray(trees.mesh.vao);
    for (const Mat4& treeModel : trees.transforms) {
        const Mat4 treeMvp = multiply(projection, multiply(view, treeModel));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"),   1, GL_FALSE, treeMvp.data());
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, treeModel.data());
        for (const SubMesh& subMesh : trees.mesh.subMeshes) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, subMesh.color.data());
            glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), subMesh.hasTexture ? 1 : 0);
            if (subMesh.hasTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, subMesh.texture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
            }
            glDrawArrays(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount);
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // arbres2
    glBindVertexArray(trees2.mesh.vao);
    for (const Mat4& treeModel : trees2.transforms) {
        const Mat4 treeMvp = multiply(projection, multiply(view, treeModel));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMvp"),   1, GL_FALSE, treeMvp.data());
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, treeModel.data());
        for (const SubMesh& subMesh : trees2.mesh.subMeshes) {
            glUniform3fv(glGetUniformLocation(shaderProgram, "uColor"), 1, subMesh.color.data());
            glUniform1i(glGetUniformLocation(shaderProgram, "uUseTexture"), subMesh.hasTexture ? 1 : 0);
            if (subMesh.hasTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, subMesh.texture);
                glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
            }
            glDrawArrays(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount);
        }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // herbe
    const Mat4 grassModel = identity();
    const Mat4 grassMvp = multiply(projection, multiply(view, grassModel));

    glUseProgram(grassShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(grassShaderProgram, "uMvp"), 1, GL_FALSE, grassMvp.data());
    glUniformMatrix4fv(glGetUniformLocation(grassShaderProgram, "uModel"), 1, GL_FALSE, grassModel.data());
    glUniform1f(glGetUniformLocation(grassShaderProgram, "uTime"), time);

    glBindVertexArray(grassMesh.vao);
    for (const SubMesh& subMesh : grassMesh.subMeshes) {
        glUniform3fv(glGetUniformLocation(grassShaderProgram, "uColor"), 1, subMesh.color.data());
        glUniform1i(glGetUniformLocation(grassShaderProgram, "uUseTexture"), subMesh.hasTexture ? 1 : 0);
        if (subMesh.hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, subMesh.texture);
            glUniform1i(glGetUniformLocation(grassShaderProgram, "uTexture"), 0);
        }
        glDrawArraysInstanced(GL_TRIANGLES, subMesh.firstVertex, subMesh.vertexCount, grassMesh.instanceCount);
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

    // masque indiquant les pixels de Link
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, framebufferSobelMaskTexture);
    glUniform1i(glGetUniformLocation(screenShaderProgram, "sobelMaskTexture"), 3);

    glUniform1i(glGetUniformLocation(screenShaderProgram, "debugMode"), debugMode);
    glUniform3f(glGetUniformLocation(screenShaderProgram, "uSunDirection"), -0.45f, 0.80f, 0.35f);
    glUniform3f(glGetUniformLocation(screenShaderProgram, "uSunColor"), 1.0f, 0.92f, 0.78f);
    glUniform1f(glGetUniformLocation(screenShaderProgram, "uSunIntensity"), 0.85f);
    glUniform1f(glGetUniformLocation(screenShaderProgram, "uAmbientLight"), 0.25f);
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

    //modelMesh = loadObjModel(std::filesystem::path(MODEL_DIR) / "IronMan.obj");
    grassShaderProgram = createShaderProgram(
        std::filesystem::path(SHADER_DIR) / "grass.vert",
        std::filesystem::path(SHADER_DIR) / "grass.frag"
    );

    // herbe
    //grassMesh = generateGrass(400000, 6.0f, std::filesystem::path(MODEL_DIR) / "grass.png");
    grassMesh = generateGrass(100000, 6.0f, std::filesystem::path(MODEL_DIR) / "grass.png");

    //terrain
    terrainMesh = generateTerrain(100, 100.0f);

    // link
    //modelMesh = loadObjModel(std::filesystem::path(MODEL_DIR) / "IronMan.obj");
    modelMesh = loadObjModel(std::filesystem::path(MODEL_DIR) / "link2/link.obj");
    houseMesh = loadObjModel(std::filesystem::path(MODEL_DIR) / "house/Biskupin_Tower.obj");

    srand(42);
    rocks = generateRocks(40, 4.0f, std::filesystem::path(MODEL_DIR) / "rock2/Modeling Clay Rock/clayrock.obj");
    //trees  = generateTrees(0,  15.0f, std::filesystem::path(MODEL_DIR) / "tree/叫ぶ木 (Debug)/o00_7100.obj");
    //trees2 = generateTrees(0, 30.0f, std::filesystem::path(MODEL_DIR) / "tree2/Willow Tree/treewillow_tslocator_gmdc.obj");

    glEnable(GL_DEPTH_TEST);

    generateFrameBuffer(
        windowWidth,
        windowHeight,
        framebufferColorTexture,
        framebufferNormalTexture,
        framebufferSobelMaskTexture,
        framebufferDepthTexture,
        framebuffer
    );

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(update);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);
    glutSpecialFunc(specialKeys);
    glutMainLoop();

    return 0;
}
