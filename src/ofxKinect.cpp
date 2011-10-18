#include "ofxKinect.h"
#include "ofMain.h"

#include "libfreenect-registration.h"

// context static
ofxKinectContext ofxKinect::kinectContext;

//--------------------------------------------------------------------
ofxKinect::ofxKinect() {
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Creating ofxKinect");
	
	bVerbose = false;
	bUseTexture = true;
	bGrabVideo = true;
	
	// set defaults
	bGrabberInited = false;
	depthPixelsRaw = NULL;
	depthPixelsBack = NULL;
	videoPixels = NULL;
	videoPixelsBack = NULL;
	
	bNeedsUpdate = false;
	bUpdateTex = false;
	bIsFrameNew = false;
	
	kinectDevice = NULL;
	
	targetTiltAngleDeg = 0;
	currentTiltAngleDeg = 0;
	bTiltNeedsApplying = false;
	
	bUseRegistration = false;
	bNearWhite = true;
	
	setClippingInMillimeters();
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
unsigned char * ofxKinect::getDepthPixels(){
	return depthPixels;
}

//---------------------------------------------------------------------------
unsigned short * ofxKinect::getRawDepthPixels(){
	return depthPixelsRaw;
}

//---------------------------------------------------------------------------
float* ofxKinect::getDistancePixels() {
	return distancePixels;
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
bool ofxKinect::open(int id){
	if(!bGrabberInited){
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}
	
	if(kinectContext.numAvailable() < 1) {
		ofLog(OF_LOG_ERROR, "ofxKinect: No available devices found");
		return false;
	}
	
	if(!kinectContext.open(*this, id)) {
		return false;
	}
	
	freenect_set_user(kinectDevice, this);
	freenect_set_depth_callback(kinectDevice, &grabDepthFrame);
	freenect_set_video_callback(kinectDevice, &grabRgbFrame);
	
	startThread(true, false); // blocking, not verbose
	
	return true;
}

//---------------------------------------------------------------------------
void ofxKinect::close(){
	if(isThreadRunning()){
		waitForThread(true);
	}
	
	bIsFrameNew = false;
	bNeedsUpdate = false;
	bUpdateTex = false;
}

//---------------------------------------------------------------------------
bool ofxKinect::isConnected(){
	return isThreadRunning();
}

//---------------------------------------------------------------------------
int ofxKinect::getDeviceId() {
	return kinectContext.getId(*this);
}

// we update the value here - but apply it in kinect thread
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

void ofxKinect::setUseRegistration(bool bUseRegistration) {
	this->bUseRegistration = bUseRegistration;
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
	
	bUseTexture = texture;
	
	int length = width*height;
	depthPixelsRaw = new unsigned short[length];
	depthPixelsBack = new unsigned short[length];
	
	videoPixels = new unsigned char[length*bytespp];
	pixels.setFromExternalPixels(videoPixels, width, height, OF_IMAGE_COLOR);
	videoPixelsBack = new unsigned char[length*bytespp];
	
	depthPixels = new unsigned char[length * bytespp];
	distancePixels = new float[length];
	
	memset(depthPixelsRaw, 0, length*sizeof(unsigned short));
	memset(depthPixelsBack, 0, length*sizeof(unsigned short));
	
	memset(videoPixels, 0, length*bytespp*sizeof(unsigned char));
	memset(videoPixelsBack, 0, length*bytespp*sizeof(unsigned char));
	
	if(bUseTexture){
		depthTex.allocate(width, height, GL_LUMINANCE);
		videoTex.allocate(width, height, infrared ? GL_LUMINANCE : GL_RGB);
	}
	
	if(!kinectContext.isInited()) {
		if(!kinectContext.init()) {
			return false;
		}
	}
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Number of devices found: %d", kinectContext.numTotal());
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Number of available devices: %d", kinectContext.numAvailable());
	
	bGrabberInited = true;
	
	return bGrabberInited;
}

//---------------------------------------------------------------------------
void ofxKinect::clear(){
	if(isConnected()){
		ofLog(OF_LOG_WARNING, "ofxKinect: Do not call clear while ofxKinect is running!");
		return;
	}
	
	if(kinectContext.numConnected() < 1) {
		kinectContext.clear();
	}
	
	if(depthPixelsRaw != NULL){
		delete[] depthPixelsRaw; depthPixelsRaw = NULL;
		delete[] depthPixelsBack; depthPixelsBack = NULL;
		
		delete[] videoPixels; videoPixels = NULL;
		delete[] videoPixelsBack; videoPixelsBack = NULL;
		
		delete [] depthPixels; depthPixels = NULL;
		delete [] distancePixels; distancePixels = NULL;
	}
	
	depthTex.clear();
	videoTex.clear();
	
	bGrabberInited = false;
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
		
		memcpy(depthPixelsRaw, depthPixelsBack, n * sizeof(short));
		memcpy(videoPixels, videoPixelsBack, n * bytespp);
		
		//we have done the update
		bNeedsUpdate = false;
		
		this->unlock();
		
		updateDepthPixels();
	}
	
	if(bUseTexture){
		depthTex.loadData(depthPixels, width, height, GL_LUMINANCE);
		videoTex.loadData(videoPixels, width, height, bInfrared ? GL_LUMINANCE : GL_RGB);
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
	bNearWhite = bEnabled;
	updateDepthLookupTable();
}

//---------------------------------------------------------------------------
bool ofxKinect::isDepthNearValueWhite(){
	return bNearWhite;
}

//---------------------------------------------------------------------------
void ofxKinect::setClippingInMillimeters(float nearClipping, float farClipping) {
	this->nearClipping = nearClipping;
	this->farClipping = farClipping;
	updateDepthLookupTable();
}

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

/* ***** PRIVATE ***** */

//---------------------------------------------------------------------------
void ofxKinect::grabDepthFrame(freenect_device *dev, void *depth, uint32_t timestamp) {
	
	ofxKinect* kinect = kinectContext.getKinect(dev);
	
	if(kinect->kinectDevice == dev && kinect->lock()) {
		try {
			freenect_frame_mode curMode = freenect_get_current_depth_mode(dev);
			memcpy(kinect->depthPixelsBack, depth, curMode.bytes);
			kinect->bNeedsUpdate = true;
		}
		catch(...) {
			ofLog(OF_LOG_ERROR, "ofxKinect: Depth memcpy failed for device %d", kinect->getDeviceId());
		}
		kinect->unlock();
	}
	else {
		ofLog(OF_LOG_WARNING, "ofxKinect: grabDepthFrame unable to lock mutex");
	}
}

//---------------------------------------------------------------------------
void ofxKinect::grabRgbFrame(freenect_device *dev, void *rgb, uint32_t timestamp) {
	
	ofxKinect* kinect = kinectContext.getKinect(dev);
	
	if(kinect->kinectDevice == dev && kinect->lock()) {
		try {
			freenect_frame_mode curMode = freenect_get_current_video_mode(dev);
			memcpy(kinect->videoPixelsBack, rgb, curMode.bytes);
			kinect->bNeedsUpdate = true;
		}
		catch (...) {
			ofLog(OF_LOG_ERROR, "ofxKinect: Rgb memcpy failed for device %d", kinect->getDeviceId());
		}
		kinect->unlock();
	}
	else {
		ofLog(OF_LOG_ERROR, "ofxKinect: grabRgbFrame unable to lock mutex");
	}
}

//---------------------------------------------------------------------------
void ofxKinect::threadedFunction(){ 
	
	freenect_set_led(kinectDevice, LED_GREEN);
	freenect_frame_mode videoMode = freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, bInfrared ? FREENECT_VIDEO_IR_8BIT : FREENECT_VIDEO_RGB);
	freenect_set_video_mode(kinectDevice, videoMode);
	freenect_frame_mode depthMode = freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, bUseRegistration ? FREENECT_DEPTH_REGISTERED : FREENECT_DEPTH_MM);
	freenect_set_depth_mode(kinectDevice, depthMode);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Device %d connection opened", kinectContext.getId(*this));
	
	freenect_start_depth(kinectDevice);
	if(bGrabVideo) {
		freenect_start_video(kinectDevice);
	}
	
	// call platform specific processors (needed for Win)
	if(freenect_process_events(kinectContext.getContext()) != 0){
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
		
		// printf("\r raw acceleration: %4d %4d %4d mks acceleration: %4f %4f %4f", ax, ay, az, dx, dy, dz);
	}
	
	// finish up a tilt on exit
	if(bTiltNeedsApplying){
		freenect_set_tilt_degs(kinectDevice, targetTiltAngleDeg);
		bTiltNeedsApplying = false;
	}
	
	freenect_stop_depth(kinectDevice);
	freenect_stop_video(kinectDevice);
	freenect_set_led(kinectDevice, LED_YELLOW);
	
	int id = kinectContext.getId(*this);
	kinectContext.close(*this);
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Device %d connection closed", id);
}

//---------------------------------------------------------------------------
int ofxKinect::numTotalDevices() {
	return kinectContext.numTotal();
}

int ofxKinect::numAvailableDevices() {
	return kinectContext.numAvailable();
}

int ofxKinect::numConnectedDevices() {
	return kinectContext.numConnected();
}

bool ofxKinect::isDeviceConnected(int id) {
	return kinectContext.isConnected(id);
}

int ofxKinect::nextAvailableId() {
	return kinectContext.nextAvailableId();
}

//---------------------------------------------------------------------------
// ofxKinectContext
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
ofxKinectContext::ofxKinectContext() {
	kinectContext = NULL;
}
ofxKinectContext::~ofxKinectContext() {
	closeAll();
	clear();
}

//---------------------------------------------------------------------------
bool ofxKinectContext::init() {
	if(freenect_init(&kinectContext, NULL) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: freenect_init failed");
		return false;
	}
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Context inited");
	return true;
}

void ofxKinectContext::clear() {
	if(isInited() && numConnected() < 1) {
		freenect_shutdown(kinectContext);
		kinectContext = NULL;
		ofLog(OF_LOG_VERBOSE, "ofxKinect: Context cleared");
	}
}

bool ofxKinectContext::isInited() {
	return kinectContext != NULL;
}

bool ofxKinectContext::open(ofxKinect& kinect, int id) {
	
	if(numConnected() >= numTotal()) {
		ofLog(OF_LOG_WARNING, "ofxKinect Cannot open any more devices");
		return false;
	}
	
	// is the id available?
	if(id < 0) {
		id = nextAvailableId();
	}
	else {
		if(isConnected(id)) {
			ofLog(OF_LOG_WARNING, "ofxKinect: Device %d already connected", id);
			return false;
		}
	}
	
	// open and add to vector
	if(freenect_open_device(kinectContext, &kinect.kinectDevice, id) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: Could not open device %d", id);
		return false;
	}
	kinects.insert(pair<int,ofxKinect*>(id, &kinect));
	
	return true;
}

void ofxKinectContext::close(ofxKinect& kinect) {
	
	// already closed?
	int id = getId(kinect);
	if(id == -1) {
		return;
	}
	
	// remove connected device and close
	std::map<int,ofxKinect*>::iterator iter = kinects.find(id);
	if(iter != kinects.end()) {
		kinects.erase(iter);
		freenect_close_device(kinect.kinectDevice);
	}
}

void ofxKinectContext::closeAll() {
	std::map<int,ofxKinect*>::iterator iter;
	for(iter = kinects.begin(); iter != kinects.end(); ++iter) {
		iter->second->close();
	}
}

//---------------------------------------------------------------------------
int ofxKinectContext::numTotal() {
	if(isInited())
		return freenect_num_devices(kinectContext);
	return 0;
}

int ofxKinectContext::numAvailable() {
	if(isInited())
		return freenect_num_devices(kinectContext) - kinects.size();
	return 0;
}

int ofxKinectContext::numConnected() {
	return kinects.size();
}

int ofxKinectContext::getId(ofxKinect& kinect) {
	std::map<int,ofxKinect*>::iterator iter;
	for(iter = kinects.begin(); iter != kinects.end(); ++iter) {
		if(iter->second == &kinect)
			return iter->first;
	}
	return -1;
}

ofxKinect* ofxKinectContext::getKinect(freenect_device* dev) {
	std::map<int,ofxKinect*>::iterator iter;
	for(iter = kinects.begin(); iter != kinects.end(); ++iter) {
		if(iter->second->kinectDevice == dev)
			return iter->second;
	}
	return NULL;
}

bool ofxKinectContext::isConnected(int index) {
	std::map<int,ofxKinect*>::iterator iter = kinects.find(index);
	return iter != kinects.end();
}

int ofxKinectContext::nextAvailableId() {
	
	// a brute force free index finder :D
	std::map<int,ofxKinect*>::iterator iter;
	for(int i = 0; i < numTotal(); ++i) {
		iter = kinects.find(i);
		if(iter == kinects.end())
			return i;
	}
	return -1;
}

