/*
 * KinectPlayer.h
 *
 *  Created on: 14/12/2010
 *      Author: arturo
 */

#pragma once

#include "ofMain.h"
#include "ofxBase3DVideo.h"
#include "ofxVectorMath.h"
#include "ofxKinectCalibration.h"

class ofxKinectPlayer: public ofxBase3DVideo {
public:
	ofxKinectPlayer();
	virtual ~ofxKinectPlayer();

	void setup(const string & file, bool color=false);
	void update();
	void draw(float x, float y);
	void draw(float , float y, float w, float h);
	void draw(const ofPoint & point);
	void draw(const ofRectangle & rect);

	ofTexture & getTextureReference();
	ofTexture & getDepthTextureReference();
	void setUseTexture(bool bUseTexture);

	float getHeight();
	float getWidth();

	void setAnchorPercent(float xPct, float yPct){};
    void setAnchorPoint(float x, float y){};
	void resetAnchor(){};

	bool isFrameNew();
	void close();

	unsigned char * getPixels();
	unsigned char * getDepthPixels();
	float * getDistancePixels();
	unsigned char * getCalibratedRGBPixels();

	float getDistanceAt(int x, int y) ;
	ofxVec3f getWorldCoordinateFor(int x, int y);

	int fps;
private:
	FILE * f;
	unsigned short *buf;
	ofTexture depthTex,videoTex;
	bool bUseTexture;
	unsigned char *rgb;

	bool readColor;

	int lastFrameTime;

	ofxKinectCalibration calibration;
};

