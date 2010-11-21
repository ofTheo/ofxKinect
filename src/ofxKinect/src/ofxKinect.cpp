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
	calibratedRGBPixels		= NULL;
	distancePixels 			= NULL;

	bNeedsUpdate			= false;
	bUpdateTex				= false;
	
	bDepthNearValueWhite	= false;

	kinectContext			= NULL;
	kinectDevice			= NULL;
	
	targetTiltAngleDeg		= 0;
	bTiltNeedsApplying		= false;

	thisKinect = this;

	rgbDepthMatrix.getPtr()[0]=0.942040;
	rgbDepthMatrix.getPtr()[1]=-0.005672;
	rgbDepthMatrix.getPtr()[2]=0.000000;
	rgbDepthMatrix.getPtr()[3]=23.953022;
	rgbDepthMatrix.getPtr()[4]=0.004628;
	rgbDepthMatrix.getPtr()[5]=0.939875;
	rgbDepthMatrix.getPtr()[6]=0.000000;
	rgbDepthMatrix.getPtr()[7]=31.486654;
	rgbDepthMatrix.getPtr()[8]=0.000000;
	rgbDepthMatrix.getPtr()[9]=0.000000;
	rgbDepthMatrix.getPtr()[10]=0.000000;
	rgbDepthMatrix.getPtr()[11]=0.000000;
	rgbDepthMatrix.getPtr()[12]=0.000005;
	rgbDepthMatrix.getPtr()[13]=0.000003;
	rgbDepthMatrix.getPtr()[14]=0.000000;
	rgbDepthMatrix.getPtr()[15]=1.000000;
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

//---------------------------------------------------------------------------
unsigned char	* ofxKinect::getDepthPixels(){
	return depthPixels;
}

//---------------------------------------------------------------------------
unsigned short 	* ofxKinect::getRawDepthPixels(){
	return depthPixelsRaw;
}

//---------------------------------------------------------------------------
float* ofxKinect::getDistancePixels() {
	return distancePixels;
}

//---------------------------------------------------------------------------
unsigned char * ofxKinect::getCalibratedRGBPixels(){
	ofxVec3f texcoord3d;
	unsigned char * calibratedPixels = calibratedRGBPixels;
	for ( int y = 0; y < 480; y++) {
		for ( int x = 0; x < 640; x++) {
			texcoord3d.set(x,y,0);
			texcoord3d = rgbDepthMatrix * texcoord3d ;
			texcoord3d.x = ofClamp(texcoord3d.x,0,640);
			texcoord3d.y = ofClamp(texcoord3d.y,0,480);
			int pos = int(texcoord3d.y)*640*3+int(texcoord3d.x)*3;
			*calibratedPixels++ = rgbPixels[pos];
			*calibratedPixels++ = rgbPixels[pos+1];
			*calibratedPixels++ = rgbPixels[pos+2];
		}
	}
	return calibratedRGBPixels;
}

//------------------------------------
ofTexture & ofxKinect::getTextureReference(){
	if(!rgbTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: getTextureReference - texture is not allocated");
	}
	return rgbTex;
}

//---------------------------------------------------------------------------
ofTexture & ofxKinect::getDepthTextureReference(){
	if(!depthTex.bAllocated()){
		ofLog(OF_LOG_WARNING, "ofxKinect: getDepthTextureReference - texture is not allocated");
	}
	return depthTex;
}

//--------------------------------------------------------------------
bool ofxKinect::open(){
	if(!bGrabberInited){
		ofLog(OF_LOG_WARNING, "ofxKinect: Cannot open, init not called");
		return false;
	}

	startThread(true, false);	// blocking, not verbose

	return true;
}

//---------------------------------------------------------------------------
void ofxKinect::close(){
	if(isThreadRunning()){
		stopThread();
	}

	usleep(500000); // some time while thread is stopping ...

	//libusb_exit(NULL);
}

//We update the value here - but apply it in update(). 
//--------------------------------------------------------------------
bool ofxKinect::setCameraTiltAngle(float angleInDegrees){

	//TODO: fix this - it causes the call to libusb_control_transfer to hang. 
	ofLog(OF_LOG_ERROR, "sorry setCameraTiltAngle is not currently working correctly in ofxKinect");
	return false;

	if(!kinectContext){
		return false;
	}

	if ( this->lock() ) {		
		targetTiltAngleDeg = angleInDegrees;
		bTiltNeedsApplying = true;
		printf("ANGLE SET!\n");
		this->unlock();
		printf("unlocked from setCameraTiltAngle!\n");
		
		return true;
	}
	
	return false;
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
	calibratedRGBPixels = new unsigned char[length*3];
	
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

//---------------------------------------------------------------------------
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
	if(!kinectContext){
		return;
	}

	if (!bNeedsUpdate){
		return;
	} else {
		bUpdateTex = true;
	}

	if ( this->lock() ) {

		for (int k = 0; k < width*height; k++){
			// ignore null pixels
			if(depthPixelsBack[k] == 2047) {
				distancePixels[k] = 0;
				depthPixels[k] = 0;
			} else {
				// using equation from https://github.com/OpenKinect/openkinect/wiki/Imaging-Information
				distancePixels[k] = 100.f / (-0.00307f * depthPixelsBack[k] + 3.33f);

			if(bDepthNearValueWhite){
			
//TODO: remove this - why are we doing background thresholding here? we should be providing the actual data.
//ANSWER: because you can't catch these values after they've been interpolated ...

				// filter out the noisey values above 1024
				if(depthPixelsBack[k] < 1024) {
						// invert and convert to 8 bit
						depthPixels[k] = (float) (2048 * 256) / (depthPixelsBack[k] - 2048);
					}
					else {
						depthPixels[k] = 0;
					}
				}
				else {
					// convert to 8 bit
					depthPixels[k] = (float) (2048 * 256) / (2048 - depthPixelsBack[k]);
				}
			}
		}
		memcpy(rgbPixels, rgbPixelsBack, width*height*3);
			
		if( bTiltNeedsApplying ){
			//TODO: find out why this hangs. it is the libusb_control_transfer call that hangs.
			//it seems to only hang when the camera is running
			//maybe we need to pause the grabbing stream?
			
			//freenect_set_tilt_in_degrees(kinectDevice, targetTiltAngleDeg);
			bTiltNeedsApplying = false;
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
float ofxKinect::getDistanceAt(int x, int y) {
	return distancePixels[y * width + x];
}

//------------------------------------
float ofxKinect::getDistanceAt(const ofPoint & p) {
	return getDistanceAt(p.x, p.y);
}

//------------------------------------
ofColor	ofxKinect::getColorAt(int x, int y) {
	int index = (y * width + x) * 3;
	ofColor c;
	c.r = rgbPixels[index++];
	c.g = rgbPixels[index++];
	c.b = rgbPixels[index];
	c.a = 255;

	return c;
}

//------------------------------------
ofColor ofxKinect::getColorAt(const ofPoint & p) {
	return getColorAt(p.x, p.y);
}

//------------------------------------
ofColor ofxKinect::getCalibratedColorAt(int x, int y){
	ofxVec3f texcoord3d;
	texcoord3d.set(x,y,0);
	texcoord3d = rgbDepthMatrix * texcoord3d;
	return getColorAt(ofClamp(texcoord3d.x,0,640),ofClamp(texcoord3d.y,0,480));
}

//------------------------------------
ofColor ofxKinect::getCalibratedColorAt(const ofPoint & p){
	return getCalibratedColorAt(p.x,p.y);
}

//------------------------------------
ofxMatrix4x4 ofxKinect::getRGBDepthMatrix(){
	return rgbDepthMatrix;
}

//------------------------------------
void ofxKinect::setRGBDepthMatrix(const ofxMatrix4x4 & matrix){
	rgbDepthMatrix=matrix;
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

//----------------------------------------------------------
void ofxKinect::draw(float _x, float _y){
	draw(_x, _y, (float)width, (float)height);
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
void ofxKinect::enableDepthNearValueWhite(bool bEnabled)
{
	bDepthNearValueWhite = bEnabled;
}

//---------------------------------------------------------------------------
bool ofxKinect::isDepthNearValueWhite()
{
	return bDepthNearValueWhite;
}

/* ***** PRIVATE ***** */

//---------------------------------------------------------------------------
void ofxKinect::grabDepthFrame(freenect_device *dev, freenect_depth *depth, uint32_t timestamp) {
	if (thisKinect->lock()) {
		try {
			memcpy(thisKinect->depthPixelsBack, depth, width*height*sizeof(uint16_t));
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
void ofxKinect::grabRgbFrame(freenect_device *dev, freenect_pixel *rgb, uint32_t timestamp) {
	if (thisKinect->lock()) {
		try {
			memcpy(thisKinect->rgbPixelsBack, rgb, FREENECT_RGB_SIZE);
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
	if (freenect_init(&kinectContext, NULL) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: freenet_init failed");
	}

	int number_devices = freenect_num_devices(kinectContext);
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Number of Devices found: " + ofToString(number_devices));

	if (number_devices < 1) {
		ofLog(OF_LOG_ERROR, "ofxKinect: didnt find a device");
		return;
	}
	
	if (freenect_open_device(kinectContext, &kinectDevice, 0) < 0) {
		ofLog(OF_LOG_ERROR, "ofxKinect: could not open device");
		return;
	}
	
	freenect_set_led(kinectDevice, LED_GREEN);
	freenect_set_depth_callback(kinectDevice, &grabDepthFrame);
	freenect_set_rgb_callback(kinectDevice, &grabRgbFrame);
	freenect_set_rgb_format(kinectDevice, FREENECT_FORMAT_RGB);
	freenect_set_depth_format(kinectDevice, FREENECT_FORMAT_11_BIT);
	
	ofLog(OF_LOG_VERBOSE, "ofxKinect: Connection opened");
	
	freenect_start_depth(kinectDevice);
	freenect_start_rgb(kinectDevice);
	
	while (isThreadRunning()) {
		int16_t ax,ay,az;
		freenect_get_raw_accelerometers(kinectDevice, &ax, &ay, &az);
		rawAccel.set(ax, ay, az);
		
		double dx,dy,dz;
		freenect_get_mks_accelerometers(kinectDevice, &dx, &dy, &dz);
		mksAccel.set(dx, dy, dz);
		
//		printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", ax, ay, az, dx, dy, dz);
	}
}
