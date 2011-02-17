#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

class testApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();
	
		void drawPointCloud();

		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		
		void startRecording();
		void stopRecording();
		void startPlayback();
		void stopPlayback();

		ofxKinect kinect;
		ofxKinectRecorder kinectRecorder;
		ofxKinectPlayer kinectPlayer;
		
		/// used to switch between the live kinect and the recording player
		ofxBase3DVideo* kinectSource;

		ofxCvColorImage		colorImg;

		ofxCvGrayscaleImage		grayBackground;
		ofxCvGrayscaleImage		grayBackgroundDiff;
		ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayThreshNear;
		ofxCvGrayscaleImage 	grayThreshFar;

		ofxCvContourFinder 	contourFinder;
		
		bool				bLearnBackground;
		
		bool				bThreshWithOpenCV;
		bool				drawPC;

		int 				nearThreshold;
		int					farThreshold;

		int					angle;
		
		int 				pointCloudRotationY;
		
		bool 				bRecord;
		bool 				bPlayback;
};
