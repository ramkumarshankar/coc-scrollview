/**
 *
 *      ┌─┐╔═╗┌┬┐┌─┐
 *      │  ║ ║ ││├┤
 *      └─┘╚═╝─┴┘└─┘
 *   ┌─┐┌─┐╔╗╔┬  ┬┌─┐┌─┐
 *   │  ├─┤║║║└┐┌┘├─┤└─┐
 *   └─┘┴ ┴╝╚╝ └┘ ┴ ┴└─┘
 *
 * Copyright (c) 2014-2016 Code on Canvas Pty Ltd, http://CodeOnCanvas.cc
 *
 * This software is distributed under the MIT license
 * https://tldrlegal.com/license/mit-license
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code
 *
 **/

#include "Grid.h"

Grid::Grid() {
    width = 4096;
    height = 4096;
}

Grid::~Grid() {
    //
}

float Grid::getWidth() {
    return width;
}

float Grid::getHeight() {
    return height;
}

void Grid::draw() {
    int numOfRows = 20;
    int numOfCols = 20;
    float rowSize = height / numOfRows;
    float colSize = width / numOfCols;

    ofSetColor(0);

    ofNoFill();
    ofSetColor(ofColor::red);
    ofSetLineWidth(2.0);
    ofDrawRectangle(0, 0, width, height);

    for(int yi=1; yi<numOfRows; yi++) {
        int x0 = 0;
        int x1 = width;
        int y0 = yi * rowSize;
        int y1 = y0;
        ofDrawLine(x0, y0, x1, y1);
    }

    for(int xi=1; xi<numOfCols; xi++) {
        int x0 = xi * colSize;
        int x1 = x0;
        int y0 = 0;
        int y1 = height;
        ofDrawLine(x0, y0, x1, y1);
    }

    ofFill();
    ofSetColor(255);
    ofSetLineWidth(1.0);
}
