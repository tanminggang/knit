#ifndef IMAGEDRAWER_H
#define IMAGEDRAWER_H

#include "ofMain.h"


class imageDrawer
{
public:
    imageDrawer();


    bool isPointOnLine(ofVec2f p1, ofVec2f startLine, ofVec2f endLine, float tolerance);

    void drawALine(ofImage& img, ofVec2f l1, ofVec2f l2, float tolerance = 0.5);

    void drawPins(ofImage &img, ofVec2f* pins, int pinsNumber);

    void printListIdx(list<int *> l);

    void drawPixels(ofImage &img, list<int *> l, ofColor color = ofColor(23,56,77));

    void getPixelIdxOfALineDDAAlgo(list<int*> * l, ofVec2f l1, ofVec2f l2);


    void setPixelIdxAndIntensityOfAThickLine(list<int *> *l, ofVec2f l1, ofVec2f l2, float width);


    float percentageOfPixelAboveLine(float localStartY, float deltaV) ;
    void percentTester();

    void incrementPixels(ofImage &img, list<int *> l, ofColor color);

    void decreasePixels(ofImage &img, list<int *> l, ofColor color);
    void increasePixels(ofImage &img, list<int *> l, ofColor color);

    void freeListOf2Int(list<int *> * l);




    void drawPixelsWithIntensity(ofImage &img, list<int *> l);

    void increasePixelsWithIntensity(ofImage &img, list<int *> l);

};

#endif // IMAGEDRAWER_H
