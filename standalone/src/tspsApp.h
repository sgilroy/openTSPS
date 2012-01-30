#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

/*********************************************************
    INCLUDES + DEFINES
*********************************************************/

    // TSPS core
    #include "ofxTSPS.h"

    #define _USE_LIVE_VIDEO         // comment out to load a movie file
    //#define USE_CUSTOM_GUI		// uncomment to add a "custom" panel to the gui
	
    // kinect support
    #include "ofxKinect.h"

    enum {
        CAMERA_NOT_INITED,
        CAMERA_KINECT,
        CAMERA_VIDEOGRABBER
    };

using namespace ofxTSPS;

/*********************************************************
    APP
*********************************************************/


class tspsApp : public ofBaseApp, public PersonListener {

	public:
		
		void setup();
		void update();
		void draw();
        void exit();
		
		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
        
        // TSPS events
    
		void personEntered( Person* newPerson, Scene* scene );
		void personMoved( Person* activePerson, Scene* scene );
		void personWillLeave( Person* leavingPerson, Scene* scene );
		void personUpdated( Person* updatedPerson, Scene* scene );
            
        #ifdef _USE_LIVE_VIDEO

        // ready for either live video or Kinect, will choose in the next step
        ofVideoGrabber 		vidGrabber;
        ofxKinect           kinect;

        // kinect or live video?
        bool bKinect, bKinectConnected;
        int cameraState;
        int tilt; //kinect only tilt var
    
		#else
		  ofVideoPlayer 		vidPlayer;
		#endif
	
        void initVideoInput();
        void closeVideoInput();
    
		int camWidth, camHeight;

		ofImage cameraImage;
    	
	//status bar stuff
		ofImage statusBar;
		int		drawStatus[3];
		ofImage personEnteredImage;
		ofImage personUpdatedImage;
		ofImage personLeftImage;
		ofTrueTypeFont timesBoldItalic;
	
	//other gui images
		ofImage background;


	PeopleTracker peopleTracker;
	    
    
    
};

#endif
