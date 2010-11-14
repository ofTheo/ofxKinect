#include "ofxKinect.h"
#include "ofUtils.h"
#include "ofMath.h"

// pointer to this class for static callback member functions
ofxKinect* thisKinect = NULL;

//--------------------------------------------------------------------
ofxKinect::ofxKinect(){
	
	//TODO: reset the right ones of these on close
	// common
	bVerbose 				= false;
	bGrabberInited 			= false;
	bUseTexture				= true;
	depthPixels				= NULL;
	depthPixelsRaw			= NULL;
	depthPixelsBack			= NULL;
	rgbPixels		  		= NULL;
	rgbPixelsBack			= NULL;
	distancePixels 			= NULL;

	bNeedsUpdate			= false;
	bUpdateTex				= false;
	
	kinectDev = NULL;
	
	thisKinect = this;
}


//--------------------------------------------------------------------
ofxKinect::~ofxKinect(){
	close();
	clear();
}

//--------------------------------------------------------------------
void ofxKinect::setVerbose(bool bTalkToMe){
	bVerbose = bTalkToMe;
}

//---------------------------------------------------------------------------
unsigned char * ofxKinect::getPixels(){
	return rgbPixels;
}

unsigned char	* ofxKinect::getDepthPixels(){
	return depthPixels;
}

unsigned short 	* ofxKinect::getRawDepthPixels(){
	return depthPixelsRaw;
}

float* ofxKinect::getDistancePixels() {
	return distancePixels;
}

//------------------------------------
ofTexture & ofxKinect::getTextureReference(){
	if(!rgbTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: getTextureReference - texture is not allocated");
	}
	return depthTex;
}

ofTexture & ofxKinect::getDepthTextureReference(){
	if(!depthTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: getDepthTextureReference - texture is not allocated");
	}
	return rgbTex;
}

//--------------------------------------------------------------------
bool ofxKinect::open(){
	if(!bGrabberInited){
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}
	
	startThread(false, false);	// non blocking, not verbose
	
	return true;
}

void ofxKinect::close(){
	if(isThreadRunning()){
		stopThread();
	}

	usleep(500000); // some time while thread is stopping ...

	//libusb_exit(NULL);
}

//--------------------------------------------------------------------
bool ofxKinect::init(bool setUseTexture){
	clear();
    
	bUseTexture = setUseTexture;
    
	int length = width*height;
	depthPixels = new unsigned char[length];
	depthPixelsRaw = new unsigned short[length];
	depthPixelsBack = new unsigned short[length];
	distancePixels = new float[length];
	
	rgbPixels = new unsigned char[length*3];
	rgbPixelsBack = new unsigned char[length*3];
	
	memset(depthPixels, 0, length*sizeof(unsigned char));
	memset(depthPixelsRaw, 0, length*sizeof(unsigned short));
	memset(depthPixelsBack, 0, length*sizeof(unsigned short));
	memset(distancePixels, 0, length*sizeof(float));
	
	memset(rgbPixels, 0, length*3*sizeof(unsigned char));
	memset(rgbPixelsBack, 0, length*3*sizeof(unsigned char));
	
	if(bUseTexture){
		depthTex.allocate(width, height, GL_LUMINANCE);
		rgbTex.allocate(width, height, GL_RGB);
	}

	bGrabberInited = true;
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Inited");
	
	return bGrabberInited;
}

void ofxKinect::clear(){
	if(depthPixels != NULL){
		delete[] depthPixels; depthPixels = NULL;
		delete[] depthPixelsRaw; depthPixelsRaw = NULL;
		delete[] depthPixelsBack; depthPixelsBack = NULL;
		delete[] distancePixels; distancePixels = NULL;
	
		delete[] rgbPixels; rgbPixels = NULL;
		delete[] rgbPixelsBack; rgbPixelsBack = NULL;
	}

	depthTex.clear();
	rgbTex.clear();
    
	bGrabberInited = false;
}

//----------------------------------------------------------
void ofxKinect::update(){
	// you need to call init() before running!
	//assert(depthPixels);
	if(!kinectDev){
		return;
	}
	
	if(!bNeedsUpdate){
		return;
	}else{
		bUpdateTex = true;
	}

	if( this->lock() ){
		
		try{		
			for(int k = 0; k < width*height; k++){
				if(depthPixelsBack[k] == 2047) {
					distancePixels[k] = 0;
					depthPixels[k] = 0;
				} else {
					// using equation from https://github.com/OpenKinect/openkinect/wiki/Imaging-Information
					distancePixels[k] = 100.f / (-0.00307f * depthPixelsBack[k] + 3.33f);	
					depthPixels[k] = (float) (2048 * 256) / (2048 - depthPixelsBack[k]);
					
					// filter out some of the background noise
					if(depthPixelsBack[k] < 1024) {
						// invert and convert to 8 bit
						depthPixels[k] = (float) ((2048 * 256) / (depthPixelsBack[k] - 2048));
					}
					else {
						depthPixels[k] = 0;
					}
				}
			}
			memcpy(rgbPixels, rgbPixelsBack, width*height*3);
		}
		catch(...){
			ofLog(OF_LOG_ERROR, "ofxKinect: update memcpy failed");
		}
		
		//we have done the update
		bNeedsUpdate = false;

		this->unlock();
	}

	if(bUseTexture){
		depthTex.loadData(depthPixels, width, height, GL_LUMINANCE);
		rgbTex.loadData(rgbPixelsBack, width, height, GL_RGB);
		bUpdateTex = false;
	} 
}

//------------------------------------
void ofxKinect::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y, float _w, float _h){
	if(bUseTexture) {
		rgbTex.draw(_x, _y, _w, _h);
	}
}

void ofxKinect::draw(float _x, float _y){
	draw(_x, _y, (float)width, (float)height);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(float _x, float _y, float _w, float _h){
	if(bUseTexture) {
		depthTex.draw(_x, _y, _w, _h);
	}
}

void ofxKinect::drawDepth(float _x, float _y){
	drawDepth(_x, _y, (float)width, (float)height);
}

//----------------------------------------------------------
float ofxKinect::getHeight(){
	return (float)height;
}

float ofxKinect::getWidth(){
	return (float)width;
}

/* ***** PRIVATE ***** */

void ofxKinect::grabDepthFrame(uint16_t *buf, int width, int height){
	if(thisKinect->lock()){		
		// raw data
		try{
			memcpy(thisKinect->depthPixelsBack, buf, width*height*sizeof(uint16_t));
			thisKinect->bNeedsUpdate = true;
		}
		catch(...){
			ofLog(OF_LOG_ERROR, "ofxKinect: Depth memcpy failed");
		}
		thisKinect->unlock();
	}else{
		ofLog(OF_LOG_WARNING, "ofxKinect: grabDepthFrame unable to lock mutex");
	}	

}

void ofxKinect::grabRgbFrame(uint8_t *buf, int width, int height){
	if(thisKinect->lock()){
		try{
			memcpy(thisKinect->rgbPixelsBack, buf, width*height*3);
			thisKinect->bNeedsUpdate = true;
		}
		catch(...){
			ofLog(OF_LOG_ERROR, "ofxKinect: Rgb memcpy failed");
		}
		thisKinect->unlock();
	}else{
		ofLog(OF_LOG_WARNING, "ofxKinect: grabRgbFrame unable to lock mutex");
	}
}

void ofxKinect::threadedFunction(){   
	libusb_init(NULL);
	 		
	kinectDev = libusb_open_device_with_vid_pid(NULL, 0x45e, 0x2ae);
	if(!kinectDev){
		ofLog(OF_LOG_ERROR, "ofxKinect: Could not open device");
		return;
	}
	
	libusb_claim_interface(kinectDev, 0);
	
	cams_init(kinectDev, &grabDepthFrame, &grabRgbFrame);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection opened");
	
	while(isThreadRunning()){
		struct timeval tv = { 0, 0 };
		int ret = libusb_handle_events_timeout(NULL, &tv); // non blocking
		if(ret != 0){
			ofLog(OF_LOG_ERROR, "ofxKinect: libusb error: " + ofToString(ret));
			break;
		}
	}
    
	libusb_release_interface(kinectDev, 0);
	
	if(kinectDev){
		libusb_close(kinectDev);
	}
	
	libusb_exit(NULL);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection closed");
}
