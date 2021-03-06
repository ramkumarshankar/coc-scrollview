/**
 *
 *      ┌─┐╔═╗┌┬┐┌─┐
 *      │  ║ ║ ││├┤
 *      └─┘╚═╝─┴┘└─┘
 *   ┌─┐┌─┐╔╗╔┬  ┬┌─┐┌─┐
 *   │  ├─┤║║║└┐┌┘├─┤└─┐
 *   └─┘┴ ┴╝╚╝ └┘ ┴ ┴└─┘
 *
 * Copyright (c) 2016 Code on Canvas Pty Ltd, http://CodeOnCanvas.cc
 *
 * This software is distributed under the MIT license
 * https://tldrlegal.com/license/mit-license
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code
 *
 **/

#include "cocScrollView.h"

namespace coc {

//--------------------------------------------------------------
static float const kEasingStop = 0.001;

//--------------------------------------------------------------
ScrollView::ScrollView() {

    windowRect.set(0, 0, 0, 0);
    contentRect.set(0, 0, 0, 0);

    bUserInteractionEnabled = false;
    bPinchZoomEnabled = true;
    bPinchZoomSupported = false;

    scrollRect.set(0, 0, 0, 0);
    scrollRectEased.set(0, 0, 0, 0);
    scrollRectAnim0.set(0, 0, 0, 0);
    scrollRectAnim1.set(0, 0, 0, 0);

    scrollEasing = 0.5;
    bounceBack = 1.0;

    dragVelDecay = 0.9;
    bDragging = false;

    zoomDownDist = 0;
    zoomMoveDist = 0;
    bZooming = false;

    animTimeStart = 0.0;
    animTimeTotal = 0.0;
    bAnimating = false;

    bDoubleTapZoomEnabled = true;
    doubleTapZoomRangeMin = 0.0;
    doubleTapZoomRangeMax = 1.0;
    doubleTapZoomIncrement = 1.0;
    doubleTapZoomIncrementTimeInSec = 0.2;
    doubleTapRegistrationTimeInSec = 0.25;
    doubleTapRegistrationDistanceInPixels = 22;

    scale = 1.0;
    scaleDown = 1.0;
    scaleMin = 1.0;
    scaleMax = 1.0;
}

ScrollView::~ScrollView() {
    //
}

//--------------------------------------------------------------
void ScrollView::setUserInteraction(bool bEnable) {
    if(bUserInteractionEnabled == bEnable) {
        return;
    }
    if(bUserInteractionEnabled == true) {

        bUserInteractionEnabled = false;
        setUserInteractionOff();

    } else {

        bUserInteractionEnabled = true;
        setUserInteractionOn();
    }
}

bool ScrollView::getUserInteraction() {
    return bUserInteractionEnabled;
}

void ScrollView::setPinchZoom(bool value) {
    bPinchZoomEnabled = value;
}

void ScrollView::setScrollEasing(float value) {
    scrollEasing = value;
}

void ScrollView::setBounceBack(float value) {
    bounceBack = value;
}

void ScrollView::setDragVelocityDecay(float value) {
    dragVelDecay = value;
}

void ScrollView::setDoubleTapZoom(bool value) {
    bDoubleTapZoomEnabled = value;
}

void ScrollView::setDoubleTapZoomRangeMin(float value) {
    doubleTapZoomRangeMin = value;
}

void ScrollView::setDoubleTapZoomRangeMax(float value) {
    doubleTapZoomRangeMax = value;
}

void ScrollView::setDoubleTapZoomIncrement(float value) {
    doubleTapZoomIncrement = value;
}

void ScrollView::setDoubleTapZoomIncrementTimeInSec(float value) {
    doubleTapZoomIncrementTimeInSec = value;
}

void ScrollView::setDoubleTapRegistrationTimeInSec(float value) {
    doubleTapRegistrationTimeInSec = value;
}

void ScrollView::setDoubleTapRegistrationDistanceInPixels(float value) {
    doubleTapRegistrationDistanceInPixels = value;
}

//--------------------------------------------------------------
void ScrollView::setup() {
    if(windowRect.isEmpty() == true) {

        // TODO: show error message + instruct on how to fix.
        // TODO: default window size should be based actual window dimensions.

        int windowSizeDefault = 512;

        coc::Rect rect;
        rect.set(0, 0, windowSizeDefault, windowSizeDefault);
        setWindowRect(rect);
    }

    if(contentRect.isEmpty() == true) {
        setContentRect(windowRect);
    }

    reset();
}

void ScrollView::reset() {
    touchPoints.clear();

    dragDownPos = glm::vec2(0, 0);
    dragMovePos = glm::vec2(0, 0);
    dragMovePosPrev = glm::vec2(0, 0);
    dragVel = glm::vec2(0, 0);
    bDragging = false;

    zoomDownPos = glm::vec2(0, 0);
    zoomMovePos = glm::vec2(0, 0);
    zoomMovePosPrev = glm::vec2(0, 0);
    bZooming = false;

    animTimeStart = 0.0;
    animTimeTotal = 0.0;
    bAnimating = false;

    scale = scaleMin;
    scaleDown = scaleMin;

    float w = contentRect.getW() * scale;
    float h = contentRect.getH() * scale;
    scrollRect.setX(0);
    scrollRect.setY(0);
    scrollRect.setW(w);
    scrollRect.setH(h);

    scrollRect = scrollRectEased = getRectContainedInWindowRect(scrollRect);

    mat = getMatrixForRect(scrollRect);
}

//--------------------------------------------------------------
void ScrollView::setWindowRect(const coc::Rect & rect) {
    if(windowRect == rect) {
        return;
    }
    windowRect = rect;
}

void ScrollView::setContentRect(const coc::Rect & rect) {
    if(contentRect == rect) {
        return;
    }
    contentRect = rect;
}

//--------------------------------------------------------------
void ScrollView::fitContentToWindow(cocAspectRatioMode aspectRatioMode) {
    float sx = windowRect.getW() / contentRect.getW();
    float sy = windowRect.getH() / contentRect.getH();

    if(aspectRatioMode == COC_ASPECT_RATIO_KEEP) {
        scaleMin = coc::min(sx, sy);
    } else if(aspectRatioMode == COC_ASPECT_RATIO_KEEP_BY_EXPANDING) {
        scaleMin = coc::max(sx, sy);
    } else {
        scaleMin = 1.0;
    }

    scaleMin = coc::min(scaleMin, 1.0);
    scaleMax = 1.0;
    scale = scaleMin;
}

//--------------------------------------------------------------
void ScrollView::setScale(float value) {
    scale = value;
    scale = coc::clamp(scale, scaleMin, scaleMax);
}

void ScrollView::setScaleMin(float value) {
    scaleMin = value;
    scale = coc::clamp(scale, scaleMin, scaleMax);
}

void ScrollView::setScaleMax(float value) {
    scaleMax = value;
    scale = coc::clamp(scale, scaleMin, scaleMax);
}

//--------------------------------------------------------------
float ScrollView::getScale() {
    return scale;
}

float ScrollView::getScaleMin() {
    return scaleMin;
}

float ScrollView::getScaleMax() {
    return scaleMax;
}

//--------------------------------------------------------------
void ScrollView::setZoom(float value) {
    float zoom = coc::clamp(value, 0.0, 1.0);
    scale = zoomToScale(zoom);
}

float ScrollView::getZoom() {
    float zoom = scaleToZoom(scale);
    return zoom;
}

bool ScrollView::isZoomed() {
    float zoom = getZoom();
    return (zoom > 0.0);
}

bool ScrollView::isZoomedInMax() {
    float zoom = getZoom();
    return (zoom == 1.0);
}

bool ScrollView::isZoomedOutMax() {
    float zoom = getZoom();
    return (zoom == 0.0);
}

//--------------------------------------------------------------
float ScrollView::zoomToScale(float value) {
    if(scaleMin == scaleMax) {
        return scaleMin;
    }
    return coc::map(value, 0.0, 1.0, scaleMin, scaleMax, true);
}

float ScrollView::scaleToZoom(float value) {
    if(scaleMin == scaleMax) {
        return 0.0;
    }
    return coc::map(value, scaleMin, scaleMax, 0.0, 1.0, true);
}

//--------------------------------------------------------------
void ScrollView::zoomToMin(const glm::vec2 & screenPoint, float timeSec) {
    zoomTo(screenPoint, scaleMin, timeSec);
}

void ScrollView::zoomToMax(const glm::vec2 & screenPoint, float timeSec) {
    zoomTo(screenPoint, scaleMax, timeSec);
}

void ScrollView::zoomTo(const glm::vec2 & screenPoint, float zoom, float timeSec) {
    bool bAnimate = animStart(timeSec);

    scrollRectAnim0 = scrollRect;
    scrollRectAnim1 = scrollRect;
    scrollRectAnim1 = getRectZoomedAtScreenPoint(scrollRectAnim1, screenPoint, zoom);
    scrollRectAnim1 = getRectContainedInWindowRect(scrollRectAnim1);

    if(bAnimate == false) {
        scrollRect = scrollRectEased = scrollRectAnim1;
    }
}

void ScrollView::zoomToContentPointAndPositionAtScreenPoint(const glm::vec2 & contentPoint,
        const glm::vec2 & screenPoint,
        float zoom,
        float timeSec) {
    bool bAnimate = animStart(timeSec);

    scrollRectAnim0 = scrollRect;
    scrollRectAnim1 = scrollRect;
    scrollRectAnim1 = getRectWithContentPointAtScreenPoint(scrollRectAnim1, contentPoint, screenPoint);
    scrollRectAnim1 = getRectZoomedAtScreenPoint(scrollRectAnim1, screenPoint, zoom);
    scrollRectAnim1 = getRectContainedInWindowRect(scrollRectAnim1);

    if(bAnimate == false) {
        scrollRect = scrollRectEased = scrollRectAnim1;
    }
}

void ScrollView::moveContentPointToScreenPoint(const glm::vec2 & contentPoint,
        const glm::vec2 & screenPoint,
        float timeSec) {
    bool bAnimate = animStart(timeSec);

    scrollRectAnim0 = scrollRect;
    scrollRectAnim1 = scrollRect;
    scrollRectAnim1 = getRectWithContentPointAtScreenPoint(scrollRectAnim1, contentPoint, screenPoint);

    if(bAnimate == false) {
        scrollRect = scrollRectEased = scrollRectAnim1;
    }
}


bool ScrollView::animStart(float animTimeInSec) {
    bAnimating = true;

    animTimeStart = coc::getTimeElapsed();
    animTimeTotal = coc::max(animTimeInSec, 0.0);

    if(animTimeTotal < 0.001) {
        animTimeTotal = 0;
        bAnimating = false;
    }

    return bAnimating;
}

//--------------------------------------------------------------
void ScrollView::setScrollPositionX(float x, bool bEase) {
    dragCancel();
    zoomCancel();

    float px = coc::clamp(x, 0.0, 1.0);
    float rx = windowRect.getX() - (scrollRect.getW() - windowRect.getW()) * px;
    scrollRect.setX(rx);
    if(bEase == false) {
        scrollRectEased.setX(rx);
    }
}

void ScrollView::setScrollPositionY(float y, bool bEase) {
    dragCancel();
    zoomCancel();

    float py = coc::clamp(y, 0.0, 1.0);
    float ry = windowRect.getY() - (scrollRect.getH() - windowRect.getH()) * py;
    scrollRect.setY(ry);
    if(bEase == false) {
        scrollRectEased.setY(ry);
    }
}

void ScrollView::setScrollPosition(float x, float y, bool bEase) {
    setScrollPositionX(x, bEase);
    setScrollPositionY(y, bEase);
}

glm::vec2 ScrollView::getScrollPosition() {
    return glm::vec2(scrollRectEased.getX(), scrollRectEased.getY());
}

glm::vec2 ScrollView::getScrollPositionNorm() {
    glm::vec2 scrollPosEasedNorm;

    float dx = windowRect.getW() - scrollRect.getW();
    float dy = windowRect.getH() - scrollRect.getH();
    if(dx >= 0) {
        scrollPosEasedNorm.x = 0;
    } else {
        scrollPosEasedNorm.x = coc::map(scrollRectEased.getX(), dx, 0.0, 1.0, 0.0, true);
    }
    if(dy >= 0) {
        scrollPosEasedNorm.y = 0;
    } else {
        scrollPosEasedNorm.y = coc::map(scrollRectEased.getY(), dy, 0.0, 1.0, 0.0, true);
    }

    return scrollPosEasedNorm;
}

//--------------------------------------------------------------
const coc::Rect & ScrollView::getWindowRect() {
    return windowRect;
}

const coc::Rect & ScrollView::getContentRect() {
    return contentRect;
}

const coc::Rect & ScrollView::getScrollRect() {
    return scrollRect;
}

const glm::mat4x4 & ScrollView::getMatrix() {
    return mat;
}

const float * ScrollView::getMatrixPtr() {
    return &mat[0][0];
}

//--------------------------------------------------------------
void ScrollView::update() {

    if(bAnimating == true) {

        float timeNow = coc::getTimeElapsed();
        float progress = coc::map(timeNow, animTimeStart, animTimeStart + animTimeTotal, 0.0, 1.0, true);
        bAnimating = (progress < 1.0);

        coc::Rect rect = coc::RectLerp(scrollRectAnim0, scrollRectAnim1, progress);
        scrollRect = rect;

        scale = scrollRect.getW() / contentRect.getH();

    } else {

        //==========================================================
        // dragging.
        //==========================================================

        if(bDragging == true || bZooming == true) {

            if(bDragging == true) {

                dragVel = dragMovePos - dragMovePosPrev;
                dragMovePosPrev = dragMovePos;

            } else if(bZooming == true) {

                dragVel = zoomMovePos - zoomMovePosPrev;
                zoomMovePosPrev = zoomMovePos;
            }

            float rx = scrollRect.getX() + dragVel.x;
            float ry = scrollRect.getY() + dragVel.y;

            scrollRect.setX(rx);
            scrollRect.setY(ry);

        } else {

            dragVel *= dragVelDecay;
            if(coc::abs(dragVel.x) < kEasingStop) {
                dragVel.x = 0;
            }
            if(coc::abs(dragVel.y) < kEasingStop) {
                dragVel.y = 0;
            }
            bool bAddVel = true;
            bAddVel = bAddVel && (coc::abs(dragVel.x) > 0);
            bAddVel = bAddVel && (coc::abs(dragVel.y) > 0);
            if(bAddVel == true) {

                float rx = scrollRect.getX() + dragVel.x;
                float ry = scrollRect.getY() + dragVel.y;

                scrollRect.setX(rx);
                scrollRect.setY(ry);
            }
        }

        //==========================================================
        // zooming.
        //==========================================================

        if(bZooming == true) {

            float zoomUnitDist = glm::vec2(windowRect.getW(), windowRect.getH()).size(); // diagonal.
            float zoomRange = scaleMax - scaleMin;
            float zoomDiff = 0;

            if(bPinchZoomSupported == true) {

                zoomDiff = zoomMoveDist - zoomDownDist;
                zoomDiff *= 4;

            } else {

                zoomDiff = zoomMovePos.x - zoomDownPos.x;
            }

            float zoom = coc::map(zoomDiff, -zoomUnitDist, zoomUnitDist, -zoomRange, zoomRange, true);

            scale = scaleDown + zoom;
            scale = coc::max(scale, 0.0);

            if(scale < scaleMin) {
                scale = scaleMin;
            } else if(scale > scaleMax) {
                scale = scaleMax;
            }

            float zoomScale = scaleToZoom(scale);
            coc::Rect rect = getRectZoomedAtScreenPoint(scrollRect, zoomMovePos, zoomScale);
            scrollRect = rect;
        }
    }

    scrollRect = getRectContainedInWindowRect(scrollRect, bounceBack);

    //==========================================================
    // apply easing to scrollRect.
    //==========================================================

    scrollRectEased.lerp(scrollRect, scrollEasing);

    if(coc::abs(scrollRect.getX() - scrollRectEased.getX()) < kEasingStop) {
        scrollRectEased.setX(scrollRect.getX());
    }
    if(coc::abs(scrollRect.getY() - scrollRectEased.getY()) < kEasingStop) {
        scrollRectEased.setY(scrollRect.getY());
    }
    if(coc::abs(scrollRect.getW() - scrollRectEased.getW()) < kEasingStop) {
        scrollRectEased.setW(scrollRect.getW());
    }
    if(coc::abs(scrollRect.getH() - scrollRectEased.getH()) < kEasingStop) {
        scrollRectEased.setH(scrollRect.getH());
    }

    mat = getMatrixForRect(scrollRectEased);
}

//-------------------------------------------------------------- the brains!
coc::Rect ScrollView::getRectContainedInWindowRect(const coc::Rect & rectToContain,
        float easing) {

    coc::Rect rect = rectToContain;
    coc::Rect boundingRect = windowRect;
    coc::Rect contentRectMin = contentRect;

    float rw = contentRectMin.getW() * scaleMin;
    float rh = contentRectMin.getH() * scaleMin;

    contentRectMin.setW(rw);
    contentRectMin.setH(rh);

    if(rect.getW() < windowRect.getW()) {
        boundingRect.setX(windowRect.getX() + (windowRect.getW() - contentRectMin.getW()) * 0.5);
        boundingRect.setW(contentRectMin.getW());
    }
    if(rect.getH() < windowRect.getH()) {
        boundingRect.setY(windowRect.getY() + (windowRect.getH() - contentRectMin.getH()) * 0.5);
        boundingRect.setH(contentRectMin.getH());
    }

    float x0 = boundingRect.getX() - coc::max(rect.getW() - boundingRect.getW(), 0.0);
    float x1 = boundingRect.getX();
    float y0 = boundingRect.getY() - coc::max(rect.getH() - boundingRect.getH(), 0.0);
    float y1 = boundingRect.getY();

    if(rect.getX() < x0) {
        float rx = rect.getX() + (x0 - rect.getX()) * easing;
        rect.setX(rx);
        if(coc::abs(x0 - rect.getX()) < kEasingStop) {
            rect.setX(x0);
        }
    } else if(rect.getX() > x1) {
        float rx = rect.getX() + (x1 - rect.getX()) * easing;
        rect.setX(rx);
        if(coc::abs(x1 - rect.getX()) < kEasingStop) {
            rect.setX(x1);
        }
    }

    if(rect.getY() < y0) {
        float ry = rect.getY() + (y0 - rect.getY()) * easing;
        rect.setY(ry);
        if(coc::abs(y0 - rect.getY()) < kEasingStop) {
            rect.setY(y0);
        }
    } else if(rect.getY() > y1) {
        float ry = rect.getY() + (y1 - rect.getY()) * easing;
        rect.setY(ry);
        if(coc::abs(y1 - rect.getY()) < kEasingStop) {
            rect.setY(y1);
        }
    }

    return rect;
}

coc::Rect ScrollView::getRectZoomedAtScreenPoint(const coc::Rect & rect,
        const glm::vec2 & screenPoint,
        float zoom) {

    float zoomScale = zoomToScale(zoom);

    glm::vec2 contentPoint = getContentPointAtScreenPoint(rect, screenPoint);

    glm::vec2 p0(0, 0);
    glm::vec2 p1(contentRect.getW(), contentRect.getH());
    p0 -= contentPoint;
    p1 -= contentPoint;
    p0 *= zoomScale;
    p1 *= zoomScale;
    p0 += screenPoint;
    p1 += screenPoint;

    coc::Rect rectNew;
    rectNew.setX(p0.x);
    rectNew.setY(p0.y);
    rectNew.setW(p1.x - p0.x);
    rectNew.setH(p1.y - p0.y);

    return rectNew;
}

coc::Rect ScrollView::getRectWithContentPointAtScreenPoint(const coc::Rect & rect,
        const glm::vec2 & contentPoint,
        const glm::vec2 & screenPoint) {

    glm::vec2 contentScreenPoint = getScreenPointAtContentPoint(rect, contentPoint);
    glm::vec2 contentPointToScreenPointDifference = screenPoint - contentScreenPoint;

    coc::Rect rectNew;
    rectNew = scrollRect;

    float rx = rectNew.getX() + contentPointToScreenPointDifference.x;
    float ry = rectNew.getY() + contentPointToScreenPointDifference.y;

    rectNew.setX(rx);
    rectNew.setY(ry);

    return rectNew;
}

glm::mat4x4 ScrollView::getMatrixForRect(const coc::Rect & rect) {

    float rectScale = rect.getW() / contentRect.getW();

    glm::mat4x4 rectMat;
    rectMat = glm::translate(rectMat, glm::vec3(rect.getX(), rect.getY(), 0.0));
    rectMat = glm::scale(rectMat, glm::vec3(rectScale, rectScale, 1.0));

    return rectMat;
}

glm::vec2 ScrollView::getContentPointAtScreenPoint(const glm::vec2 & screenPoint) {

    return getContentPointAtScreenPoint( scrollRectEased, screenPoint);
}

glm::vec2 ScrollView::getContentPointAtScreenPoint(const coc::Rect & rect, const glm::vec2 & screenPoint) {

    glm::vec2 contentPoint;
    contentPoint.x = coc::map(screenPoint.x, rect.getX(), rect.getX() + rect.getW(), 0, contentRect.getW(), true);
    contentPoint.y = coc::map(screenPoint.y, rect.getY(), rect.getY() + rect.getH(), 0, contentRect.getH(), true);
    return contentPoint;
}

glm::vec2 ScrollView::getScreenPointAtContentPoint(const coc::Rect & rect, const glm::vec2 & contentPoint) {

    glm::vec2 screenPoint;
    screenPoint.x = coc::map(contentPoint.x, 0, contentRect.getW(), rect.getX(), rect.getX() + rect.getW(), true);
    screenPoint.y = coc::map(contentPoint.y, 0, contentRect.getH(), rect.getY(), rect.getY() + rect.getH(), true);
    return screenPoint;
}

//--------------------------------------------------------------
void ScrollView::dragDown(const glm::vec2 & point) {
    dragDownPos = dragMovePos = dragMovePosPrev = point;
    dragVel = glm::vec2(0, 0);

    bDragging = true;
    bAnimating = false;
}

void ScrollView::dragMoved(const glm::vec2 & point) {
    dragMovePos = point;
}

void ScrollView::dragUp(const glm::vec2 & point) {
    dragMovePos = point;

    bDragging = false;
}

void ScrollView::dragCancel() {
    dragVel = glm::vec2(0, 0);

    bDragging = false;
}

//--------------------------------------------------------------
void ScrollView::zoomDown(const glm::vec2 & point, float pointDist) {
    if(bPinchZoomEnabled == false) {
        return;
    }

    zoomDownPos = zoomMovePos = zoomMovePosPrev = point;
    zoomDownDist = zoomMoveDist = pointDist;

    scaleDown = scale;

    bZooming = true;
    bAnimating = false;
}

void ScrollView::zoomMoved(const glm::vec2 & point, float pointDist) {
    if(bPinchZoomEnabled == false) {
        return;
    }

    zoomMovePos = point;
    zoomMoveDist = pointDist;
}

void ScrollView::zoomUp(const glm::vec2 & point, float pointDist) {
    if(bPinchZoomEnabled == false) {
        return;
    }

    zoomMovePos = point;
    zoomMoveDist = pointDist;

    bZooming = false;
}

void ScrollView::zoomCancel() {
    bZooming = false;
}

//--------------------------------------------------------------
void ScrollView::mouseMoved(int x, int y) {
    //
}

void ScrollView::mousePressed(int x, int y, int button) {
    if(button == 0) {

        touchDown(x, y, 0);

    } else if(button == 2) {

        touchDown(x, y, 0);
        touchDown(x, y, 2);
    }
}

void ScrollView::mouseDragged(int x, int y, int button) {
    touchMoved(x, y, button);
}

void ScrollView::mouseReleased(int x, int y, int button) {
    touchUp(x, y, button);
}

//--------------------------------------------------------------
void ScrollView::touchDown(int x, int y, int id) {
    bool bHit = windowRect.isInside(x, y);
    if(bHit == false) {
        return;
    }

    ScrollViewTouchPoint touchPointNew;
    touchPointNew.touchPos = glm::vec2(x, y);
    touchPointNew.touchID = id;
    touchPointNew.touchDownTimeInSec = coc::getTimeElapsed();

    //---------------------------------------------------------- double tap.
    glm::vec2 touchPointDiff = touchPointNew.touchPos - touchDownPointLast.touchPos;
    float touchTimeDiff = touchPointNew.touchDownTimeInSec - touchDownPointLast.touchDownTimeInSec;

    touchDownPointLast = touchPointNew;

    bool bDoubleTap = true;
    bDoubleTap = bDoubleTap && (touchTimeDiff < doubleTapRegistrationTimeInSec);
    bDoubleTap = bDoubleTap && (touchPointDiff.size() < doubleTapRegistrationDistanceInPixels);

    if(bDoubleTapZoomEnabled == true &&
            bDoubleTap == true) {

        dragCancel();
        zoomCancel();

        touchPoints.clear();
        touchDownPointLast.touchPos = glm::vec2(0, 0);
        touchDownPointLast.touchDownTimeInSec = 0.0;

        touchDoubleTap(x, y, id);
        return;
    }

    //----------------------------------------------------------
    if(touchPoints.size() >= 2) {
        // max 2 touches.
        return;
    }

    touchPoints.push_back(touchPointNew);

    if(touchPoints.size() == 1) {

        zoomCancel();
        dragDown(touchPoints[0].touchPos);

    } else if(touchPoints.size() == 2) {

        glm::vec2 tp0(touchPoints[0].touchPos);
        glm::vec2 tp1(touchPoints[1].touchPos);
        glm::vec2 tmp = (tp1 - tp0) * glm::vec2(0.5, 0.5) + tp0;
        float dist = (tp1 - tp0).size();

        dragCancel();
        zoomDown(tmp, dist);
    }
}

void ScrollView::touchMoved(int x, int y, int id) {
    int touchIndex = -1;
    for(int i=0; i<touchPoints.size(); i++) {
        ScrollViewTouchPoint & touchPoint = touchPoints[i];
        if(touchPoint.touchID == id) {
            touchPoint.touchPos.x = x;
            touchPoint.touchPos.y = y;
            touchIndex = i;
            break;
        }
    }

    if(touchIndex == -1) {
        return;
    }

    if(touchPoints.size() == 1) {

        dragMoved(touchPoints[0].touchPos);

    } else if(touchPoints.size() == 2) {

        glm::vec2 tp0(touchPoints[0].touchPos);
        glm::vec2 tp1(touchPoints[1].touchPos);
        glm::vec2 tmp = (tp1 - tp0) * glm::vec2(0.5, 0.5) + tp0;
        float dist = (tp1 - tp0).size();

        zoomMoved(tmp, dist);
    }
}

void ScrollView::touchUp(int x, int y, int id) {
    int touchIndex = -1;
    for(int i=0; i<touchPoints.size(); i++) {
        ScrollViewTouchPoint & touchPoint = touchPoints[i];
        if(touchPoint.touchID == id) {
            touchPoint.touchPos.x = x;
            touchPoint.touchPos.y = y;
            touchIndex = i;
            break;
        }
    }

    if(touchIndex == -1) {
        return;
    }

    if(touchPoints.size() == 1) {

        dragUp(touchPoints[0].touchPos);

    } else if(touchPoints.size() == 2) {

        glm::vec2 tp0(touchPoints[0].touchPos);
        glm::vec2 tp1(touchPoints[1].touchPos);
        glm::vec2 tmp = (tp1 - tp0) * glm::vec2(0.5, 0.5) + tp0;
        float dist = (tp1 - tp0).size();

        zoomUp(tmp, dist);
    }

    touchPoints.clear();
}

void ScrollView::touchDoubleTap(int x, int y, int id) {
    if(bDoubleTapZoomEnabled == false) {
        return;
    }

    bool bHit = windowRect.isInside(x, y);
    if(bHit == false) {
        return;
    }

    glm::vec2 touchPoint(x, y);

    float zoomCurrent = getZoom();
    float zoomTarget = 0.0;

    bool bZoomedInMax = (zoomCurrent == doubleTapZoomRangeMax);
    if(bZoomedInMax == true) {
        zoomTarget = doubleTapZoomRangeMin; // zoom all the way out.
    } else {
        zoomTarget = zoomCurrent + doubleTapZoomIncrement;
    }
    zoomTarget = coc::clamp(zoomTarget, doubleTapZoomRangeMin, doubleTapZoomRangeMax);

    float zoomTimeSec = coc::abs(zoomTarget - zoomCurrent);
    zoomTimeSec *= doubleTapZoomIncrementTimeInSec;

    zoomTo(touchPoint, zoomTarget, zoomTimeSec);
}

void ScrollView::touchCancelled(int x, int y, int id) {
    //
}

}
