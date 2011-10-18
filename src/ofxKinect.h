#pragma once

#include "ofMain.h"
#include "libfreenect.h"
#include "ofxBase3DVideo.h"

#ifndef _MSC_VER
#include <libusb.h>
#endif

class ofxKinectContext;

/// \class ofxKinect
///
/// wrapper for a freenect kinect device
///
/// references:
/// - http://openkinect.org/wiki/Main_Page
/// - https://github.com/OpenKinect/libfreenect/blob/master/include/libfreenect.h
///
class ofxKinect : public ofxBase3DVideo, protected ofThread {
	
public:
	
	ofxKinect();
	virtual ~ofxKinect();
	
	/// are the current frames new?
	bool isFrameNew();
	
	/// open the connection and start grabbing images
	bool open(int id = -1);
	
	/// close the connection and stop grabbing images
	void close();
	
	/// distorts the depth image to be aligned to the rgb image
	void setUseRegistration(bool bUseRegistration = false);
	
	/// initialize resources, must be called before open()
	/// infrared controls whether the video image is rgb or IR
	/// set vdeo to false to disabel vidoe image grabbing (saves bandwidth)
	/// set texture to false if you don't need to use the internal textures
	///
	/// naturally, if you disable the video image the video pixels and
	/// RGB color will be 0
	///
	bool init(bool infrared=false, bool video=true, bool texture=true);
	
	/// updates the pixel buffers and textures - make sure to call this to update to the latetst incoming frames
	void update(); 
	
	/// clear resources, do not call this while ofxKinect is running!
	void clear();
	
	/// is the connection currently open?
	bool isConnected();
	
	/// get the device id
	/// returns -1 if not connected
	int getDeviceId();
	
	/// set tilt angle of the camera in degrees
	/// 0 is flat, the range is -30 to 30
	bool setCameraTiltAngle(float angleInDegrees);
	
	// get camera angle
	float getCurrentCameraTiltAngle();
	float getTargetCameraTiltAngle();
	
	float getDistanceAt(int x, int y);
	float getDistanceAt(const ofPoint & p);
	
	/// calculates the coordinate in the world for the pixel (perspective calculation). Center of image is (0.0)
	ofVec3f getWorldCoordinateAt(int cx, int cy);
	ofVec3f getWorldCoordinateAt(float cx, float cy, float wz);
	
	ofColor getColorAt(int x, int y);
	ofColor getColorAt(const ofPoint & p);
	
	ofColor getCalibratedColorAt(int x, int y);
	ofColor getCalibratedColorAt(const ofPoint & p);
	
	float getHeight();
	float getWidth();
	
	ofPoint getRawAccel();
	ofPoint getMksAccel(); 
	
	/// get the pixels of the most recent rgb frame
	unsigned char * getPixels();
	
	/// get the pixels of the most recent depth frame
	unsigned char * getDepthPixels(); // grey scale values
	unsigned short * getRawDepthPixels(); // raw 11 bit values
	
	// get the rgb pixels corrected to match the depth frame
	unsigned char * getCalibratedRGBPixels();
	
	/// get the distance in millimeters to a given point
	float* getDistancePixels();
	
	/// get the rgb texture
	ofTexture & getTextureReference();
	
	/// get the greyscale depth texture
	ofTexture & getDepthTextureReference();
	
	/// for 007 compatibility
	ofPixels & getPixelsRef();
	
	/**
	 set the near value of the pixels in the greyscale depth image to white?
	 
	 bEnabled = true : pixels close to the camera are brighter (default)
	 bEnabled = false: pixels closer to the camera are darker
	 **/
	void enableDepthNearValueWhite(bool bEnabled=true);
	bool isDepthNearValueWhite();
	void setClippingInMillimeters(float nearClipping = 500, float farClipping = 4000);
	
	void setVerbose(bool bTalkToMe);
	
	void setUseTexture(bool bUse);
	void draw(float x, float y, float w, float h);
	void draw(float x, float y);
	void draw(const ofPoint & point);
	void draw(const ofRectangle & rect);
	
	void drawDepth(float x, float y, float w, float h);
	void drawDepth(float x, float y);
	void drawDepth(const ofPoint & point);
	void drawDepth(const ofRectangle & rect);
	
	const static int width = 640;
	const static int height = 480;
	
	/// \section Static global kinect context functions
	
	/// get the total number of devices
	static int numTotalDevices();
	
	/// get the number of available devices (not connected)
	static int numAvailableDevices();
	
	/// get the number of currently connected devices
	static int numConnectedDevices();
	
	/// is the an id already connected?
	static bool isDeviceConnected(int id);
	
	/// get the id of the next available device,
	/// returns -1 if nothing found
	static int nextAvailableId();
	
protected:
	
	bool bUseTexture;
	ofTexture depthTex; // the depth texture
	ofTexture videoTex; // the RGB texture
	bool bVerbose;
	bool bGrabberInited;
	
	ofPixels pixels;
	unsigned char * videoPixels;
	unsigned short * depthPixelsRaw;
	
	ofPoint rawAccel;
	ofPoint mksAccel;
	
	float targetTiltAngleDeg;
	float currentTiltAngleDeg;
	bool bTiltNeedsApplying;
	
private:
	
	friend class ofxKinectContext;
	
	// global statics shared between kinect instances
	static ofxKinectContext kinectContext;
	
	freenect_device * kinectDevice; // kinect device handle
	
	unsigned short * depthPixelsBack; // depth back
	unsigned char * videoPixelsBack; // rgb back
	
	unsigned char * depthPixels;
	float * distancePixels;
	vector<unsigned char> depthLookupTable;
	void updateDepthLookupTable();
	void updateDepthPixels();
	
	bool bIsFrameNew;
	bool bNeedsUpdate;
	bool bUpdateTex;
	bool bGrabVideo;
	bool bUseRegistration;
	bool bNearWhite;
	
	float nearClipping, farClipping;
	
	bool bInfrared;
	int bytespp;
	
	// libfreenect callbacks
	static void grabDepthFrame(freenect_device *dev, void *depth, uint32_t timestamp);
	static void grabRgbFrame(freenect_device *dev, void *rgb, uint32_t timestamp);
	
	// thread function
	void threadedFunction();
};

/// \class ofxKinect
///
/// wrapper for the freenect context
///
/// do not use this directly
///
class ofxKinectContext {
	
public:
	
	ofxKinectContext();
	~ofxKinectContext();
	
	/// \section Main
	
	/// init the freenect context
	bool init();
	
	/// clear the freenect context
	/// closes all currently connected devices
	void clear();
	
	/// is the context inited?
	bool isInited();
	
	/// open a kinect device
	/// an id of -1 will open the first available
	bool open(ofxKinect& kinect, int id=-1);
	
	/// close a kinect device
	void close(ofxKinect& kinect);
	
	/// closes all currently connected kinects
	void closeAll();
	
	/// \section Util
	
	/// get the total number of devices
	int numTotal();
	
	/// get the number of available devices (not connected)
	int numAvailable();
	
	/// get the number of currently connected devices
	int numConnected();
	
	/// get the device id of a kinect object
	/// returns index or -1 if not connected
	int getId(ofxKinect& kinect);
	
	/// get the kinect object from a device pointer
	/// returns NULL if not found
	ofxKinect* getKinect(freenect_device* dev);
	
	/// is the an id already connected?
	bool isConnected(int id);
	
	/// get the id of the next available device,
	/// returns -1 if nothing found
	int nextAvailableId();
	
	/// get the raw pointer
	freenect_context* getContext() {return kinectContext;}
	
private:
	
	freenect_context * kinectContext; // kinect context handle
	std::map<int,ofxKinect*> kinects; // the connected kinects
};
