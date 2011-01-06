/*
 * ofxKinectCalibration.cpp
 *
 *  Created on: 03/01/2011
 *      Author: arturo
 */

#include "ofxKinectCalibration.h"

/*
 these values constrain the maximum distance in the depthPixels image to:
 - as near as possible (raw value of 0)
 - 4 meters away, maximum
 both near and far clipping planes should be user-settable
 */
float ofxKinectCalibration::nearClipping = rawToCentimeters(0), ofxKinectCalibration::farClipping = 400;

bool ofxKinectCalibration::lookupsCalculated = false;
float ofxKinectCalibration::distancePixelsLookup[2048];
unsigned char ofxKinectCalibration::depthPixelsLookupNearWhite[2048];
unsigned char ofxKinectCalibration::depthPixelsLookupFarWhite[2048];

double ofxKinectCalibration::fx_d = 1.0 / 5.9421434211923247e+02;
double ofxKinectCalibration::fy_d = 1.0 / 5.9104053696870778e+02;
float ofxKinectCalibration::cx_d = 3.3930780975300314e+02;
float ofxKinectCalibration::cy_d = 2.4273913761751615e+02;
double ofxKinectCalibration::fx_rgb = 5.2921508098293293e+02;
double ofxKinectCalibration::fy_rgb = 5.2556393630057437e+02;
float ofxKinectCalibration::cx_rgb = 3.2894272028759258e+02;
float ofxKinectCalibration::cy_rgb = 2.6748068171871557e+02;

ofxKinectCalibration::ofxKinectCalibration():
	T_rgb( 1.9985242312092553e-02f, -7.4423738761617583e-04f, -1.0916736334336222e-02f ),
	R_rgb(9.9984628826577793e-01f, 1.2635359098409581e-03f, -1.7487233004436643e-02f, 0,
		 -1.4779096108364480e-03f, 9.9992385683542895e-01f, -1.2251380107679535e-02f, 0,
		  1.7470421412464927e-02f, 1.2275341476520762e-02f, 9.9977202419716948e-01f, 0,
		  0,0,0,1)
{
	depthPixels				= NULL;
	calibratedRGBPixels		= NULL;
	distancePixels 			= NULL;
	bDepthNearValueWhite	= false;
	calculateLookups();
	R_rgb.preMultTranslate(-T_rgb);
	R_rgb = ofxMatrix4x4::getTransposedOf(R_rgb);
}

inline float ofxKinectCalibration::rawToCentimeters(unsigned short raw) {
	return 100 * (k1 * tanf((raw / k2) + k3) - k4);
}

inline unsigned short ofxKinectCalibration::centimetersToRaw(float centimeters) {
	return (unsigned short) (k2 * (atanf((k4 + (centimeters / 100)) / k1) - k3));
}

void ofxKinectCalibration::setClippingInCentimeters(float nearClipping, float farClipping) {
	ofxKinectCalibration::nearClipping = nearClipping;
	ofxKinectCalibration::farClipping = farClipping;
	lookupsCalculated = false;
	calculateLookups();
}

float ofxKinectCalibration::getNearClipping() {
	return nearClipping;
}

float ofxKinectCalibration::getFarClipping() {
	return farClipping;
}

void ofxKinectCalibration::calculateLookups() {
	if(!lookupsCalculated) {
		ofLog(OF_LOG_VERBOSE, "Setting up LUT for distance and depth values.");
		
		for(int i = 0; i < 2048; i++){
			if(i == 2047) {
				distancePixelsLookup[i] = 0;
				depthPixelsLookupNearWhite[i] = 0;
				depthPixelsLookupFarWhite[i] = 0;
			} else {
				distancePixelsLookup[i] = rawToCentimeters(i);
				depthPixelsLookupNearWhite[i] = ofMap(distancePixelsLookup[i], nearClipping, farClipping, 0, 255, true);
				depthPixelsLookupFarWhite[i] = 255 - depthPixelsLookupNearWhite[i];
			}
		}
	}
	lookupsCalculated = true;
}

ofxKinectCalibration::~ofxKinectCalibration() {
	// TODO Auto-generated destructor stub
}

void ofxKinectCalibration::init(int _bytespp){
	int length = width*height;
	bytespp = _bytespp;
	depthPixels = new unsigned char[length];
	distancePixels = new float[length];
	calibratedRGBPixels = new unsigned char[length*bytespp];

	memset(depthPixels, 0, length*sizeof(unsigned char));
	memset(distancePixels, 0, length*sizeof(float));

}

void ofxKinectCalibration::clear(){

	if(depthPixels != NULL){
		delete[] depthPixels; depthPixels = NULL;
		delete[] distancePixels; distancePixels = NULL;
	}
}

void ofxKinectCalibration::update(unsigned short * depth){
	int n = width * height;
	if(bDepthNearValueWhite) {
		for(int i = 0; i < n; i++){
			distancePixels[i] = distancePixelsLookup[depth[i]];
			depthPixels[i] = depthPixelsLookupNearWhite[depth[i]];
		}
	} else {
		for(int i = 0; i < n; i++){
			distancePixels[i] = distancePixelsLookup[depth[i]];
			depthPixels[i] = depthPixelsLookupFarWhite[depth[i]];
		}
	}
}

void ofxKinectCalibration::enableDepthNearValueWhite(bool bEnabled){
	bDepthNearValueWhite = bEnabled;
}

bool ofxKinectCalibration::isDepthNearValueWhite(){
	return bDepthNearValueWhite;
}

unsigned char * ofxKinectCalibration::getCalibratedRGBPixels(unsigned char * rgb){
	//calibration method from:  http://nicolas.burrus.name/index.php/Research/KinectCalibration
	static ofxVec3f texcoord3d;
	static ofxVec2f texcoord2d;
	unsigned char * calibratedPixels = calibratedRGBPixels;
	float * _distancePixels = distancePixels;

	for ( int y = 0; y < 480; y++) {
		for ( int x = 0; x < 640; x++) {
			texcoord3d = getWorldCoordinateFor(x,y,(*_distancePixels++)*.01);
			if(texcoord3d.z){
				texcoord3d = R_rgb * texcoord3d;
				const float invZ = 1.0f / texcoord3d.z;
				texcoord2d.x = ofClamp(round(texcoord3d.x * fx_rgb *invZ) + cx_rgb,0,639);
				texcoord2d.y = ofClamp(round(texcoord3d.y * fy_rgb *invZ) + cy_rgb,0,479);

				int pos = int(texcoord2d.y)*640*3+int(texcoord2d.x)*3;
				*calibratedPixels++ = rgb[pos];
				*calibratedPixels++ = rgb[pos+1];
				*calibratedPixels++ = rgb[pos+2];
			}else{
				*calibratedPixels++ = 0;
				*calibratedPixels++ = 0;
				*calibratedPixels++ = 0;
			}
		}
	}
	return calibratedRGBPixels;
}

unsigned char * ofxKinectCalibration::getDepthPixels(){
	return depthPixels;
}

float * ofxKinectCalibration::getDistancePixels(){
	return distancePixels;
}

ofxPoint2f ofxKinectCalibration::getCalibratedColorCoordAt(int x, int y){
	//calibration method from:  http://nicolas.burrus.name/index.php/Research/KinectCalibration
	ofxVec3f texcoord3d;
	ofxVec2f texcoord2d;
	texcoord3d = getWorldCoordinateFor(x,y);
	texcoord3d = R_rgb * texcoord3d;
	const float invZ = 1/ texcoord3d.z;
	texcoord2d.x = ofClamp((texcoord3d.x * fx_rgb *invZ) + cx_rgb,0,640);
	texcoord2d.y = ofClamp((texcoord3d.y * fy_rgb *invZ) + cy_rgb,0,480);

	return texcoord2d;
}


float ofxKinectCalibration::getDistanceAt(int x, int y){
	return distancePixels[y * width + x];
}

float ofxKinectCalibration::getDistanceAt(const ofPoint & p){
	return getDistanceAt(p.x, p.y);
}

ofxPoint2f ofxKinectCalibration::getCalibratedColorCoordAt(const ofPoint & p){
	return getCalibratedColorCoordAt(p.x,p.y);
}

ofxPoint3f ofxKinectCalibration::getWorldCoordinateFor(int x, int y){
	const double depth = getDistanceAt(x,y)/100.0;
	return getWorldCoordinateFor(x,y,depth);
}

ofxPoint3f ofxKinectCalibration::getWorldCoordinateFor(int x, int y, double z){
	//Based on http://graphics.stanford.edu/~mdfisher/Kinect.html

	ofxVec3f result;
	result.x = float((x - cx_d) * z * fx_d);
	result.y = float((y - cy_d) * z * fy_d);
	result.z = z;

	return result;
}
