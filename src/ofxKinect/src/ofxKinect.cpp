#include "ofxKinect.h"
#include "ofMain.h"

// pointer to this class for static callback member functions
ofxKinect* thisKinect = NULL;



//--------------------------------------------------------------------
ofxKinect::ofxKinect()
{
	ofLog(OF_LOG_VERBOSE, "Creating ofxKinect.");

	//TODO: reset the right ones of these on close
	// common
	bVerbose 				= false;
	bGrabberInited 			= false;
	bUseTexture				= true;
	depthPixelsRaw			= NULL;
	depthPixelsBack			= NULL;
	videoPixels		  		= NULL;
	videoPixelsBack			= NULL;

	bNeedsUpdate			= false;
	bUpdateTex				= false;

	kinectContext			= NULL;
	kinectDevice			= NULL;
	
	targetTiltAngleDeg		= 0;
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

//--------------------------------------------------------------------
bool ofxKinect::isFrameNew(){
	if(isThreadRunning()){
		return !bNeedsUpdate;
	}
	return false;	
}

//--------------------------------------------------------------------
bool ofxKinect::open(){
	if(!bGrabberInited){
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}

	if (freenect_init(&kinectContext, NULL) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: freenet_init failed");
		return false;
	}

	int number_devices = freenect_num_devices(kinectContext);
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Number of Devices found: " + ofToString(number_devices));

	if (number_devices < 1) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Did not find a device");
		return false;
	}

	if (freenect_open_device(kinectContext, &kinectDevice, 0) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Could not open device");
		return false;
	}

	freenect_set_user(kinectDevice, this);

	startThread(true, false);	// blocking, not verbose

	return true;
}

//---------------------------------------------------------------------------
void ofxKinect::close(){
	if(isThreadRunning()){
		waitForThread(true);
	}

	//usleep(500000); // some time while thread is stopping ...

	//libusb_exit(NULL);
}

//We update the value here - but apply it in kinect thread.
//--------------------------------------------------------------------
bool ofxKinect::setCameraTiltAngle(float angleInDegrees){


	if(!kinectContext){
		return false;
	}

	targetTiltAngleDeg = ofClamp(angleInDegrees,-30,30);
	bTiltNeedsApplying = true;

	return true;
}

//--------------------------------------------------------------------
bool ofxKinect::init(bool infrared, bool setUseTexture){
	clear();

	bInfrared = infrared;
	bytespp = infrared?1:3;

	calibration.init(bytespp);

	bUseTexture = setUseTexture;

	int length = width*height;
	depthPixelsRaw = new unsigned short[length];
	depthPixelsBack = new unsigned short[length];

	videoPixels = new unsigned char[length*bytespp];
	videoPixelsBack = new unsigned char[length*bytespp];
	
	memset(depthPixelsRaw, 0, length*sizeof(unsigned short));
	memset(depthPixelsBack, 0, length*sizeof(unsigned short));

	memset(videoPixels, 0, length*bytespp*sizeof(unsigned char));
	memset(videoPixelsBack, 0, length*bytespp*sizeof(unsigned char));

	if(bUseTexture){
		depthTex.allocate(width, height, GL_LUMINANCE);
		videoTex.allocate(width, height, infrared?GL_LUMINANCE:GL_RGB);
	}

	bGrabberInited = true;

	ofLog(OF_LOG_VERBOSE, "ofxKinect: Inited");

	return bGrabberInited;
}

//---------------------------------------------------------------------------
void ofxKinect::clear(){
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
	if(!kinectContext){
		return;
	}

	if (!bNeedsUpdate){
		return;
	} else {
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
		videoTex.loadData(videoPixelsBack, width, height, bInfrared?GL_LUMINANCE:GL_RGB);
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
ofxPoint3f ofxKinect::getWorldCoordinateFor(int x, int y) {
	return calibration.getWorldCoordinateFor(x,y);
}


//------------------------------------
ofColor	ofxKinect::getColorAt(int x, int y) {
	int index = (y * width + x) * 3;
	ofColor c;
	c.r = videoPixels[index++];
	c.g = videoPixels[index++];
	c.b = videoPixels[index];
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
	return getCalibratedColorAt(calibration.getCalibratedColorCoordAt(p));
}

//------------------------------------
void ofxKinect::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y, float _w, float _h){
	if(bUseTexture) {
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
void ofxKinect::drawDepth(const ofPoint & point){
	drawDepth(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinect::draw(const ofRectangle & rect){
	draw(rect.x, rect.y, rect.width, rect.height);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(const ofRectangle & rect){
	drawDepth(rect.x, rect.y, rect.width, rect.height);
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
float ofxKinect::getHeight(){
	return (float)height;
}

//---------------------------------------------------------------------------
float ofxKinect::getWidth(){
	return (float)width;
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
	freenect_set_video_format(kinectDevice, bInfrared?FREENECT_VIDEO_IR_8BIT:FREENECT_VIDEO_RGB);
	freenect_set_depth_format(kinectDevice, FREENECT_DEPTH_11BIT);
	freenect_set_depth_callback(kinectDevice, &grabDepthFrame);
	freenect_set_video_callback(kinectDevice, &grabRgbFrame);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection opened");

	freenect_start_depth(kinectDevice);
	freenect_start_video(kinectDevice);
	
	while (isThreadRunning()) {
		if( bTiltNeedsApplying ){

			freenect_set_tilt_degs(kinectDevice, targetTiltAngleDeg);
			bTiltNeedsApplying = false;
		}

		freenect_update_tilt_state(kinectDevice);
		freenect_raw_tilt_state * tilt = freenect_get_tilt_state(kinectDevice);

		rawAccel.set(tilt->accelerometer_x, tilt->accelerometer_y, tilt->accelerometer_z);
		
		double dx,dy,dz;
		freenect_get_mks_accel(tilt, &dx, &dy, &dz);
		mksAccel.set(dx, dy, dz);
		
		ofSleepMillis(10);

//		printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", ax, ay, az, dx, dy, dz);
	}

//TODO: uncomment these when they are implemented in freenect	
	freenect_set_tilt_degs(kinectDevice, 0);
	freenect_update_tilt_state(kinectDevice);
	freenect_stop_depth(kinectDevice);
	freenect_stop_video(kinectDevice);
	freenect_set_led(kinectDevice, LED_YELLOW);

	freenect_close_device(kinectDevice);
	freenect_shutdown(kinectContext);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection closed");
}
