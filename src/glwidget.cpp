/* CSC 473 Summer 2014
 * Assignment #1
 *
 * Ryan Guy
 * V00484803
 *
 */

/*
    CSC 486 Assignment 1
    Ryan Guy
    V00484803
*/

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtOpenGL>
#include <QCursor>
#include <GL/glu.h>
#include "glwidget.h"
#include "glm/glm.hpp"
#include "quaternion.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

//! [0]
GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

    screenWidth = 1200;
    screenHeight = 600;

    //initializealize update/draw timers
    float updatesPerSecond = 30;

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateSimulation()));
    updateTimer->start(1000.0/updatesPerSecond);

    deltaTimer = new QTime();
    deltaTimer->start();

    // Initialize camera
    glm::vec3 pos = glm::vec3(12, 10.0, 12);
    glm::vec3 dir = glm::normalize(glm::vec3(-pos.x, -pos.y, -pos.z));
    camera = camera3d(pos, dir, screenWidth, screenHeight,
                      60.0, 0.5, 100.0);

    // for updating camera movement
    isMovingForward = false;
    isMovingBackward = false;
    isMovingRight = false;
    isMovingLeft = false;
    isMovingUp = false;
    isMovingDown = false;
    isRotatingRight = false;
    isRotatingLeft = false;
    isRotatingUp = false;
    isRotatingDown = false;
    isRollingRight = false;
    isRollingLeft = false;

    // simulation system
    minDeltaTimeModifier = 0.125;
    maxDeltaTimeModifier = 1.0;
    deltaTimeModifier = maxDeltaTimeModifier;
    runningTime = 0.0;
    currentFrame = 0;


    double radius = 0.1;
    fluidSim = SPHFluidSimulation(radius);

    std::vector<glm::vec3> points;
    int nx = 20;
    int ny = 20;
    int nz = 10;
    float pad = 1.05*radius;
    float stagger = 0.1*radius;
    glm::vec3 r = glm::vec3(0.1, 0.1, 0.1);
    for (int k=0; k<nz; k++) {
        for (int j=0; j<ny; j++) {
            for (int i=0; i<nx; i++) {
                float sx = (2*((float)rand()/RAND_MAX)-1)*stagger;
                float sy = (2*((float)rand()/RAND_MAX)-1)*stagger;
                float sz = (2*((float)rand()/RAND_MAX)-1)*stagger;

                glm::vec3 p;
                p = r + glm::vec3(i*pad+sx, j*pad+sy, k*pad+sz);
                points.push_back(p);
            }
        }
    }
    fluidSim.addFluidParticles(points);

    fluidSim.setBounds(0.0, 2.0, 0.0, 8.0, 0.0, 1.0);

    /*
    nx = 100;
    ny = 30;
    nz = 8;
    pad = 0.6*radius;
    std::vector<glm::vec3> obstaclePoints;
    r = glm::vec3(0.0, -0.1, 3.0);
    for (int k=0; k<nz; k++) {
        for (int j=0; j<ny; j++) {
            for (int i=0; i<nx; i++) {
                glm::vec3 p;
                p = r + glm::vec3(i*pad, j*pad, k*pad);
                obstaclePoints.push_back(p);
                p = r + glm::vec3(i*pad + 0.5*pad, j*pad + 0.5*pad, k*pad + 0.5*pad);
                obstaclePoints.push_back(p);

                p = r + glm::vec3(i*pad, j*pad, k*pad -3.0);
                obstaclePoints.push_back(p);
                p = r + glm::vec3(i*pad + 0.5*pad, j*pad + 0.5*pad, k*pad + 0.5*pad-3.0);
                obstaclePoints.push_back(p);
            }
        }
    }

    nx = 8;
    ny = 30;
    nz = 70;
    r = glm::vec3(0.0, -0.1, 0.3);
    for (int k=0; k<nz; k++) {
        for (int j=0; j<ny; j++) {
            for (int i=0; i<nx; i++) {
                glm::vec3 p;
                p = r + glm::vec3(i*pad, j*pad, k*pad);
                obstaclePoints.push_back(p);
                p = r + glm::vec3(i*pad + 0.5*pad, j*pad + 0.5*pad, k*pad + 0.5*pad);
                obstaclePoints.push_back(p);

                p = r + glm::vec3(i*pad+4.0, j*pad, k*pad);
                obstaclePoints.push_back(p);
                p = r + glm::vec3(i*pad + 0.5*pad + 4.0, j*pad + 0.5*pad, k*pad + 0.5*pad);
                obstaclePoints.push_back(p);
            }
        }
    }
    int id = fluidSim.addObstacleParticles(obstaclePoints);
    */


}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(screenWidth, screenHeight);
}

void GLWidget::initializeGL()
{
    static const GLfloat lightPos[4] = { 20.0f, 20.0f, 20.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


// move and rotate camera
void GLWidget::updateCameraMovement(float dt) {
    float camSpeed = 10.0;
    float camRotSpeed = 2.0;

    if (isMovingForward) { camera.moveForward(camSpeed * dt); }
    if (isMovingBackward) { camera.moveBackward(camSpeed * dt); }
    if (isMovingRight) { camera.moveRight(camSpeed * dt); }
    if (isMovingLeft) { camera.moveLeft(camSpeed * dt); }
    if (isMovingUp) { camera.moveUp(camSpeed * dt); }
    if (isMovingDown) { camera.moveDown(camSpeed * dt); }
    if (isRotatingRight) { camera.rotateRight(camRotSpeed * dt); }
    if (isRotatingLeft) { camera.rotateLeft(camRotSpeed * dt); }
    if (isRotatingUp) { camera.rotateUp(camRotSpeed * dt); }
    if (isRotatingDown) { camera.rotateDown(camRotSpeed * dt); }
    if (isRollingRight) { camera.rollRight(camRotSpeed * dt); }
    if (isRollingLeft) { camera.rollLeft(camRotSpeed * dt); }
}

void GLWidget::initializeSimulation() {
}

void GLWidget::stopSimulation() {
}

void GLWidget::updateSimulation() {
    // find delta time
    float dt = (float) deltaTimer->elapsed() / 1000;
    runningTime = runningTime + dt;
    deltaTimer->restart();
    updateCameraMovement(dt);

    dt *= deltaTimeModifier;  // speed of simulation

    dt = 1.0/30.0;

    // fluidSim test
    fluidSim.update(dt);
    if (isRendering) {
        writeFrame();
    } else {
        updateGL();
    }
}

void GLWidget::writeFrame() {
    updateGL();

    std::string s = std::to_string(currentFrame);
    s.insert(s.begin(), 6 - s.size(), '0');
    s = "test_render/" + s + ".png";
    bool r = saveFrameToFile(QString::fromStdString(s));

    qDebug() << r << QString::fromStdString(s);

    currentFrame += 1;
}


// drawn when animation is running
void GLWidget::drawAnimation() {

}

bool GLWidget::saveFrameToFile(QString fileName) {
    GLubyte *data = (GLubyte*)malloc(4*(int)screenWidth*(int)screenHeight);
    if( data ) {
        glReadPixels(0, 0, screenWidth, screenHeight,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    QImage image((int)screenWidth, (int)screenHeight, QImage::Format_RGB32);
    for (int j=0; j<screenHeight; j++) {
        for (int i=0; i<screenWidth; i++) {
            int idx = 4*(j*screenWidth + i);
            char r = data[idx+0];
            char g = data[idx+1];
            char b = data[idx+2];

            // sets 32 bit pixel at (x,y) to yellow.
            //uchar *p = image.scanLine(j) + i;
            //*p = qRgb(255, 0, 0);
            QRgb value = qRgb(r, g, b);
            image.setPixel(i, screenHeight-j-1, value);
        }
    }
    bool r = image.save(fileName);
    free(data);

    return r;
}

void GLWidget::paintGL()
{
    camera.set();
    utils::drawGrid();

    float scale = 2.0;
    glm::mat4 scaleMat = glm::transpose(glm::mat4(scale, 0.0, 0.0, 0.0,
                                                  0.0, scale, 0.0, 0.0,
                                                  0.0, 0.0, scale, 0.0,
                                                  0.0, 0.0, 0.0, 1.0));
    glPushMatrix();
    glMultMatrixf((GLfloat*)&scaleMat);
    fluidSim.draw();
    glPopMatrix();

    camera.unset();
}

void GLWidget::resizeGL(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    camera.resize((float)width, (float)height);
    updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    // turn on camera movement
    isMovingForward = event->key()  == Qt::Key_W;
    isMovingBackward = event->key() == Qt::Key_S;
    isMovingRight = event->key()    == Qt::Key_D;
    isMovingLeft = event->key()     == Qt::Key_A;
    isMovingUp = event->key()       == Qt::Key_T;
    isMovingDown = event->key()     == Qt::Key_G;

    isRotatingRight = event->key()  == Qt::Key_E;
    isRotatingLeft = event->key()   == Qt::Key_Q;
    isRotatingUp = event->key()     == Qt::Key_F;
    isRotatingDown = event->key()   == Qt::Key_R;
    isRollingRight = event->key()   == Qt::Key_X;
    isRollingLeft = event->key()    == Qt::Key_Z;

    // slow down simulation
    if (event->key() == Qt::Key_C) {
        deltaTimeModifier = minDeltaTimeModifier;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    // turn off camera movement
    isMovingForward = event->key()  == Qt::Key_W ? false : isMovingForward;
    isMovingBackward = event->key() == Qt::Key_S ? false : isMovingBackward;
    isMovingRight = event->key()    == Qt::Key_D ? false : isMovingRight;
    isMovingLeft = event->key()     == Qt::Key_A ? false : isMovingLeft;
    isMovingUp = event->key()       == Qt::Key_T ? false : isMovingUp;
    isMovingDown = event->key()     == Qt::Key_G ? false : isMovingDown;

    isRotatingRight = event->key()  == Qt::Key_E ? false : isRotatingRight;
    isRotatingLeft = event->key()   == Qt::Key_Q ? false : isRotatingLeft;
    isRotatingDown = event->key()   == Qt::Key_R ? false : isRotatingDown;
    isRotatingUp = event->key()     == Qt::Key_F ? false : isRotatingUp;
    isRollingRight = event->key()   == Qt::Key_X ? false : isRollingRight;
    isRollingLeft = event->key()    == Qt::Key_Z ? false : isRollingLeft;

    if (event->key() == Qt::Key_C) {
        deltaTimeModifier = maxDeltaTimeModifier;
    }

    if (event->key() == Qt::Key_G) {
        fluidSim.setDampingConstant(0.5);
        fluidSim.setBounds(0.0, 2.0, 0.0, 8.0, 0.0, 6.0);
        isRendering = true;
    }

}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    (void)event;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    (void)event;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    (void)event;
}








