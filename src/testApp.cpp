#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
	// kinect.init(true)  //shows infrared image
	kinect.init();
	kinect.setVerbose(true);
	kinect.open();

	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);

	nearThreshold = 50;
	farThreshold  = 180;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);

	angle = 0;
	kinect.setCameraTiltAngle(angle);
}

//--------------------------------------------------------------
void testApp::update()
{
	ofBackground(100, 100, 100);
	kinect.update();

	grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
	//we do two thresholds - one for the far plane and one for the near plane
	//we then do a cvAnd to get the pixels which are a union of the two thresholds.	
	if( bThreshWithOpenCV ){
		grayThreshFar = grayImage;
		grayThresh = grayImage;
		grayThreshFar.threshold(farThreshold, true);
		grayThresh.threshold(nearThreshold);
		cvAnd(grayThresh.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
	}else{
	
		//or we do it ourselves - show people how they can work with the pixels
	
		unsigned char * pix = grayImage.getPixels();
		int numPixels = grayImage.getWidth() * grayImage.getHeight();

		for(int i = 0; i < numPixels; i++){
			if( pix[i] > nearThreshold && pix[i] < farThreshold ){
				pix[i] = 255;
			}else{
				pix[i] = 0;
			}
		}
	}

	//update the cv image
	grayImage.flagImageChanged();

    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
}

//--------------------------------------------------------------
void testApp::draw()
{
	ofSetColor(255, 255, 255);

	kinect.drawDepth(10, 10, 400, 300);
	kinect.draw(420, 10, 400, 300);
	
	grayImage.draw(10, 320, 400, 300);
	contourFinder.draw(10, 320, 400, 300);
	
	// point cloud is commented out because we need a proper camera class to explore it effectively
	/*
	ofPushMatrix();
	ofTranslate(420, 320);
	drawPointCloud();
	ofPopMatrix();
	 */


	ofSetColor(255, 255, 255);
	ofDrawBitmapString("accel is: " + ofToString(kinect.getMksAccel().x, 2) + " / " 
									+ ofToString(kinect.getMksAccel().y, 2) + " / "
									+ ofToString(kinect.getMksAccel().z, 2), 20, 658 );

	char reportStr[1024];
	sprintf(reportStr, "using opencv threshold = %i (press spacebar)\nset near threshold %i (press: + -)\nset far threshold %i (press: < >) num blobs found %i, fps: %f",bThreshWithOpenCV, nearThreshold, farThreshold, contourFinder.nBlobs, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 690);
	ofDrawBitmapString("tilt angle: " + ofToString(angle),20,670);
}

void testApp::drawPointCloud() {
	int step = 2;
	ofScale(1. / step, 1. / step, 1. / step);
	// two magic numbers derived visually
	float scaleFactor = .0021;
	float minDistance = -10;
	int w = 640;
	int h = 480;
	ofTranslate(w / 2, h / 2);
	ofRotateY(mouseX);
	ofTranslate(-w / 2, -h / 2);
	float* distancePixels = kinect.getDistancePixels();
	glBegin(GL_POINTS);
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			int i = y * w + x;
			float z = distancePixels[i];
			float scaledx = (x - w / 2) * (z + minDistance) * scaleFactor;
			float scaledy = (y - h / 2) * (z + minDistance) * scaleFactor;
			glVertex3f(scaledx, scaledy, z);
		}
	}
	glEnd();
}

//--------------------------------------------------------------
void testApp::exit(){
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key)
{
	switch (key)
	{
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
		break;
	
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
		case '<':		
		case ',':		
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
		case '-':		
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;

		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;

		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}

