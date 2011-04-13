/*
 * KinectPlayer.cpp
 *
 *  Created on: 14/12/2010
 *      Author: arturo
 */

#include "ofxKinectPlayer.h"

#ifdef _MSC_VER
	#define uint16_t unsigned short
#endif

//--------------------------------------------------------------------
ofxKinectPlayer::ofxKinectPlayer(){
	f = 0;
	filename = "";
	buf = 0;
	rgb = 0;
	bUseTexture = true;
	fps = 30;
}

//-----------------------------------------------------------
ofxKinectPlayer::~ofxKinectPlayer() {
	close();

	if(buf) delete[] buf;
	if(rgb) delete[] rgb;

	calibration.clear();
	depthTex.clear();
	videoTex.clear();
}

//-----------------------------------------------------------
void ofxKinectPlayer::setUseTexture(bool _bUseTexture){
	bUseTexture = _bUseTexture;
}

//-----------------------------------------------------------
void ofxKinectPlayer::setup(const string & file, bool video){
	f = fopen(ofToDataPath(file).c_str(), "rb");
	filename = file;
	if(!buf) buf 		= new uint16_t[640*480];
	if(!rgb) rgb = new unsigned char[640*480*3];
	memset(rgb,255,640*480*3);
	if(!depthTex.bAllocated() && bUseTexture)
		depthTex.allocate(640,480,GL_LUMINANCE);
	if(!videoTex.bAllocated() && bUseTexture)
		videoTex.allocate(640,480,GL_RGB);
	bVideo = video;
	calibration.init(3);
	lastFrameTime = ofGetElapsedTimeMillis();
}

//-----------------------------------------------------------
void ofxKinectPlayer::update(){
	if(!f) return;
	if((ofGetElapsedTimeMillis()-lastFrameTime)<(1000./float(fps)))	return;
	lastFrameTime = ofGetElapsedTimeMillis();
	if(bVideo)
		fread(rgb,640*480*3,1,f);
	fread(buf,640*480*sizeof(uint16_t),1,f);

	// loop?
	if(bLoop && std::feof(f) > 0) {
		f = fopen(ofToDataPath(filename).c_str(), "rb");
	}

	calibration.update(buf);
	if(bUseTexture){
		depthTex.loadData(calibration.getDepthPixels(),640,480,GL_LUMINANCE);
		videoTex.loadData(rgb,640,480,GL_RGB);
	}

}

//-----------------------------------------------------------
void ofxKinectPlayer::draw(float x, float y){
	if(bUseTexture) {
		videoTex.draw(x,y);
	}
}

//-----------------------------------------------------------
void ofxKinectPlayer::draw(float x, float y, float w, float h){
	if(bUseTexture) {
		videoTex.draw(x,y,w,h);
	}
}

//----------------------------------------------------------
void ofxKinectPlayer::draw(const ofPoint & point){
	draw(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinectPlayer::draw(const ofRectangle & rect){
	draw(rect.x, rect.y, rect.width, rect.height);
}

//---------------------------------------------------------------------------
void ofxKinectPlayer::drawDepth(float _x, float _y){
	if(bUseTexture) {
		depthTex.draw(_x, _y);
	}
}

//----------------------------------------------------------
void ofxKinectPlayer::drawDepth(float _x, float _y, float _w, float _h){
	if(bUseTexture) {
		depthTex.draw(_x, _y, _w, _h);
	}
}

//----------------------------------------------------------
void ofxKinectPlayer::drawDepth(const ofPoint & point){
	drawDepth(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinectPlayer::drawDepth(const ofRectangle & rect){
	drawDepth(rect.x, rect.y, rect.width, rect.height);
}

//-----------------------------------------------------------
unsigned char * ofxKinectPlayer::getPixels(){
	return rgb;
}

//-----------------------------------------------------------
unsigned char * ofxKinectPlayer::getDepthPixels(){
	return calibration.getDepthPixels();
}

//-----------------------------------------------------------
float * ofxKinectPlayer::getDistancePixels(){
	return calibration.getDistancePixels();
}

//-----------------------------------------------------------
unsigned char * ofxKinectPlayer::getCalibratedRGBPixels(){
	return calibration.getCalibratedRGBPixels(rgb);
}

//-----------------------------------------------------------
ofTexture & ofxKinectPlayer::getTextureReference(){
	return depthTex;
}

//-----------------------------------------------------------
ofTexture & ofxKinectPlayer::getDepthTextureReference(){
	return depthTex;
}

//-----------------------------------------------------------
float ofxKinectPlayer::getWidth(){
	return (float) width;
}

//-----------------------------------------------------------
float ofxKinectPlayer::getHeight(){
	return (float) height;
}

//-----------------------------------------------------------
bool ofxKinectPlayer::isFrameNew(){
	return true;
}

//-----------------------------------------------------------
void ofxKinectPlayer::close(){
	if(f)
		fclose(f);
	f = 0;
	filename = "";
}

//------------------------------------
float ofxKinectPlayer::getDistanceAt(int x, int y) {
	return calibration.getDistanceAt(x,y);
}

//------------------------------------
ofxVec3f ofxKinectPlayer::getWorldCoordinateFor(int x, int y) {
	return calibration.getWorldCoordinateFor(x,y);
}

