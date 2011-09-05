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

	/// setup the recording to read,
	/// set video to true to enable the video (infrared/RGB) image
	/// make sure you recorded the video frames or reading them will fail big time!
	void setup(const string & file, bool video=false);
	void update();
	
	void draw(float x, float y);
	void draw(float , float y, float w, float h);
	void draw(const ofPoint & point);
	void draw(const ofRectangle & rect);

	void drawDepth(float x, float y, float w, float h);
	void drawDepth(float x, float y);
	void drawDepth(const ofPoint & point);
	void drawDepth(const ofRectangle & rect);

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
	
	void loop()				{bLoop = true;}
	void noLoop()			{bLoop = false;}
	void setLoop(bool loop)	{bLoop = loop;}
	bool isLooping()		{return bLoop;}
	

	unsigned char * getPixels();
	unsigned char * getDepthPixels();
	float * getDistancePixels();
	unsigned char * getCalibratedRGBPixels();
	
	/// for 007 compatibility
	ofPixelsRef getPixelsRef();

	float getDistanceAt(int x, int y) ;
	ofxVec3f getWorldCoordinateFor(int x, int y);

	int fps;
	
	const static int	width = 640;
	const static int	height = 480;
	
private:
	FILE * f;
	string filename;
	unsigned short *buf;
	ofTexture depthTex,videoTex;
	bool bUseTexture;
	unsigned char *rgb;
	
	ofPixels pixels;

	bool bVideo;
	bool bLoop;

	int lastFrameTime;

	ofxKinectCalibration calibration;
};

