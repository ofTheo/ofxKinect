/*==============================================================================

    Copyright (c) 2010, 2011 ofxKinect Team

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
    
    ----------------------------------------------------------------------------
    
    This project uses libfreenect, copyrighted by the Open Kinect Project using
    the Apache License v2. See the file "APACHE20" in libs/libfreenect.
    
    See http://www.openkinect.org & https://github.com/OpenKinect/libfreenect 
    for documentation
    
==============================================================================*/
#include "ofxKinect.h"
#include "ofMain.h"

#include "libfreenect-registration.h"

#define OFX_KINECT_GRAVITY 9.80665

// context static
ofxKinectContext ofxKinect::kinectContext;

//--------------------------------------------------------------------
ofxKinect::ofxKinect() {
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Creating ofxKinect");

	deviceId = -1;
	serial = "";
	
	bUseTexture = true;
	bGrabVideo = true;

	// set defaults
	bGrabberInited = false;

	bNeedsUpdate = false;
	bUpdateTex = false;
	bIsFrameNew = false;
    
	bIsVideoInfrared = false;
	videoBytesPerPixel = 3;

	kinectDevice = NULL;

	targetTiltAngleDeg = 0;
	currentTiltAngleDeg = 0;
	bTiltNeedsApplying = false;
    
    currentLed = -1;
    bLedNeedsApplying = false;
	
	lastDeviceId = -1;
	tryCount = 0;
	timeSinceOpen = 0;
	bGotData = false;

	bUseRegistration = false;
	bNearWhite = true;

	setDepthClipping();
}

//--------------------------------------------------------------------
ofxKinect::~ofxKinect() {
	close();
	clear();
}

//--------------------------------------------------------------------
bool ofxKinect::init(bool infrared, bool video, bool texture) {
	if(isConnected()) {
		ofLog(OF_LOG_WARNING, "ofxKinect: Do not call init while ofxKinect is running!");
		return false;
	}

	clear();

	bIsVideoInfrared = infrared;
	bGrabVideo = video;
	videoBytesPerPixel = infrared?1:3;

	bUseTexture = texture;

	// allocate
	depthPixelsRaw.allocate(width, height, 1);
	depthPixelsRawBack.allocate(width, height, 1);

	videoPixels.allocate(width, height, videoBytesPerPixel);
	videoPixelsBack.allocate(width, height, videoBytesPerPixel);

	depthPixels.allocate(width, height, 1);
	distancePixels.allocate(width, height, 1);

	 // set
	depthPixelsRaw.set(0);
	depthPixelsRawBack.set(0);

	videoPixels.set(0);
	videoPixelsBack.set(0);

	depthPixels.set(0);    
	distancePixels.set(0);

	if(bUseTexture) {
		depthTex.allocate(width, height, GL_LUMINANCE);
		videoTex.allocate(width, height, infrared ? GL_LUMINANCE : GL_RGB);
	}

	if(!kinectContext.isInited()) {
		if(!kinectContext.init()) {
			return false;
		}
	}

	bGrabberInited = true;

	return bGrabberInited;
}

//---------------------------------------------------------------------------
void ofxKinect::clear() {
	if(isConnected()) {
		ofLog(OF_LOG_WARNING, "ofxKinect: Do not call clear while ofxKinect is running!");
		return;
	}

	depthPixelsRaw.clear();
	depthPixelsRawBack.clear();

	videoPixels.clear();
	videoPixelsBack.clear();

	depthPixels.clear();
	distancePixels.clear();

	depthTex.clear();
	videoTex.clear();

	bGrabberInited = false;
}

//--------------------------------------------------------------------
void ofxKinect::setRegistration(bool bUseRegistration) {
	this->bUseRegistration = bUseRegistration;
}

//--------------------------------------------------------------------
bool ofxKinect::open(int id) {
	if(!bGrabberInited) {
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}

	if(!kinectContext.open(*this, id)) {
		return false;
	}

	lastDeviceId = deviceId;
	timeSinceOpen = ofGetElapsedTimef();
	bGotData = false;

	freenect_set_user(kinectDevice, this);
	freenect_set_depth_callback(kinectDevice, &grabDepthFrame);
	freenect_set_video_callback(kinectDevice, &grabVideoFrame);

	startThread(true, false); // blocking, not verbose

	return true;
}

//--------------------------------------------------------------------
bool ofxKinect::open(string serial) {
	if(!bGrabberInited) {
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}
	
	if(!kinectContext.open(*this, serial)) {
		return false;
	}
	
	lastDeviceId = deviceId;
	timeSinceOpen = ofGetElapsedTimef();
	bGotData = false;
	
	freenect_set_user(kinectDevice, this);
	freenect_set_depth_callback(kinectDevice, &grabDepthFrame);
	freenect_set_video_callback(kinectDevice, &grabVideoFrame);
	
	startThread(true, false); // blocking, not verbose
	
	return true;
}

//---------------------------------------------------------------------------
void ofxKinect::close() {
	if(isThreadRunning()) {
		waitForThread(true);
	}

	deviceId = -1;
	serial = "";
	bIsFrameNew = false;
	bNeedsUpdate = false;
	bUpdateTex = false;
}

//---------------------------------------------------------------------------
bool ofxKinect::isConnected() {
	return isThreadRunning();
}

//--------------------------------------------------------------------
bool ofxKinect::isFrameNew() {
	if(isThreadRunning()) {
		bool curIsFrameNew = bIsFrameNew;
		bIsFrameNew = false;
		return curIsFrameNew;
	}
	return false;
}

//----------------------------------------------------------
void ofxKinect::update() {
	if(!bGrabberInited) {
		return;
	}

	if(!bNeedsUpdate && !bGotData && tryCount < 5 && ofGetElapsedTimef() - timeSinceOpen > 2.0 ){
		close();
		ofLog(OF_LOG_WARNING, "ofxKinect: Device %d isn't delivering data, reconnecting tries: %d", lastDeviceId, tryCount+1);
		kinectContext.buildDeviceList();
		open(lastDeviceId);
		tryCount++;
		timeSinceOpen = ofGetElapsedTimef();
		return;
	}

	if(!bNeedsUpdate){
		return;
	} else {
		bIsFrameNew = true;
		bUpdateTex = true;
		bGotData = true;
		tryCount = 0;
	}

	if(this->lock()) {
		int n = width * height;

		depthPixelsRaw = depthPixelsRawBack;
		videoPixels = videoPixelsBack;

		// we have done the update
		bNeedsUpdate = false;

		this->unlock();

		updateDepthPixels();
	}

	if(bUseTexture) {
		depthTex.loadData(depthPixels.getPixels(), width, height, GL_LUMINANCE);
		videoTex.loadData(videoPixels.getPixels(), width, height, bIsVideoInfrared?GL_LUMINANCE:GL_RGB);
		bUpdateTex = false;
	}
}

//------------------------------------
float ofxKinect::getDistanceAt(int x, int y) {
	return depthPixelsRaw[y * width + x];
}

//------------------------------------
float ofxKinect::getDistanceAt(const ofPoint & p) {
	return getDistanceAt(p.x, p.y);
}

//------------------------------------
ofVec3f ofxKinect::getWorldCoordinateAt(int x, int y) {
	return getWorldCoordinateAt(x, y, getDistanceAt(x, y));
}

//------------------------------------
ofVec3f ofxKinect::getWorldCoordinateAt(float cx, float cy, float wz) {
	double wx, wy;
	freenect_camera_to_world(kinectDevice, cx, cy, wz, &wx, &wy);
	return ofVec3f(wx, wy, wz);
}

//------------------------------------
ofColor ofxKinect::getColorAt(int x, int y) {
	int index = (y * width + x) * videoBytesPerPixel;
	ofColor c;
	c.r = videoPixels[index + 0];
	c.g = videoPixels[index + (videoBytesPerPixel-1)/2];
	c.b = videoPixels[index + (videoBytesPerPixel-1)];
	c.a = 255;

	return c;
}

//------------------------------------
ofColor ofxKinect::getColorAt(const ofPoint & p) {
	return getColorAt(p.x, p.y);
}

//---------------------------------------------------------------------------
ofPoint ofxKinect::getRawAccel() {
	return rawAccel;
}

//---------------------------------------------------------------------------
ofPoint ofxKinect::getMksAccel() {
	return mksAccel;
}

//---------------------------------------------------------------------------
float ofxKinect::getAccelPitch(){
	return ofRadToDeg(asin(getMksAccel().z/OFX_KINECT_GRAVITY));
}

//---------------------------------------------------------------------------
float ofxKinect::getAccelRoll(){
	return ofRadToDeg(asin(getMksAccel().x/OFX_KINECT_GRAVITY));
}

//---------------------------------------------------------------------------
unsigned char * ofxKinect::getPixels() {
	return videoPixels.getPixels();
}

//---------------------------------------------------------------------------
unsigned char * ofxKinect::getDepthPixels() {
	return depthPixels.getPixels();
}

//---------------------------------------------------------------------------
unsigned short * ofxKinect::getRawDepthPixels() {
	return depthPixelsRaw.getPixels();
}

//---------------------------------------------------------------------------
float* ofxKinect::getDistancePixels() {
	return distancePixels.getPixels();
}

ofPixels & ofxKinect::getPixelsRef(){
	return videoPixels;
}

ofPixels & ofxKinect::getDepthPixelsRef(){
	return depthPixels;
}

ofShortPixels & ofxKinect::getRawDepthPixelsRef(){
	return depthPixelsRaw;
}

ofFloatPixels & ofxKinect::getDistancePixelsRef(){
	return distancePixels;
}

//------------------------------------
ofTexture& ofxKinect::getTextureReference(){
	if(!videoTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: Device %d video texture is not allocated", deviceId);
	}
	return videoTex;
}

//---------------------------------------------------------------------------
ofTexture& ofxKinect::getDepthTextureReference(){
	if(!depthTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: Device %d depth texture is not allocated", deviceId);
	}
	return depthTex;
}

//---------------------------------------------------------------------------
void ofxKinect::enableDepthNearValueWhite(bool bEnabled) {
	bNearWhite = bEnabled;
	updateDepthLookupTable();
}

//---------------------------------------------------------------------------
bool ofxKinect::isDepthNearValueWhite() {
	return bNearWhite;
}

//---------------------------------------------------------------------------
void ofxKinect::setDepthClipping(float nearClip, float farClip) {
	nearClipping = nearClip;
	farClipping = farClip;
	updateDepthLookupTable();
}

//---------------------------------------------------------------------------
float ofxKinect::getNearClipping() {
    return nearClipping;
}

//---------------------------------------------------------------------------
float ofxKinect::getFarClipping() {
    return farClipping;
}

// we update the value here, but apply it in kinect thread
//--------------------------------------------------------------------
bool ofxKinect::setCameraTiltAngle(float angleInDegrees) {

	if(!bGrabberInited) {
		return false;
	}

	targetTiltAngleDeg = ofClamp(angleInDegrees, -30, 30);
	bTiltNeedsApplying = true;

	return true;
}

//--------------------------------------------------------------------
float ofxKinect::getTargetCameraTiltAngle() {
	return targetTiltAngleDeg;
}

float ofxKinect::getCurrentCameraTiltAngle() {
	return currentTiltAngleDeg;
}

//--------------------------------------------------------------------

void ofxKinect::setLed(ofxKinect::LedMode mode) {
	if(mode == currentLed) {
		return;
	}
    bLedNeedsApplying = true;
    currentLed = mode;
}

//------------------------------------
void ofxKinect::setUseTexture(bool bUse){
	bUseTexture = bUse;
}

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y, float _w, float _h) {
	if(bUseTexture && bGrabVideo) {
		videoTex.draw(_x, _y, _w, _h);
	}
}

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y) {
	draw(_x, _y, (float)width, (float)height);
}

//----------------------------------------------------------
void ofxKinect::draw(const ofPoint & point) {
	draw(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinect::draw(const ofRectangle & rect) {
	draw(rect.x, rect.y, rect.width, rect.height);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(float _x, float _y, float _w, float _h) {
	if(bUseTexture) {
		depthTex.draw(_x, _y, _w, _h);
	}
}

//---------------------------------------------------------------------------
void ofxKinect::drawDepth(float _x, float _y) {
	drawDepth(_x, _y, (float)width, (float)height);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(const ofPoint & point) {
	drawDepth(point.x, point.y);
}

//----------------------------------------------------------
void ofxKinect::drawDepth(const ofRectangle & rect) {
	drawDepth(rect.x, rect.y, rect.width, rect.height);
}

//---------------------------------------------------------------------------
int ofxKinect::getDeviceId() {
	return deviceId;
}

//---------------------------------------------------------------------------
string ofxKinect::getSerial() {
	return serial;
}

//----------------------------------------------------------
float ofxKinect::getHeight() {
	return (float) height;
}

//---------------------------------------------------------------------------
float ofxKinect::getWidth() {
	return (float) width;
}

//----------------------------------------------------------
void ofxKinect::listDevices() {
	kinectContext.listDevices();
}

//---------------------------------------------------------------------------
int ofxKinect::numTotalDevices() {
	return kinectContext.numTotal();
}

//---------------------------------------------------------------------------
int ofxKinect::numAvailableDevices() {
	return kinectContext.numAvailable();
}

//---------------------------------------------------------------------------
int ofxKinect::numConnectedDevices() {
	return kinectContext.numConnected();
}

//---------------------------------------------------------------------------
bool ofxKinect::isDeviceConnected(int id) {
	return kinectContext.isConnected(id);
}

//---------------------------------------------------------------------------
bool ofxKinect::isDeviceConnected(string serial) {
	return kinectContext.isConnected(serial);
}

//---------------------------------------------------------------------------
int ofxKinect::nextAvailableId() {
	return kinectContext.nextAvailableId();
}

//---------------------------------------------------------------------------
string ofxKinect::nextAvailableSerial() {
	return kinectContext.nextAvailableSerial();
}

/* ***** PRIVATE ***** */

//---------------------------------------------------------------------------
void ofxKinect::updateDepthLookupTable() {
	unsigned char nearColor = bNearWhite ? 255 : 0;
	unsigned char farColor = bNearWhite ? 0 : 255;
	unsigned int maxDepthLevels = 10000;
	depthLookupTable.resize(maxDepthLevels);
	depthLookupTable[0] = 0;
	for(int i = 1; i < maxDepthLevels; i++) {
		depthLookupTable[i] = ofMap(i, nearClipping, farClipping, nearColor, farColor, true);
	}
}

//----------------------------------------------------------
void ofxKinect::updateDepthPixels() {
	int n = width * height;
	for(int i = 0; i < n; i++) {
		distancePixels[i] = depthPixelsRaw[i];
	}
	for(int i = 0; i < n; i++) {
		depthPixels[i] = depthLookupTable[depthPixelsRaw[i]];
	}
}

//---------------------------------------------------------------------------
void ofxKinect::grabDepthFrame(freenect_device *dev, void *depth, uint32_t timestamp) {

	ofxKinect* kinect = kinectContext.getKinect(dev);

	if(kinect->kinectDevice == dev) {
		kinect->lock();
		freenect_frame_mode curMode = freenect_get_current_depth_mode(dev);
		kinect->depthPixelsRawBack.setFromPixels((unsigned short*) depth, width, height, 1);
		kinect->bNeedsUpdate = true;
		kinect->unlock();
    }
}

//---------------------------------------------------------------------------
void ofxKinect::grabVideoFrame(freenect_device *dev, void *video, uint32_t timestamp) {

	ofxKinect* kinect = kinectContext.getKinect(dev);

	if(kinect->kinectDevice == dev) {
		kinect->lock();
		freenect_frame_mode curMode = freenect_get_current_video_mode(dev);
		kinect->videoPixelsBack.setFromPixels((unsigned char*)video, width, height, curMode.data_bits_per_pixel/8);
		kinect->bNeedsUpdate = true;
		kinect->unlock();
	}
}

//---------------------------------------------------------------------------
void ofxKinect::threadedFunction(){

	if (currentLed < 0) { 
        freenect_set_led(kinectDevice, (freenect_led_options)ofxKinect::LED_GREEN); 
    }
	
	freenect_frame_mode videoMode = freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, bIsVideoInfrared?FREENECT_VIDEO_IR_8BIT:FREENECT_VIDEO_RGB);
	freenect_set_video_mode(kinectDevice, videoMode);
	freenect_frame_mode depthMode = freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, bUseRegistration?FREENECT_DEPTH_REGISTERED:FREENECT_DEPTH_MM);
	freenect_set_depth_mode(kinectDevice, depthMode);

	ofLog(OF_LOG_VERBOSE, "ofxKinect: Device %d %s connection opened", deviceId, serial.c_str());

	freenect_start_depth(kinectDevice);
	if(bGrabVideo) {
		freenect_start_video(kinectDevice);
	}

	// call platform specific processors (needed for Win)
	if(freenect_process_events(kinectContext.getContext()) != 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Device %d freenect_process_events failed!", deviceId);
		return;
	}

	while(isThreadRunning()) {
		
		if(bTiltNeedsApplying) {
			freenect_set_tilt_degs(kinectDevice, targetTiltAngleDeg);
			bTiltNeedsApplying = false;
		}
		
		if(bLedNeedsApplying) {
			if(currentLed == ofxKinect::LED_DEFAULT) {
				freenect_set_led(kinectDevice, (freenect_led_options)ofxKinect::LED_GREEN);
			}
			else {
				freenect_set_led(kinectDevice, (freenect_led_options)currentLed);
			}
			bLedNeedsApplying = false;
		}

		freenect_update_tilt_state(kinectDevice);
		freenect_raw_tilt_state * tilt = freenect_get_tilt_state(kinectDevice);
		currentTiltAngleDeg = freenect_get_tilt_degs(tilt);

		rawAccel.set(tilt->accelerometer_x, tilt->accelerometer_y, tilt->accelerometer_z);

		double dx,dy,dz;
		freenect_get_mks_accel(tilt, &dx, &dy, &dz);
		mksAccel.set(dx, dy, dz);

		// ... and $0.02 for the scheduler
		ofSleepMillis(10);
	}

	// finish up a tilt on exit
	if(bTiltNeedsApplying) {
		freenect_set_tilt_degs(kinectDevice, targetTiltAngleDeg);
		bTiltNeedsApplying = false;
	}

	freenect_stop_depth(kinectDevice);
	freenect_stop_video(kinectDevice);
	if (currentLed < 0) { 
        freenect_set_led(kinectDevice, (freenect_led_options)ofxKinect::LED_YELLOW); 
    }

	kinectContext.close(*this);
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Device %d connection closed", deviceId);
}

//---------------------------------------------------------------------------
// ofxKinectContext
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
ofxKinectContext::ofxKinectContext() {
	bInited = false;
	kinectContext = NULL;
}
ofxKinectContext::~ofxKinectContext() {
	closeAll();
	clear();
}

//---------------------------------------------------------------------------
static bool sortKinectPairs(ofxKinectContext::KinectPair A, ofxKinectContext::KinectPair B){
	return A.serial < B.serial;
}
        
//---------------------------------------------------------------------------
bool ofxKinectContext::init() {
	
	if(freenect_init(&kinectContext, NULL) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: freenect_init failed");
		bInited = false;
		return false;
	}

	bInited = true;
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Context inited");
	
	buildDeviceList();
	listDevices(true);

	return true;
}

//---------------------------------------------------------------------------
void ofxKinectContext::clear() {
	if(isInited() && numConnected() < 1) {
		freenect_shutdown(kinectContext);
		kinectContext = NULL;
		bInited = false;
		ofLog(OF_LOG_VERBOSE, "ofxKinect: Context cleared");
	}
}

bool ofxKinectContext::isInited() {
	return bInited;
}

bool ofxKinectContext::open(ofxKinect& kinect, int id) {
	
	// rebuild if necessary (aka new kinects plugged in)
	buildDeviceList();
	
	if(numConnected() >= numTotal()) {
		ofLog(OF_LOG_WARNING, "ofxKinect: No available devices found");
		return false;
	}
	
	// is the id available?
	if(id < 0) {
		id = nextAvailableId();
	}
	if(isConnected(id)) {
		ofLog(OF_LOG_WARNING, "ofxKinect: Device %d already connected", id);
		return false;
	}
	
	// open and add to vector
	if(freenect_open_device(kinectContext, &kinect.kinectDevice, id) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Could not open device %d", id);
		return false;
	}
	kinects.insert(pair<int,ofxKinect*>(id, &kinect));
	
	// set kinect id & serial from bus id
	kinect.deviceId = id;
	kinect.serial = deviceList[getDeviceIndex(id)].serial;

	return true;
}

bool ofxKinectContext::open(ofxKinect& kinect, string serial) {
	
	// rebuild if necessary (aka new kinects plugged in)
	buildDeviceList();
	
	if(numConnected() >= numTotal()) {
		ofLog(OF_LOG_WARNING, "ofxKinect: No available devices found");
		return false;
	}
	
	// is the serial available?
	if(isConnected(serial)) {
		ofLog(OF_LOG_WARNING, "ofxKinect: Device %s already connected", serial.c_str());
		return false;
	}
	
	// open and add to vector
	if(freenect_open_device_by_camera_serial(kinectContext, &kinect.kinectDevice, serial.c_str()) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Could not open device %s", serial.c_str());
		return false;
	}
	int index = getDeviceIndex(serial);
	kinects.insert(pair<int,ofxKinect*>(deviceList[index].id, &kinect));
	kinect.deviceId = deviceList[index].id;
	kinect.serial = serial;
	
	return true;
}

void ofxKinectContext::close(ofxKinect& kinect) {

	// check if it's already closed
	int id = -1;
	std::map<int,ofxKinect*>::iterator iter;
	for(iter = kinects.begin(); iter != kinects.end(); ++iter) {
		if(iter->second == &kinect) {
			id = iter->first;
			break;
		}
	}
	if(id == -1)
		return;

	// remove connected device and close
	iter = kinects.find(id);
	if(iter != kinects.end()) {
		kinects.erase(iter);
		freenect_close_device(kinect.kinectDevice);
	}
}

void ofxKinectContext::closeAll() {
	// make copy of map to avoid invalidating iter when calling close()
	std::map<int,ofxKinect*> kinectsCopy(kinects);
    std::map<int,ofxKinect*>::iterator iter;
    for(iter = kinectsCopy.begin(); iter != kinectsCopy.end(); ++iter) {
        iter->second->close();
    }
}

//---------------------------------------------------------------------------
void ofxKinectContext::buildDeviceList() {
	
	deviceList.clear();
	
	// build the device list from freenect
	freenect_device_attributes * devAttrib; 
	int numDevices = freenect_list_device_attributes(kinectContext, &devAttrib);
	
	// save bus ids ...
	for(int i = 0; i < numDevices; i++){
		KinectPair kp;
		kp.id = i;
		kp.serial = (string) devAttrib->camera_serial; 
		deviceList.push_back(kp);
		devAttrib = devAttrib->next;
	}
	freenect_free_device_attributes(devAttrib);
	
	// sort devices by serial number
	sort(deviceList.begin(), deviceList.end(), sortKinectPairs);
}

void ofxKinectContext::listDevices(bool verbose) {
    if(!isInited())
		init();
	
	stringstream stream;
	
	if(numTotal() == 0) {
		stream << "ofxKinect: No devices found";
		return;
	}
	else if(numTotal() == 1) {
		stream << "ofxKinect: " << 1 << " device found";
	}
	else {
		stream << "ofxKinect: " << deviceList.size() << " devices found";
	}
	
	if(verbose) {
		ofLog(OF_LOG_VERBOSE, stream.str());
	}
	else {
		cout << stream.str() << endl;
	}
	stream.str("");
	
	for(int i = 0; i < deviceList.size(); ++i) {
		stream << "    id: " << deviceList[i].id << " serial: " << deviceList[i].serial;
		if(verbose) {
			ofLog(OF_LOG_VERBOSE, stream.str());
		}
		else {
			cout << stream.str() << endl;
		}
		stream.str("");
	}
}

int ofxKinectContext::numTotal() {
    if(!isInited())
		init();
    return freenect_num_devices(kinectContext);
}

int ofxKinectContext::numAvailable() {
	if(!isInited())
		init();
    return freenect_num_devices(kinectContext) - kinects.size();
}

int ofxKinectContext::numConnected() {
	return kinects.size();
}

ofxKinect* ofxKinectContext::getKinect(freenect_device* dev) {
	std::map<int,ofxKinect*>::iterator iter;
	for(iter = kinects.begin(); iter != kinects.end(); ++iter) {
		if(iter->second->kinectDevice == dev)
			return iter->second;
	}
	return NULL;
}

int ofxKinectContext::getDeviceIndex(int id) {
	for(int i = 0; i < deviceList.size(); ++i) {
		if(deviceList[i].id == id)
			return i;
	}
	return -1;
}

int ofxKinectContext::getDeviceIndex(string serial) {
	for(int i = 0; i < deviceList.size(); ++i) {
		if(deviceList[i].serial == serial)
			return i;
	}
	return -1;
}

bool ofxKinectContext::isConnected(int id) {
	std::map<int,ofxKinect*>::iterator iter = kinects.find(id);
	return iter != kinects.end();
}

bool ofxKinectContext::isConnected(string serial) {
	std::map<int,ofxKinect*>::iterator iter;
	for(iter = kinects.begin(); iter != kinects.end(); ++iter) {
		if(iter->second->getSerial() == serial)
			return true;
	}
	return false;
}

int ofxKinectContext::nextAvailableId() {
	if(!isInited())
		init();
	
	// a brute force free index finder :D
	std::map<int,ofxKinect*>::iterator iter;
	for(int i = 0; i < deviceList.size(); ++i) {
		iter = kinects.find(deviceList[i].id);
		if(iter == kinects.end())
			return deviceList[i].id;
	}
	return -1;
}

string ofxKinectContext::nextAvailableSerial() {
	if(!isInited())
		init();
	
	int id = nextAvailableId();
	if(id == -1) {
		return "";
	}
	return deviceList[getDeviceIndex(id)].serial;
}

