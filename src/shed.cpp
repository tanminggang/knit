#include "shed.h"


shed::shed(ofImage oriImg)
{
    setupParameter();

    originalImg = oriImg;




    // initialize the sketch image who is use to perform the computation
    setSketch();

    w = sketchImg.getWidth();
    h = sketchImg.getHeight();

    initializeMask();
    setDisplayImg();


    // initialize drawer, it's a utility tool to access image
    drawer = imageDrawer();

    // set wheel that contains pins position
    setWheel();

    // set lines who represent all the strings possibilities betweens pins
    initializeLines();


    setEmptyResult();
    currentPinIdx1 = 0;
    nextPinIdx1 = -1;

}

void shed::setSketch(){

    int w = originalImg.getWidth();
    int h = originalImg.getHeight();

    sketchImg.clone(originalImg);

    int diff = 0;
    if( w > h ){
        diff = w - h;
        sketchImg.crop(diff/2, 0, h, h);
    } else {
        diff = h - w;
        sketchImg.crop(0, diff/2,  w, w);
    }

    sketchImg.update();
}

void shed::setEmptyResult(){

    result.allocate(w, h , OF_IMAGE_COLOR);
    result.setColor(ofColor::white);

}

void shed::setDisplayImg()
{

    displayImg.clone(originalImg);

    int diff = 0;
    if( w > h ){
        diff = w - h;
        displayImg.crop(diff/2, 0, h, h);
    } else {
        diff = h - w;
        displayImg.crop(0, diff/2,  w, w);
    }

    displayImg.update();

}

void shed::initializeMask()
{

    mask = new float * [w];

    for (int i = 0 ; i < w; i++){
        mask[i] = new float [h];
    }

    for (int x = 0; x < w; x++){
        for(int y = 0; y < h; y++){
            mask[x][y] = 1;
        }
    }

}

void shed::setWheel(){

    ofVec2f centerWheel = ofVec2f( w/2 , w/2 );
    float radius = (w-1)/2.0 ;    // we want not to be at border but inside

    wel = wheel(numberPinsP, radius, centerWheel);
}

void shed::initializeLines(){

    // initializing lines
    lines = new list<int*> * [wel.pinsNumber];
    for (int i = 0; i < wel.pinsNumber; i++) {
        lines[i] = new list<int*> [wel.pinsNumber];
    }

    for(int i = 0; i < wel.pinsNumber; i ++ ){
        for(int j = 0; j < wel.pinsNumber; j++){

            if ( i != j){
                lines[i][j] = drawer.getPixelIdxOfALineDDAAlgo(sketchImg, wel.pins[i], wel.pins[j]);
            } else {
                lines[i][j] = * (new list<int *>);  //case where the line is not well definite
            }

        }

    }
}

void shed::destroyLine(){


    int * temp;
    // FIXME

    for( int x = 0; x < wel.pinsNumber; x++)
    {
        for( int y= 0; y < wel.pinsNumber; y++){
            list<int * > l = lines[x][y];

            while (! l.empty()) {
                temp = l.front();
                delete [] temp;
                l.pop_front();

            }

        }
    }

    for (int i = 0; i < wel.pinsNumber; i++ ){ // is it correct ? needed?
         delete [] lines[i] ;
    }
    delete [] lines;

}

void shed::setupParameter(){

    shedParameter.setName("Shed Parameters");
    shedParameter.add(numberStringP.set("#strings",0, 0, 20000));
    shedParameter.add(numberPinsP.set("#pins",380, 4, 1200));
    shedParameter.add(algoOpacityP.set("algo opacity",56,0,255));
    shedParameter.add(drawOpacityP.set("draw opacity",36,0,255));

    shedParameter.add(stopIncrementationP.set("stop drawing", false));


    numberStringReal = numberStringP;
    algoOpacityReal = algoOpacityP;
}


shed::~shed(){

    for (int i = 0; i < wel.pinsNumber; i++ ){ // is it correct ? needed?
         delete [] lines[i] ;
    }
    delete [] lines;

}



void shed::checkchange(){

    if(numberPinsP != wel.pinsNumber){
        destroyLine();
        setWheel();
        setSketch();
        initializeLines();
        computeStringPath();
        drawString();
    }

    if(numberStringReal != numberStringP){
        numberStringReal = numberStringP;
        setSketch();
        computeStringPath();
        drawString();
    }

    if(algoOpacityReal != algoOpacityP){
        algoOpacityReal = algoOpacityP;
        setSketch();
        computeStringPath();
        drawString();
    }

    if(drawOpacityReal != drawOpacityP){
        drawOpacityReal = drawOpacityP;
        drawString();
    }


}




// decrease the lightness of the pixel who index are contain in l
void shed::decreaseDarkness(list<int*> l, float decreasingV) {

    ofColor color(decreasingV,decreasingV,decreasingV);
    drawer.incrementPixels(sketchImg, l, color);

    sketchImg.update();

}


// return the lightness score adaptation of the pixel contain in l
float shed::lineScore( list<int*> l){

    ofColor color;
    float lightness;

    int numberOfPixel = 0;
    float score = 0;

    for (std::list<int * >::iterator it = l.begin(); it != l.end(); it++)
    {
        color = sketchImg.getColor( (*it)[0], (*it)[1] );
        lightness = color.getLightness();
        score =  score   + (color.limit() - lightness) ;  // 255 lightness is white
        numberOfPixel = numberOfPixel + 1;

    }

    score = score / (float) numberOfPixel; // to not advantage long line

    return score;

}

float shed::lineScoreDelta( list<int*> l){

    ofColor color1;
    ofColor color2;

    float lightness1;
    float lightness2;


    int numberOfPixel = 0;
    float score = 0;

    float scoreTemp = 0;

    std::list<int * >::iterator next_it = l.begin();
    next_it++;

    for (std::list<int * >::iterator it = l.begin(); next_it != l.end(); it++)
    {
        color1 = sketchImg.getColor( (*it)[0], (*it)[1] );
        lightness1 = color1.getLightness();

        color2 = sketchImg.getColor( (*next_it)[0], (*next_it)[1] );
        lightness2 = color2.getLightness();

        scoreTemp =  color1.limit() - abs(lightness1 - lightness2) ;  // 255 lightness is white
        score += scoreTemp;

        numberOfPixel = numberOfPixel + 1;
        next_it++;

    }

    score = score / (float) numberOfPixel; // to not advantage long line

    return score;

}


// return the lightness score adaptation of the pixel contain in l
float shed::lineScoreWeighByMaskFactor( list<int*> l){

    ofColor color;
    float lightness;

    int numberOfPixel = 0;
    float score = 0;
    float tempScore = 0;

    for (std::list<int * >::iterator it = l.begin(); it != l.end(); it++)
    {
        color = sketchImg.getColor( (*it)[0], (*it)[1] );
        lightness = color.getLightness();

        // calculate score using negative value for white and positive value for black
        tempScore =  ( color.limit() - lightness) -  (color.limit() / 3 ) ;
        // multiply by the mask factor
        tempScore *= mask[(*it)[0]][(*it)[1]];

        score += tempScore;
        numberOfPixel = numberOfPixel + 1;

    }

    score = score / (float) numberOfPixel; // to not advantage long line

    return score;

}


// return the lightness score adaptation of the pixel contain in l
float shed::lineScoreWeighByMaskFactorCumulative( list<int*> l){

    ofColor color;
    float lightness;

    int numberOfPixel = 0;
    float score = 0;
    float tempScore = 0;

    for (std::list<int * >::iterator it = l.begin(); it != l.end(); it++)
    {
        color = sketchImg.getColor( (*it)[0], (*it)[1] );
        lightness = color.getLightness();

        // calculate score using negative value for white and positive value for black
        tempScore =  ( color.limit() - lightness)  ;
        // multiply by the mask factor
        tempScore *= mask[(*it)[0]][(*it)[1]];

        score += tempScore;
        numberOfPixel = numberOfPixel + 1;

    }

    score = score / (float) numberOfPixel; // to not advantage long line

    return score;

}


int shed::findNextBestPin(int pinIdx){
// go through all the pins and determine the next best one
// the Score function only need to be positive monotone


    float bestScore = INT_MIN;
    float tempScore = 0;

    int bestNextIdx = 0;
    int tempIdx = 0;


    for( int i = 0; i < wel.pinsNumber; i++){
        tempIdx = ( i + pinIdx) % wel.pinsNumber;
        tempScore = lineScoreWeighByMaskFactor(lines[pinIdx][tempIdx]);


        if (tempScore > bestScore){
            bestScore = tempScore;
            bestNextIdx = tempIdx;
        }
    }

    return bestNextIdx;

}




void shed::computeStringPath(){

    float decreaseV = algoOpacityReal;

    stringPath.clear();
    stringPath.push_back(0);

    int currentPinIdx = 0;
    int nextPinIdx = -1;

    for( int i = 0 ; i< numberStringReal; i++ ){
        nextPinIdx = findNextBestPin(currentPinIdx);
        // decrease the value of the pixel that are under the line

        decreaseDarkness(lines[currentPinIdx][nextPinIdx], decreaseV);

        stringPath.push_back(nextPinIdx);
        currentPinIdx = nextPinIdx;
    }

}

void shed::computeNextPinAndDrawOneString(){

    if (! stopIncrementationP){


        float decreaseV = algoOpacityP;
        int opacity = drawOpacityP;


        nextPinIdx1 = findNextBestPin(currentPinIdx1);

        // decrease the value of the pixel that are under the line
        decreaseDarkness(lines[currentPinIdx1][nextPinIdx1], decreaseV);


        // draw the line
        drawer.decreasePixels(result, lines[currentPinIdx1][nextPinIdx1], ofColor(opacity,opacity,opacity));

        std::cout << "step: " << currentPinIdx1 << ":" << nextPinIdx1 << std::endl;

        numberStringP++;

        // update the pin
        currentPinIdx1 = nextPinIdx1;


    }


}

void shed::randomifyNextPinAndDrawOneString(){


    std::cout << "  we are inside drawOne "<< std::endl;
    if (! stopIncrementationP){

        float decreaseV = algoOpacityP;
        int opacity = drawOpacityP;


        nextPinIdx1 = rand() % numberPinsP ;

        // decrease the value of the pixel that are under the line
        decreaseDarkness(lines[currentPinIdx1][nextPinIdx1], decreaseV);

        // draw the line
        drawer.decreasePixels(result, lines[currentPinIdx1][nextPinIdx1], ofColor(opacity,opacity,opacity));



        std::cout << "      we are computing in drawOne.    currentPin: "<<  currentPinIdx1 << ", nextPin: " << nextPinIdx1 << std::endl;

        numberStringP++;

        // update the pin
        currentPinIdx1 = nextPinIdx1;


    }


}








void shed::drawString(){

    result.allocate(sketchImg.getWidth(), sketchImg.getHeight(), OF_IMAGE_COLOR);
    result.setColor(ofColor::white);

    int opacity = drawOpacityReal;

    std::list<int>::iterator next_it = stringPath.begin();
    next_it++;

    for (std::list<int>::iterator it = stringPath.begin();  next_it  != stringPath.end()  ; it++)
    {
        drawer.decreasePixels(result, lines[*it][*next_it], ofColor(opacity,opacity,opacity));
        next_it++;

    }

}


/*
 * Go through a the display image and set the original image if there is no preference factor at these position
*/
void shed::computeLeftDisplayImg()
{

    setDisplayImg();

    ofColor color = ofColor::lavender;

    for( int x = 0 ; x < displayImg.getWidth(); x++ ){
        for ( int y = 0; y < displayImg.getHeight(); y++ ){
            if (mask[x][y] != 1) {
               displayImg.setColor(x,y, color);
            }

        }
    }

    displayImg.update();


}

/*
 *  Draw on mask using the brush given in parameter
 * The brush type must be of the form float **
 *
 * example you can give an array like this:
 *
 *  0 1 0   or  1 1
 *  1 1 1       0 3
 *  0 1 0
 *
 *
 *
*/
void shed::brushMask( int x, int y ,float ** brushType, int sizeBrush){

    int tempIdxX = 0;
    int tempIdxY = 0;

    if ((x > w -1 ) or (y > h -1) or (x < 0) or (y < 0)){
        std::cout << "brush mask must be call for pixel inside the sketch image" << std::endl;
        throw 20;
    }

    int middle = sizeBrush / 2;

    for( int i = 0; i < sizeBrush; i++ ){
        for( int j = 0; j < sizeBrush; j++ ){
            tempIdxX = x - middle + i;
            tempIdxY = y - middle + j;
            if  (!( (tempIdxX < 0) or (tempIdxX > w -1 ) or (tempIdxY < 0 ) or (tempIdxY > h -1 ) ) ) { // border case of the brush
                mask[tempIdxX][tempIdxY] *= brushType[i][j];
            }
         }

    }



}


ofImage shed::displayGrid()
{

    ofImage ret;




}

















