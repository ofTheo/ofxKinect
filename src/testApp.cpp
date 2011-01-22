#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup() {
	//kinect.init(true);  //shows infrared image
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
	
	drawPC = false;
}

//--------------------------------------------------------------
void testApp::update() {
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
void testApp::draw() {
	ofSetColor(255, 255, 255);
	if(drawPC){
		ofPushMatrix();
		ofTranslate(420, 320);
		// we need a proper camera class
		drawPointCloud();
		ofPopMatrix();
	}else{
		kinect.drawDepth(10, 10, 400, 300);
		kinect.draw(420, 10, 400, 300);

		grayImage.draw(10, 320, 400, 300);
		contourFinder.draw(10, 320, 400, 300);
	}
	

	ofSetColor(255, 255, 255);
	ofDrawBitmapString("accel is: " + ofToString(kinect.getMksAccel().x, 2) + " / " 
									+ ofToString(kinect.getMksAccel().y, 2) + " / "
									+ ofToString(kinect.getMksAccel().z, 2), 20, 658 );

	char reportStr[1024];
	sprintf(reportStr, "using opencv threshold = %i (press spacebar)\nset near threshold %i (press: + -)\nset far threshold %i (press: < >) num blobs found %i, fps: %f",bThreshWithOpenCV, nearThreshold, farThreshold, contourFinder.nBlobs, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 690);
	ofDrawBitmapString("tilt angle: " + ofToString(angle),20,670);
	ofDrawBitmapString("press p to switch between images and point cloud",20,680);
}

void testApp::drawPointCloud() {
	ofScale(400, 400, 400);
	int w = 640;
	int h = 480;
	ofRotateY(mouseX);
	float* distancePixels = kinect.getDistancePixels();
	glBegin(GL_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			ofPoint cur = kinect.getWorldCoordinateFor(x, y);
			ofColor color = kinect.getCalibratedColorAt(x,y);
			glColor3ub((unsigned char)color.r,(unsigned char)color.g,(unsigned char)color.b);
			glVertex3f(cur.x, cur.y, cur.z);
		}
	}
	glEnd();
}

//--------------------------------------------------------------
void testApp::exit(){
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
		break;
		case'p':
			drawPC = !drawPC;
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

