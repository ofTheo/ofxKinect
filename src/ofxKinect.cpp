#include "ofxKinect.h"
#include "ofMain.h"

// pointer to this class for static callback member functions
ofxKinect* thisKinect = NULL;



//--------------------------------------------------------------------
ofxKinect::ofxKinect()
{
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Creating ofxKinect");

	bVerbose 				= false;
	bUseTexture				= true;
	bGrabVideo				= true;
	
	// set defaults
	bGrabberInited 			= false;
	depthPixelsRaw			= NULL;
	depthPixelsBack			= NULL;
	videoPixels		  		= NULL;
	videoPixelsBack			= NULL;

	bNeedsUpdate			= false;
	bUpdateTex				= false;
	bIsFrameNew = false;

	kinectContext			= NULL;
	kinectDevice			= NULL;
	
	targetTiltAngleDeg		= 0;
    currentTiltAngleDeg     = 0;
	bTiltNeedsApplying		= false;

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
	return videoPixels;
}

//---------------------------------------------------------------------------
unsigned char	* ofxKinect::getDepthPixels(){
	return calibration.getDepthPixels();
}

//---------------------------------------------------------------------------
unsigned short 	* ofxKinect::getRawDepthPixels(){
	return depthPixelsBack;
}

//---------------------------------------------------------------------------
float* ofxKinect::getDistancePixels() {
	return calibration.getDistancePixels();
}

//---------------------------------------------------------------------------
unsigned char * ofxKinect::getCalibratedRGBPixels(){
	return calibration.getCalibratedRGBPixels(videoPixels);
}

//------------------------------------
ofTexture & ofxKinect::getTextureReference(){
	if(!videoTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: getTextureReference - texture is not allocated");
	}
	return videoTex;
}

//---------------------------------------------------------------------------
ofTexture & ofxKinect::getDepthTextureReference(){
	if(!depthTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: getDepthTextureReference - texture is not allocated");
	}
	return depthTex;
}

//---------------------------------------------------------------------------
ofPixels & ofxKinect::getPixelsRef() {
	return pixels;
}

//--------------------------------------------------------------------
bool ofxKinect::isFrameNew(){
	if(isThreadRunning()){
		bool curIsFrameNew = bIsFrameNew;
		bIsFrameNew = false;
		return curIsFrameNew;
	}
	return false;	
}

//--------------------------------------------------------------------
bool ofxKinect::open(){
	if(!bGrabberInited){
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}

	int number_devices = freenect_num_devices(kinectContext);
	if (number_devices < 1) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Did not find a device");
		return false;
	}

	if (freenect_open_device(kinectContext, &kinectDevice, 0) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Could not open device");
		return false;
	}

	freenect_set_user(kinectDevice, this);
	freenect_set_depth_callback(kinectDevice, &grabDepthFrame);
	freenect_set_video_callback(kinectDevice, &grabRgbFrame);

	startThread(true, false);	// blocking, not verbose

	return true;
}

//---------------------------------------------------------------------------
void ofxKinect::close(){
	if(isThreadRunning()){
		waitForThread(true);
	}
	
	bIsFrameNew = false;
	bNeedsUpdate	= false;
	bUpdateTex		= false;
}

//---------------------------------------------------------------------------
bool ofxKinect::isConnected(){
	return isThreadRunning();
}

//We update the value here - but apply it in kinect thread.
//--------------------------------------------------------------------
bool ofxKinect::setCameraTiltAngle(float angleInDegrees){

	if(!bGrabberInited){
		return false;
	}

	targetTiltAngleDeg = ofClamp(angleInDegrees,-30,30);
	bTiltNeedsApplying = true;

	return true;
}

//--------------------------------------------------------------------
float ofxKinect::getTargetCameraTiltAngle(){
    return targetTiltAngleDeg;
}

float ofxKinect::getCurrentCameraTiltAngle(){
    return currentTiltAngleDeg;
}


//--------------------------------------------------------------------
bool ofxKinect::init(bool infrared, bool video, bool texture){
	if(isConnected()){
		ofLog(OF_LOG_WARNING, "ofxKinect: Do not call init while ofxKinect is running!");
		return false;
	}
	
	clear();

	bInfrared = infrared;
	bGrabVideo = video;
	bytespp = infrared?1:3;

	calibration.init(bytespp);

	bUseTexture = texture;

	int length = width*height;
	depthPixelsRaw = new unsigned short[length];
	depthPixelsBack = new unsigned short[length];

	videoPixels = new unsigned char[length*bytespp];
	pixels.setFromExternalPixels(videoPixels, width, height, OF_IMAGE_COLOR);
	videoPixelsBack = new unsigned char[length*bytespp];
	
	memset(depthPixelsRaw, 0, length*sizeof(unsigned short));
	memset(depthPixelsBack, 0, length*sizeof(unsigned short));

	memset(videoPixels, 0, length*bytespp*sizeof(unsigned char));
	memset(videoPixelsBack, 0, length*bytespp*sizeof(unsigned char));

	if(bUseTexture){
		depthTex.allocate(width, height, GL_LUMINANCE);
		videoTex.allocate(width, height, infrared?GL_LUMINANCE:GL_RGB);
	}
	
	if (freenect_init(&kinectContext, NULL) < 0){
		ofLog(OF_LOG_ERROR, "ofxKinect: freenet_init failed");
		return false;
	}
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Inited");

	int number_devices = freenect_num_devices(kinectContext);
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Number of Devices found: " + ofToString(number_devices));

	bGrabberInited = true;

	return bGrabberInited;
}

//---------------------------------------------------------------------------
void ofxKinect::clear(){
	if(isConnected()){
		ofLog(OF_LOG_WARNING, "ofxKinect: Do not call clear while ofxKinect is running!");
		return;
	}
	
	if(kinectContext != NULL){
		freenect_shutdown(kinectContext);
	}
	
	if(depthPixelsRaw != NULL){
		delete[] depthPixelsRaw; depthPixelsRaw = NULL;
		delete[] depthPixelsBack; depthPixelsBack = NULL;

		delete[] videoPixels; videoPixels = NULL;
		delete[] videoPixelsBack; videoPixelsBack = NULL;
	}

	depthTex.clear();
	videoTex.clear();
	calibration.clear();
	
	bGrabberInited = false;
}

//----------------------------------------------------------
void ofxKinect::update(){
	if(!bGrabberInited){
		return;
	}

	if (!bNeedsUpdate){
		return;
	} else {
		bIsFrameNew = true;
		bUpdateTex = true;
	}

	if ( this->lock() ) {
		int n = width * height;
		
		calibration.update(depthPixelsBack);
		memcpy(depthPixelsRaw,depthPixelsBack,n*sizeof(short));
		memcpy(videoPixels, videoPixelsBack, n * bytespp);

		//we have done the update
		bNeedsUpdate = false;

		this->unlock();
	}

	if(bUseTexture){
		depthTex.loadData(calibration.getDepthPixels(), width, height, GL_LUMINANCE);
		videoTex.loadData(videoPixels, width, height, bInfrared?GL_LUMINANCE:GL_RGB);
		bUpdateTex = false;
	}
}


//------------------------------------
float ofxKinect::getDistanceAt(int x, int y) {
	return calibration.getDistanceAt(x,y);
}

//------------------------------------
float ofxKinect::getDistanceAt(const ofPoint & p) {
	return calibration.getDistanceAt(p);
}

//------------------------------------
ofxVec3f ofxKinect::getWorldCoordinateFor(int x, int y) {
	return calibration.getWorldCoordinateFor(x,y);
}


//------------------------------------
ofColor	ofxKinect::getColorAt(int x, int y) {
	int index = (y * width + x) * bytespp;
	ofColor c;
	c.r = videoPixels[index + 0];
	c.g = videoPixels[index + (bytespp-1)/2];
	c.b = videoPixels[index + (bytespp-1)];
	c.a = 255;

	return c;
}

//------------------------------------
ofColor ofxKinect::getColorAt(const ofPoint & p) {
	return getColorAt(p.x, p.y);
}

//------------------------------------
ofColor ofxKinect::getCalibratedColorAt(int x, int y){
	return getColorAt(calibration.getCalibratedColorCoordAt(x,y));
}

//------------------------------------
ofColor ofxKinect::getCalibratedColorAt(const ofPoint & p){
	return getColorAt(calibration.getCalibratedColorCoordAt(p));
}

//------------------------------------
void ofxKinect::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y, float _w, float _h){
	if(bUseTexture && bGrabVideo) {
		videoTex.draw(_x, _y, _w, _h);
	}
}

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y){
	draw(_x, _y, (float)width, (float)height);
}

//----------------------------------------------------------
void ofxKinect::draw(const ofPoint & point){
	draw(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinect::draw(const ofRectangle & rect){
	draw(rect.x, rect.y, rect.width, rect.height);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(float _x, float _y, float _w, float _h){
	if(bUseTexture) {
		depthTex.draw(_x, _y, _w, _h);
	}
}

//---------------------------------------------------------------------------
void ofxKinect::drawDepth(float _x, float _y){
	drawDepth(_x, _y, (float)width, (float)height);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(const ofPoint & point){
	drawDepth(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(const ofRectangle & rect){
	drawDepth(rect.x, rect.y, rect.width, rect.height);
}

//----------------------------------------------------------
float ofxKinect::getHeight(){
	return (float) height;
}

//---------------------------------------------------------------------------
float ofxKinect::getWidth(){
	return (float) width;
}

//---------------------------------------------------------------------------
ofPoint ofxKinect::getRawAccel(){
	return rawAccel;
}

//---------------------------------------------------------------------------
ofPoint ofxKinect::getMksAccel(){
	return mksAccel;
}

//---------------------------------------------------------------------------
void ofxKinect::enableDepthNearValueWhite(bool bEnabled){
	calibration.enableDepthNearValueWhite(bEnabled);
}

//---------------------------------------------------------------------------
bool ofxKinect::isDepthNearValueWhite(){
	return calibration.isDepthNearValueWhite();
}

/* ***** PRIVATE ***** */

//---------------------------------------------------------------------------
void ofxKinect::grabDepthFrame(freenect_device *dev, void *depth, uint32_t timestamp) {
	if (thisKinect->lock()) {
		try {
			memcpy(thisKinect->depthPixelsBack, depth, FREENECT_DEPTH_11BIT_SIZE);
			thisKinect->bNeedsUpdate = true;
		}
		catch(...) {
			ofLog(OF_LOG_ERROR, "ofxKinect: Depth memcpy failed");
		}
		thisKinect->unlock();
	} else {
		ofLog(OF_LOG_WARNING, "ofxKinect: grabDepthFrame unable to lock mutex");
	}
}

//---------------------------------------------------------------------------
void ofxKinect::grabRgbFrame(freenect_device *dev, void *rgb, uint32_t timestamp) {
	if (thisKinect->lock()) {
		try {
			memcpy(thisKinect->videoPixelsBack, rgb, thisKinect->bInfrared?FREENECT_VIDEO_IR_8BIT_SIZE:FREENECT_VIDEO_RGB_SIZE);
			thisKinect->bNeedsUpdate = true;
		}
		catch (...) {
			ofLog(OF_LOG_ERROR, "ofxKinect: Rgb memcpy failed");
		}
		thisKinect->unlock();
	} else {
		ofLog(OF_LOG_ERROR, "ofxKinect: grabRgbFrame unable to lock mutex");
	}
}

//---------------------------------------------------------------------------
void ofxKinect::threadedFunction(){	

	
	freenect_set_led(kinectDevice, LED_GREEN);
	freenect_set_video_format(kinectDevice, bInfrared?FREENECT_VIDEO_IR_8BIT:FREENECT_VIDEO_YUV_RGB);
	freenect_set_depth_format(kinectDevice, FREENECT_DEPTH_11BIT);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection opened");

	freenect_start_depth(kinectDevice);
	if(bGrabVideo) {
		freenect_start_video(kinectDevice);
	}

	// call platform specific processors (needed for Win)
	if(freenect_process_events(kinectContext) != 0){
		ofLog(OF_LOG_ERROR, "ofxKinect: freenect_process_events failed!");
		return;
	}
	
	while(isThreadRunning()){
		if(bTiltNeedsApplying){
			freenect_set_tilt_degs(kinectDevice, targetTiltAngleDeg);
			bTiltNeedsApplying = false;
		}

		freenect_update_tilt_state(kinectDevice);
		freenect_raw_tilt_state * tilt = freenect_get_tilt_state(kinectDevice);
        
        currentTiltAngleDeg = freenect_get_tilt_degs(tilt);

		rawAccel.set(tilt->accelerometer_x, tilt->accelerometer_y, tilt->accelerometer_z);
		
		double dx,dy,dz;
		freenect_get_mks_accel(tilt, &dx, &dy, &dz);
		mksAccel.set(dx, dy, dz);
		
		ofSleepMillis(10);

//		printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", ax, ay, az, dx, dy, dz);
	}
	
	// finish up a tilt on exit
	if(bTiltNeedsApplying){
		freenect_set_tilt_degs(kinectDevice, targetTiltAngleDeg);
		bTiltNeedsApplying = false;
	}

	freenect_stop_depth(kinectDevice);
	freenect_stop_video(kinectDevice);
	freenect_set_led(kinectDevice, LED_YELLOW);

	freenect_close_device(kinectDevice);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection closed");
}

//---------------------------------------------------------------------------
ofxKinectCalibration& ofxKinect::getCalibration() {
	return calibration;
}
