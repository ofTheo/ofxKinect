/*
 * KinectRecorder.cpp
 *
 *  Created on: 03/01/2011
 *      Author: arturo
 */

#include "ofxKinectRecorder.h"

ofxKinectRecorder::ofxKinectRecorder() {
	f=NULL;

}

ofxKinectRecorder::~ofxKinectRecorder() {
	// TODO Auto-generated destructor stub
}

void ofxKinectRecorder::init(const string & filename){
	f = fopen(ofToDataPath(filename).c_str(),"wb");
}

void ofxKinectRecorder::newFrame(unsigned char* rgb, unsigned short * raw_depth){
	if(!f) return;
	fwrite(rgb,640*480*3,1,f);
	fwrite(raw_depth,640*480*sizeof(short),1,f);
}

void ofxKinectRecorder::close(){
	if(!f) return;
	fclose(f);
	f=0;
}

bool ofxKinectRecorder::isOpened(){
	return f;
}
