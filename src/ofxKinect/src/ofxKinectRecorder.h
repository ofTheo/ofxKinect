/*
 * KinectRecorder.h
 *
 *  Created on: 03/01/2011
 *      Author: arturo
 */

#ifndef OFXKINECTRECORDER_H_
#define OFXKINECTRECORDER_H_

#include "ofMain.h"

class ofxKinectRecorder {
public:
	ofxKinectRecorder();
	virtual ~ofxKinectRecorder();

	void init(const string & filename);
	void newFrame(unsigned char* rgb, unsigned short * raw_depth);
	void close();

	bool isOpened();

private:
	FILE * f;
};

#endif /* KINECTRECORDER_H_ */
