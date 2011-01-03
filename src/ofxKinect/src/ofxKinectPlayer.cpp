/*
 * KinectPlayer.cpp
 *
 *  Created on: 14/12/2010
 *      Author: arturo
 */

#include "ofxKinectPlayer.h"



//--------------------------------------------------------------------
ofxKinectPlayer::ofxKinectPlayer(){
	f = 0;
	buf = 0;
	rgb = 0;
	bUseTexture = true;
	fps = 30;
}

ofxKinectPlayer::~ofxKinectPlayer() {
	close();

	if(buf) delete[] buf;
	if(rgb) delete[] rgb;

	calibration.clear();
	depthTex.clear();
	videoTex.clear();
}

void ofxKinectPlayer::setUseTexture(bool _bUseTexture){
	bUseTexture = _bUseTexture;
}

void ofxKinectPlayer::setup(const string & file, bool color){
	f = fopen(ofToDataPath(file).c_str(), "rb");
	if(!buf) buf 		= new uint16_t[640*480];
	if(!rgb) rgb = new unsigned char[640*480*3];
	memset(rgb,255,640*480*3);
	if(!depthTex.bAllocated() && bUseTexture)
		depthTex.allocate(640,480,GL_LUMINANCE);
	if(!videoTex.bAllocated() && bUseTexture)
		videoTex.allocate(640,480,GL_RGB);
	readColor = color;
	calibration.init(3);
	lastFrameTime = ofGetElapsedTimeMillis();
}

void ofxKinectPlayer::update(){
	if(!f) return;
	if((ofGetElapsedTimeMillis()-lastFrameTime)<(1000./float(fps))) return;
	lastFrameTime = ofGetElapsedTimeMillis();
	if(readColor)
		fread(rgb,640*480*3,1,f);
	fread(buf,640*480*sizeof(uint16_t),1,f);

	calibration.update(buf);
	if(bUseTexture){
		depthTex.loadData(calibration.getDepthPixels(),640,480,GL_LUMINANCE);
		videoTex.loadData(rgb,640,480,GL_RGB);
	}

}

void ofxKinectPlayer::draw(float x, float y){
	depthTex.draw(x,y);
}

void ofxKinectPlayer::draw(float x, float y, float w, float h){
	depthTex.draw(x,y,w,h);
}

//----------------------------------------------------------
void ofxKinectPlayer::draw(const ofPoint & point){
	draw(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinectPlayer::draw(const ofRectangle & rect){
	draw(rect.x, rect.y, rect.width, rect.height);
}


unsigned char * ofxKinectPlayer::getPixels(){
	return rgb;
}

unsigned char * ofxKinectPlayer::getDepthPixels(){
	return calibration.getDepthPixels();
}

float * ofxKinectPlayer::getDistancePixels(){
	return calibration.getDistancePixels();
}

unsigned char * ofxKinectPlayer::getCalibratedRGBPixels(){
	return calibration.getCalibratedRGBPixels(rgb);
}


ofTexture & ofxKinectPlayer::getTextureReference(){
	return depthTex;
}

ofTexture & ofxKinectPlayer::getDepthTextureReference(){
	return depthTex;
}

float ofxKinectPlayer::getHeight(){
	return 480;
}

float ofxKinectPlayer::getWidth(){
	return 640;
}

bool ofxKinectPlayer::isFrameNew(){
	return true;
}

void ofxKinectPlayer::close(){
	if(f)
		fclose(f);
	f = 0;
}

//------------------------------------
float ofxKinectPlayer::getDistanceAt(int x, int y) {
	return calibration.getDistanceAt(x,y);
}


//------------------------------------
ofxPoint3f ofxKinectPlayer::getWorldCoordinateFor(int x, int y) {
	return calibration.getWorldCoordinateFor(x,y);
}

