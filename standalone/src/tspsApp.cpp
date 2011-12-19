#include "tspsApp.h"

using namespace ofxTSPS;

class TSPSPersonAttributes {
public:
	TSPSPersonAttributes(){
		height = 0;
	}

	float height;
};

//--------------------------------------------------------------
void tspsApp::setup(){
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	ofBackground(223, 212, 190);
	
	camWidth = 320;
	camHeight = 240;
    
    // allocate images + setup people tracker
	
	peopleTracker.setup(camWidth, camHeight);
	peopleTracker.loadFont("fonts/times.ttf", 10);
	peopleTracker.setListener( this );
    
    bKinect         = false;
    cameraState     = CAMERA_NOT_INITED;
    
	#ifdef _USE_LIVE_VIDEO
    
    // are there any kinects out there?
    kinect.init();
    bKinectConnected = (kinect.numAvailableDevices() >= 1);
    
    // no kinects connected, let's just try to set up the device
    bKinectConnected = (kinect.numAvailableDevices() >= 1);
    
    if (kinect.numAvailableDevices() < 1 || !peopleTracker.useKinect()){
        kinect.clear();        
        bKinect = false;
        initVideoInput();
    } else {
        bKinect = true;
        initVideoInput();
    }
    
    #else
    vidPlayer.loadMovie("testmovie/twoPeopleStand.mov");
    vidPlayer.play();
    camWidth = vidPlayer.width;
    camHeight = vidPlayer.height;
	#endif

	
	peopleTracker.setActiveDimensions( ofGetWidth(), ofGetHeight()-68 );

	//load GUI / interface images

	personEnteredImage.loadImage("graphic/triggers/PersonEntered_Active.png");
	personUpdatedImage.loadImage("graphic/triggers/PersonUpdated_Active.png");
	personLeftImage.loadImage("graphic/triggers/PersonLeft_Active.png");
	statusBar.loadImage("graphic/bottomBar.png");
	background.loadImage("graphic/background.png");
	timesBoldItalic.loadFont("fonts/timesbi.ttf", 16);
    
	drawStatus[0] = 0;
	drawStatus[1] = 0;
	drawStatus[2] = 0;
}

//--------------------------------------------------------------
void tspsApp::update(){
    if (peopleTracker.useKinect() && !bKinect){
        bKinect = true;
        initVideoInput();
    } else if (!peopleTracker.useKinect() && bKinect){
        bKinect = false;
        initVideoInput();
    }
    
    bool bNewFrame = false;

	#ifdef _USE_LIVE_VIDEO
    if ( cameraState != CAMERA_NOT_INITED){
        if ( cameraState == CAMERA_KINECT ){
            kinect.update();
            bNewFrame = true;//kinect.isFrameNew();
        } else {
            vidGrabber.grabFrame();
            bNewFrame = vidGrabber.isFrameNew();
        }
    }    
    #else
    vidPlayer.update();
    bNewFrame = vidPlayer.isFrameNew();
	#endif
    
	if (bNewFrame){        
        #ifdef _USE_LIVE_VIDEO
        if ( cameraState == CAMERA_KINECT ){
			cameraImage.setFromPixels(kinect.getDepthPixels(), camWidth,camHeight, OF_IMAGE_GRAYSCALE);
        } else {
			cameraImage.setFromPixels(vidGrabber.getPixelsRef());
			cameraImage.setImageType(OF_IMAGE_GRAYSCALE);
        }
#else
        cameraImage.setFromPixels(vidPlayer.getPixelsRef());
		cameraImage.setImageType(OF_IMAGE_GRAYSCALE);
#endif
        peopleTracker.update(cameraImage);
        
		//iterate through the people
		for(int i = 0; i < peopleTracker.totalPeople(); i++){
			Person* p = peopleTracker.personAtIndex(i);
            if (cameraState == CAMERA_KINECT) p->depth = kinect.getDistanceAt( p->centroid );
		}
	}
}

//delegate methods for people entering and exiting
void tspsApp::personEntered( Person* newPerson, Scene* scene )
{
	newPerson->customAttributes = new TSPSPersonAttributes();

	//do something with them
	ofLog(OF_LOG_VERBOSE, "person %d of size %f entered!\n", newPerson->pid, newPerson->area);
	drawStatus[0] = 10;
}

void tspsApp::personMoved( Person* activePerson, Scene* scene )
{

	//do something with the moving person
	ofLog(OF_LOG_VERBOSE, "person %d of moved to (%f,%f)!\n", activePerson->pid, activePerson->boundingRect.x, activePerson->boundingRect.y);
	drawStatus[1] = 10;
}

void tspsApp::personWillLeave( Person* leavingPerson, Scene* scene )
{
	//do something to clean up
	ofLog(OF_LOG_VERBOSE, "person %d left after being %d frames in the system\n", leavingPerson->pid, leavingPerson->age);
	drawStatus[2] = 10;
}

void tspsApp::personUpdated( Person* updatedPerson, Scene* scene )
{
	TSPSPersonAttributes* attrbs = (TSPSPersonAttributes*)updatedPerson->customAttributes;

	ofLog(OF_LOG_VERBOSE, "updated %d person\n", updatedPerson->pid);
	drawStatus[1] = 10;
}

//--------------------------------------------------------------
void tspsApp::draw(){
	ofEnableAlphaBlending();
	ofSetHexColor(0xffffff);
	ofPushStyle();
	background.draw(0,0);
	peopleTracker.draw();

	ofPopStyle();

	//draw status bar stuff

	statusBar.draw(0,700);//ofGetHeight()-statusBar.height);
	if (drawStatus[0] > 0){
		drawStatus[0]--;
		personEnteredImage.draw(397,728);
	}
	if (drawStatus[1] > 0){
		drawStatus[1]--;
		personUpdatedImage.draw(533,728);
	}
	if (drawStatus[2] > 0){
		drawStatus[2]--;
		personLeftImage.draw(666,728);
	}

	ofSetColor(0, 169, 157);
	char numPeople[1024];
	sprintf(numPeople, "%i", peopleTracker.totalPeople());
    
	timesBoldItalic.drawString(numPeople,350,740);
}


//--------------------------------------------------------------
void tspsApp::exit(){
    if ( !cameraState == CAMERA_KINECT){  
    }
}

//--------------------------------------------------------------
void tspsApp::keyPressed  (int key){

	switch (key){
		case ' ':{
			peopleTracker.relearnBackground();
		} break;
		case 'f':{
			ofToggleFullscreen();
		} break;
	}
}

//--------------------------------------------------------------
void tspsApp::initVideoInput(){
    
#ifdef _USE_LIVE_VIDEO
    if ( bKinect && !cameraState == CAMERA_KINECT ){
        kinect.init();
        bKinectConnected = (kinect.numAvailableDevices() >= 1);
        if (!bKinectConnected){
            bKinect = false;
            return;
        }
        
        if ( cameraState == CAMERA_VIDEOGRABBER ){
            vidGrabber.close();
            cameraState = CAMERA_NOT_INITED;
        }
        
        if ( !cameraState == CAMERA_KINECT){            
            kinect.init();
            bool bOpened = kinect.open();
            if (bOpened){
                cameraState = CAMERA_KINECT;
                //set this so we can access video settings through the interface
                peopleTracker.setVideoGrabber(&kinect, TSPS_INPUT_KINECT);
            }
        }        
    } else {      
        if ( cameraState == CAMERA_NOT_INITED || cameraState == CAMERA_KINECT){
            
            if ( cameraState == CAMERA_KINECT ){
                kinect.close();
                kinect.clear();
                cameraState = CAMERA_NOT_INITED;
            }
            
            vidGrabber.setVerbose(false);
            vidGrabber.videoSettings();
            bool bAvailable = vidGrabber.initGrabber(camWidth,camHeight);
            if (bAvailable){ 
                cameraState = CAMERA_VIDEOGRABBER;
                //set this so we can access video settings through the interface
                peopleTracker.setVideoGrabber(&vidGrabber, TSPS_INPUT_VIDEO);
            }
        }
    }
#endif
    
};

//--------------------------------------------------------------
void tspsApp::closeVideoInput(){
#ifdef _USE_LIVE_VIDEO
    if ( cameraState == CAMERA_KINECT ){
        kinect.close();
        kinect.clear();
        cameraState = CAMERA_NOT_INITED;
    } else if ( cameraState == CAMERA_VIDEOGRABBER ){
        vidGrabber.close();
        cameraState = CAMERA_NOT_INITED;
    }
#endif
}

//--------------------------------------------------------------
void tspsApp::mouseMoved(int x, int y ){}

//--------------------------------------------------------------
void tspsApp::mouseDragged(int x, int y, int button){}

//--------------------------------------------------------------
void tspsApp::mousePressed(int x, int y, int button){}

//--------------------------------------------------------------
void tspsApp::mouseReleased(int x, int y, int button){}

//--------------------------------------------------------------
void tspsApp::windowResized(int w, int h){}
