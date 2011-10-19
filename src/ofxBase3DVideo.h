#pragma once

/// \class ofxBase3DVideo
///
/// a base class for 3D video devices
class ofxBase3DVideo: public ofBaseVideo {

    public:
        
        /// get the pixels of the most recent depth frame
        virtual unsigned char* getDepthPixels()=0;
        
        /// get the distance in millimeters to a given point as a float array
        virtual float* getDistancePixels()=0;
        
        /// get the grayscale depth texture
        virtual ofTexture& getDepthTextureReference()=0;
};
