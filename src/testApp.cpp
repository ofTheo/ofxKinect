#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup()
{
	kinect.init();
	kinect.setVerbose(true);
	kinect.open();
    
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayBg.allocate(kinect.width, kinect.height);
	grayDiff.allocate(kinect.width, kinect.height);

	bLearnBakground = true;
	threshold = 80;

	ofSetFrameRate(30);
}

//--------------------------------------------------------------
void testApp::update()
{
	ofBackground(100, 100, 100);
	kinect.update();
}

//--------------------------------------------------------------
void testApp::draw()
{
	kinect.drawRgb(10, 10, 400, 300);
	kinect.draw(450, 10, 400, 300);
}

//--------------------------------------------------------------
void testApp::exit()
{
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key)
{
	switch (key)
	{
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
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

