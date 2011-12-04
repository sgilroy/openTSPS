
#include "PeopleTracker.h"
#include "CPUImageFilter.h"

//scales down tracking images for improved performance
#define TRACKING_SCALE_FACTOR .5

//Fix for FMAX not in Visual Studio C++
#if defined _MSC_VER
#define fmax max
#define fmin min
#pragma warning (disable:4996)
#define snprintf sprintf_s
#endif

using namespace ofxCv;
using namespace cv;

namespace ofxTSPS {
    
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark Setup

    //---------------------------------------------------------------------------
    void PeopleTracker::setup(int w, int h)
    {	
        ofAddListener(ofEvents.mousePressed, this, &PeopleTracker::mousePressed);
        
        width  = w;
        height = h;
        
        cameraImage.allocate(width, height, OF_IMAGE_COLOR);
        warpedImage.allocate(width, height, OF_IMAGE_COLOR);
        backgroundImage.allocate(width, height, OF_IMAGE_COLOR);
        differencedImage.allocate(width, height, OF_IMAGE_COLOR);
        
        //setup contour finder
        contourFinder.setThreshold(15);
        contourFinder.getTracker().setMaximumAge(30);
        
        //setup background
        //backgroundDifferencer.setLearningTime(900);
        //backgroundDifferencer.setThresholdValue(10);
        
        //set up optical flow
        //opticalFlow.allocate( width*TRACKING_SCALE_FACTOR, height*TRACKING_SCALE_FACTOR );
        //opticalFlow.setCalcStep(5,5);
        //grayLastImage = graySmallImage;
        
        //set tracker
        bOscEnabled = bTuioEnabled = bTcpEnabled = bWebSocketsEnabled = false;
        p_Settings = Settings::getInstance();
        
        //gui.loadFromXML();	
        //gui.setDraw(true);		
        
        //setup gui quad in manager
        gui.setup();
        gui.setupQuadGui( width, height );
        gui.loadSettings("settings/settings.xml");
        activeHeight = ofGetHeight();
        activeWidth = ofGetWidth();
        
        activeViewIndex = 4;
        
        //setup view rectangles 
        
        cameraView.setup(width, height);
        adjustedView.setup(width, height);
        bgView.setup(width, height);
        processedView.setup(width, height);
        dataView.setup(width, height);
        
        updateViewRectangles();
        
        cameraView.setImage(cameraImage);
        cameraView.setTitle("Camera Source View", "Camera");
        cameraView.setColor(218,173,90);
        
        adjustedView.setImage(warpedImage);
        adjustedView.setTitle("Adjusted Camera View", "Adjusted");
        adjustedView.setColor(174,139,138);
        
        bgView.setImage(backgroundImage);
        bgView.setTitle("Background Reference View", "Background");
        bgView.setColor(213,105,68);
            
        processedView.setImage(differencedImage);
        processedView.setTitle("Differenced View", "Differencing");
        processedView.setColor(113,171,154);
        
        dataView.setTitle("Data View", "Data");
        dataView.setColor(191,120,0);
        
        setActiveView(PROCESSED_VIEW);
        
        //persistentTracker.setListener( this );
        //updateSettings();
        lastHaarFile = "";
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setHaarXMLFile(string haarFile)
    {
        haarFile = "haar/" + haarFile;
        
        //check if haar file has changed
        if(lastHaarFile != haarFile){
            ofLog(OF_LOG_VERBOSE, "changing haar file to " + haarFile);
            //haarFinder.setup(haarFile);
            //haarTracker.setup(&//haarFinder);
            lastHaarFile = haarFile;
        }
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark Setup Communication

    //---------------------------------------------------------------------------
    void PeopleTracker::setupTuio(string ip, int port)
    {
        ofLog(OF_LOG_VERBOSE, "SEND TUIO");
        bTuioEnabled = true;
        p_Settings->oscPort = port;
        p_Settings->oscHost = ip;
        tuioClient.setup(ip, port);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setupOsc(string ip, int port)
    {
        ofLog(OF_LOG_VERBOSE, "SEND OSC");
        bOscEnabled = true;
        p_Settings->oscPort = port;
        p_Settings->oscHost = ip;
        oscClient.setupSender(ip, port);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setupTcp(int port)
    {
        bTcpEnabled = true;
        ofLog(OF_LOG_VERBOSE, "SEND TCP TO PORT "+port);
        p_Settings->tcpPort = port;
        tcpClient.setup(port);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setupWebSocket( int port)
    {
        ofLog(OF_LOG_VERBOSE, "SEND VIA WEBSOCKETS AT PORT "+port);
        bWebSocketsEnabled = true;
        p_Settings->webSocketPort = port;
        webSocketServer.setup(port);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setListener(PersonListener* listener)
    {
        eventListener = listener;
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark Track People
    //---------------------------------------------------------------------------
    void PeopleTracker::update(ofImage _cameraImage)
    {
        //grayImage = image;
        //colorImage = image;
        cameraImage = _cameraImage;
        updateSettings();
        trackPeople();
    }

    /*
    //---------------------------------------------------------------------------
    void PeopleTracker::update(ofxCvGrayscaleImage image)
    {
        grayImage = image;
        
        updateSettings();
        trackPeople();
    }*/

    //---------------------------------------------------------------------------
    void PeopleTracker::updateSettings()
    {
        setHaarXMLFile(p_Settings->haarFile);

        //check to enable OSC
        if (p_Settings->bSendOsc && !bOscEnabled) setupOsc(p_Settings->oscHost, p_Settings->oscPort);
        else if (!p_Settings->bSendOsc) bOscEnabled = false;
        
        //check to enable TUIO
        if (p_Settings->bSendTuio && !bTuioEnabled) setupTuio(p_Settings->tuioHost, p_Settings->tuioPort);
        else if (!p_Settings->bSendTuio) bTuioEnabled = false;

        //check to enable TCP
        if (p_Settings->bSendTcp && !bTcpEnabled) setupTcp(p_Settings->tcpPort);
        else if (!p_Settings->bSendTcp) bTcpEnabled = false;
            
        //check to enable websockets
        if (p_Settings->bSendWebSockets && !bWebSocketsEnabled){
            setupWebSocket(p_Settings->webSocketPort);
        } else if (!p_Settings->bSendWebSockets){
            bWebSocketsEnabled = false;
            webSocketServer.close();
        }
        //switch camera view if new panel is selected
        if (p_Settings->currentPanel != p_Settings->lastCurrentPanel) setActiveView(p_Settings->currentPanel + 1);

        // Set the current view within the gui so the image can only be warped when in Camera View
        if (cameraView.isActive()) {
            gui.changeGuiCameraView(true);
        } else {
            gui.changeGuiCameraView(false);
        }
    }

    /**
     * Core Method
     * Run every frame to update
     * the system to the current location
     * of people
     */
    //---------------------------------------------------------------------------
    void PeopleTracker::trackPeople()
    {	
        //-------------------
        //QUAD WARPING
        //-------------------
            
        //warp background
        getQuadSubImage(&cameraImage, &warpedImage, &p_Settings->quadWarpScaled, OF_IMAGE_COLOR);
        
        //graySmallImage.scaleIntoMe(grayImageWarped);
        //grayBabyImage.scaleIntoMe(grayImageWarped);
        //grayDiff = grayImageWarped;
        
        //amplify (see cpuimagefilter class)
        if(p_Settings->bAmplify){
            //warpedImage.amplify(warpedImage, p_Settings->highpassAmp/15.0f);
        }
        
        //grayImageWarped = grayDiff;
        
        //-------------------
        //BACKGROUND
        //-------------------
            
        //learn background (either in reset or additive)
        if (p_Settings->bLearnBackground){
            cout << "Learning Background" << endl;
            //backgroundDifferencer.reset();
            //backgroundDifferencer.update(warpedImage, backgroundImage);
            copy( warpedImage, backgroundImage);
            backgroundImage.update();
            //grayBg = grayImageWarped;
        }
        
        //progressive relearn background
        // TO:DO
        /*if (p_Settings->bLearnBackgroundProgressive){
            if (p_Settings->bLearnBackground) floatBgImg = grayBg;
            floatBgImg.addWeighted( grayImageWarped, p_Settings->fLearnRate * .00001);
            grayBg = floatBgImg;
            //cvConvertScale( floatBgImg.getCvImage(), grayBg.getCvImage(), 255.0f/65535.0f, 0 );       
            //grayBg.flagImageChanged();			
        }*/
        
        // set threshold
        //backgroundDifferencer.setThresholdValue( p_Settings->threshold );
        contourFinder.setThreshold(p_Settings->threshold);
        
        if(p_Settings->trackType == TRACK_ABSOLUTE){
            //backgroundDifferencer.setDifferenceMode( RunningBackground::ABSOLUTE );
            //grayDiff.absDiff(grayBg, grayImageWarped);
            absdiff( backgroundImage, warpedImage, differencedImage);
        }
        else{
            //grayDiff = grayImageWarped;
            if(p_Settings->trackType == TRACK_LIGHT){
                //backgroundDifferencer.setDifferenceMode( RunningBackground::BRIGHTER );            
                //grayDiff = grayBg - grayImageWarped;
                //cvSub(grayBg.getCvImage(), grayDiff.getCvImage(), grayDiff.getCvImage());
                subtract( warpedImage, backgroundImage, differencedImage);
            }
            else if(p_Settings->trackType == TRACK_DARK){ 
                //backgroundDifferencer.setDifferenceMode( RunningBackground::DARKER );            
                //cvSub(grayDiff.getCvImage(), grayBg.getCvImage(), grayDiff.getCvImage());
                //grayDiff = grayImageWarped - grayBg;
                subtract ( backgroundImage, warpedImage, differencedImage);
            }
        }
        
        //backgroundDifferencer.update(differencedImage, backgroundImage);
        differencedImage.update();
        
        //-----------------------
        // IMAGE TREATMENT
        //-----------------------
        if(p_Settings->bSmooth){ // TO:DO
            //blur();
            //grayDiff.blur((p_Settings->smooth * 2) + 1); //needs to be an odd number
        }
        
        //highpass filter (see cpuimagefilter class)	
        if(p_Settings->bHighpass){ // TO:DO
            //grayDiff.highpass(p_Settings->highpassBlur, p_Settings->highpassNoise);
        }
        
        //threshold	
        //grayDiff.threshold(p_Settings->threshold);
        
        //-----------------------
        // TRACKING
        //-----------------------	
        //find the optical flow
        if (p_Settings->bTrackOpticalFlow){ // TO:DO
            //opticalFlow.calc(grayLastImage, graySmallImage, 11);
        }
        
        //accumulate and store all found haar features.
        /*
        vector<ofRectangle> haarRects;
        if(p_Settings->bDetectHaar){
            //haarTracker.findHaarObjects( grayBabyImage );
            float x, y, w, h;
            while(//haarTracker.hasNextHaarItem()){
                //haarTracker.getHaarItemPropertiesEased( &x, &y, &w, &h );
                haarRects.push_back( ofRectangle(x,y,w,h) );
            }
        }
        */
        
        char pringString[1024];
        //sprintf(pringString, "found %i haar items this frame", (int) haarRects.size());
        ofLog(OF_LOG_VERBOSE, pringString);
        
        //setup stuff for min + max size
        contourFinder.setMinArea( p_Settings->minBlob*width*height );
        contourFinder.setMaxArea( p_Settings->maxBlob*width*height );
        
        contourFinder.findContours( differencedImage );
        //persistentTracker.trackBlobs(contourFinder.blobs);
        
        // TO:DO!!!!
        //scene.averageMotion = opticalFlow.flowInRegion(0,0,width,height);
        //scene.percentCovered = 0; 
        
        // By setting maxVector and minVector outside the following for-loop, blobs do NOT have to be detected first
        //            before optical flow can begin working.
        if(p_Settings->bTrackOpticalFlow) {
            //opticalFlow.maxVector = p_Settings->maxOpticalFlow;
            //opticalFlow.minVector = p_Settings->minOpticalFlow;
        }
        
        RectTracker& tracker = contourFinder.getTracker();
        
        for(int i = 0; i < contourFinder.size(); i++){
            unsigned int id = contourFinder.getLabel(i);
            if(tracker.existsPrevious(id)) {
                Person* p = trackedPeople[id];
                //somehow we are not tracking this person, safeguard (shouldn't happen)
                if(NULL == p){
                    ofLog(OF_LOG_WARNING, "ofxPerson::warning. encountered persistent blob without a person behind them\n");
                    continue;
                }
                p->oid = i; //hack ;(
                
                scene.percentCovered += p->area;
                
                //update this person with new blob info
                p->update(p_Settings->bCentroidDampen);

                
                //normalize simple contour
                for (int i=0; i<p->simpleContour.size(); i++){
                    p->simpleContour[i].x /= width;
                    p->simpleContour[i].y /= height;
                }
                /*
                 TO:DO
                ofRectangle roi;
                roi.x		= fmax( (p->boundingRect.x - p_Settings->haarAreaPadding) * TRACKING_SCALE_FACTOR, 0.0f );
                roi.y		= fmax( (p->boundingRect.y - p_Settings->haarAreaPadding) * TRACKING_SCALE_FACTOR, 0.0f );
                roi.width	= fmin( (p->boundingRect.width  + p_Settings->haarAreaPadding*2) * TRACKING_SCALE_FACTOR, grayBabyImage.width - roi.x );
                roi.height	= fmin( (p->boundingRect.height + p_Settings->haarAreaPadding*2) * TRACKING_SCALE_FACTOR, grayBabyImage.width - roi.y );	
                */
                //sum optical flow for the person
                if(p_Settings->bTrackOpticalFlow){
                    //p->opticalFlowVectorAccumulation = opticalFlow.flowInRegion(roi);
                }
                
                //detect haar patterns (faces, eyes, etc)
                if (p_Settings->bDetectHaar){
                    bool bHaarItemSet = false;
                        
                    //find the region of interest, expanded by haarArea.
                    //bound by the frame edge
                    //cout << "ROI is " << roi.x << "  " << roi.y << " " << roi.width << "  " << roi.height << endl;
                    bool haarThisFrame = false;
                    /*for(int i = 0; i < haarRects.size(); i++){
                        ofRectangle hr = haarRects[i];
                        //check to see if the haar is contained within the bounding rectangle
                        if(hr.x > roi.x && hr.y > roi.y && hr.x+hr.width < roi.x+roi.width && hr.y+hr.height < roi.y+roi.height){
                            hr.x /= TRACKING_SCALE_FACTOR;
                            hr.y /= TRACKING_SCALE_FACTOR;
                            hr.width /= TRACKING_SCALE_FACTOR;
                            hr.height /= TRACKING_SCALE_FACTOR;
                            p->setHaarRect(hr);
                            haarThisFrame = true;
                            break;
                        }
                    }
                    if(!haarThisFrame){
                        p->noHaarThisFrame();
                    }*/
                }
                
                if(eventListener != NULL){
                    if( p->velocity.x != 0 || p->velocity.y != 0){
                        eventListener->personMoved(p, &scene);
                    }
                    eventListener->personUpdated(p, &scene);
                }
            } else {
                ofPoint centroid = toOf(contourFinder.getCentroid(i));
                blobOn( centroid.x, centroid.y, id, i );
            }
        }
        
        // delete old blobs
        
        map<unsigned int,Person*>::iterator it;    
        it = trackedPeople.begin();
        while(it != trackedPeople.end()){
            Person* p = (*it).second;
            if (p == NULL){
                // wtfz
                trackedPeople.erase(it++);
            /*} else if (p->age > 60){
                trackedPeople.erase(it++);
            */} else if ( !(tracker.existsPrevious((*it).first) && tracker.existsCurrent((*it).first)) && !tracker.existsCurrent((*it).first) ){
                trackedPeople.erase(it++);
            } else {
                ++it;
            }
        }
        
        //normalize it
        scene.percentCovered /= width*height;
        
        //-----------------------
        // VIEWS
        //-----------------------	
        
        //store the old image
        //grayLastImage = graySmallImage;
        
        //update views
        
        cameraView.update(cameraImage);
        if (p_Settings->bAdjustedViewInColor)
            adjustedView.update(warpedImage);
        else
            adjustedView.update(warpedImage);
        bgView.update(backgroundImage);
        processedView.update(differencedImage);
        
        //-----------------------
        // COMMUNICATION
        //-----------------------	

        if (trackedPeople.size() > 0){
            
            map<unsigned int,Person*>::iterator it;    
            it = trackedPeople.begin();
            while(it != trackedPeople.end()){
                Person* p = (*it).second;
                if (p == NULL){
                    // wtfz
                    ++it;
                    continue;
                }
            
                ofPoint centroid = p->getCentroidNormalized(width, height);
        //			if(p_Settings->bUseHaarAsCenter && p->hasHaarRect()){
                
                if (bTuioEnabled){
                    ofPoint tuioCursor = p->getCentroidNormalized(width, height);
                    tuioClient.cursorDragged( tuioCursor.x, tuioCursor.y, p->oid);
                }
                
                if (bOscEnabled){
                    if( p->velocity.x != 0 || p->velocity.y != 0){
                        oscClient.personMoved(p, centroid, width, height, p_Settings->bSendOscContours);
                    }
                    oscClient.personMoved(p, centroid, width, height, p_Settings->bSendOscContours);
                }
                
                if (bTcpEnabled){
                    tcpClient.personMoved(p, centroid, width, height, p_Settings->bSendOscContours);
                }
                
                if (bWebSocketsEnabled){
                    webSocketServer.personMoved(p, centroid, width, height, p_Settings->bSendOscContours);
                }
                
                ++it;
            }
        }
        
        if(bTuioEnabled){
            tuioClient.update();		
        }
        
        if (bOscEnabled){
            oscClient.ip = p_Settings->oscHost;
            oscClient.port = p_Settings->oscPort;
            oscClient.update();
        };
        
        if (bTcpEnabled){
            tcpClient.port = p_Settings->oscPort;
            tcpClient.update();
            tcpClient.send();
        }
        
        if (bWebSocketsEnabled){
            if (p_Settings->webSocketPort != webSocketServer.getPort()){
                webSocketServer.close();
                webSocketServer.setup( p_Settings->webSocketPort );
            }
            //sent automagically
            webSocketServer.send();
        }
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark Person Management
    //---------------------------------------------------------------------------
    void PeopleTracker::blobOn( int x, int y, int id, int index ){
        //ofxCvTrackedBlob blob = persistentTracker.getById( id );
        Person* newPerson = new Person(id, index, contourFinder);//order, blob);
        trackedPeople[id] = newPerson;
        cout<<"sweet! "<<id<<":"<<trackedPeople[id]<<endl;
        //trackedPeople.push_back( newPerson );
        if(eventListener != NULL){
            eventListener->personEntered(newPerson, &scene);
        }
        
        ofPoint centroid = newPerson->getCentroidNormalized(width, height);
        
        if(bTuioEnabled){
            tuioClient.cursorPressed(1.0*x/width, 1.0*y/height, id);
        }
        if(bOscEnabled){
            oscClient.personEntered(newPerson, centroid, width, height, p_Settings->bSendOscContours);
        }
        if(bTcpEnabled){
            tcpClient.personEntered(newPerson, centroid, width, height, p_Settings->bSendOscContours);
        }
        if(bWebSocketsEnabled){
            webSocketServer.personEntered(newPerson, centroid, width, height, p_Settings->bSendOscContours);
        }
        
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::blobMoved( int x, int y, int id, int order ){/*not used*/}

    //---------------------------------------------------------------------------
    void PeopleTracker::blobOff( int x, int y, int id, int order )
    {
        Person* p = getTrackedPerson(id);
        //ensure we are tracking
        if(NULL == p){
            ofLog(OF_LOG_WARNING, "ofxPerson::warning. encountered persistent blob without a person behind them\n");		
            return;
        }
        
        //alert the delegate
        if(eventListener != NULL){
            eventListener->personWillLeave(p, &scene);
        }
        
        ofPoint centroid = p->getCentroidNormalized(width, height);
        if (bTuioEnabled) {
            tuioClient.cursorReleased(centroid.x, centroid.y, order);	
        }
        //send osc kill message if enabled
        if (bOscEnabled){
            oscClient.personWillLeave(p, centroid, width, height, p_Settings->bSendOscContours);
        };
        
        //send tcp kill message if enabled
        if(bTcpEnabled){
            tcpClient.personWillLeave(p, centroid, width, height, p_Settings->bSendOscContours);
        }
        
        if(bWebSocketsEnabled){
            webSocketServer.personWillLeave(p, centroid, width, height, p_Settings->bSendOscContours);
        }
        
        //delete the object and remove it from the vector    
        //map<unsigned int,Person*>::iterator it;
        //it=mymap.find(id);
        trackedPeople.erase(id);
    }

    //---------------------------------------------------------------------------
    Person* PeopleTracker::getTrackedPerson( int pid )
    {    
    //    for( int i = 0; i < trackedPeople.size(); i++ ) {
    //        if( trackedPeople[i]->pid == pid ) {
    //            return trackedPeople[i];
    //        }
    //    }
        return trackedPeople[pid];
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark Draw
    //---------------------------------------------------------------------------
    void PeopleTracker::draw()
    {
        draw(0,0);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::draw(int x, int y)
    {
        draw(x,y,drawMode);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::draw(int x, int y, int mode)
    {
        // run lean + mean if we're minimized
        if (p_Settings->bMinimized) return;
        ofPushMatrix();
            ofTranslate(x, y, 0);
            // draw the incoming, the grayscale, the bg and the thresholded difference
            ofSetHexColor(0xffffff);
        
            //draw large image
            if (activeViewIndex ==  CAMERA_SOURCE_VIEW){
                cameraView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);		
                gui.drawQuadGui( activeView.x, activeView.y, activeView.width, activeView.height );
            } else if ( activeViewIndex == ADJUSTED_CAMERA_VIEW){
                adjustedView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);				
            } else if ( activeViewIndex == REFERENCE_BACKGROUND_VIEW){
                bgView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);			
            } else if ( activeViewIndex == PROCESSED_VIEW){ 
                processedView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);
            } else if ( activeViewIndex == DATA_VIEW ){
                ofPushMatrix();
                    ofTranslate(activeView.x, activeView.y);
                    drawBlobs(activeView.width, activeView.height);
                ofPopMatrix();
                dataView.drawLarge(activeView.x, activeView.y, activeView.width, activeView.height);
            }
            
            //draw all images small
            cameraView.draw();
            adjustedView.draw();
            bgView.draw();
            processedView.draw();
            dataView.draw();	
            
            ofPushMatrix();
                ofTranslate(dataView.x, dataView.y);
                drawBlobs(dataView.width, dataView.height);
            ofPopMatrix();
            
        ofPopMatrix();
        
        //draw framerate in a box
        
        char frmrate[1024];
        sprintf(frmrate, "Frame rate: %f", ofGetFrameRate() );
        
        ofPushStyle();
        ofFill();
        ofSetColor(196,182,142);
        ofRect(cameraView.x, cameraView.y + cameraView.height + spacing*3 + 8, cameraView.width*2 + spacing, spacing*4);
        ofPopStyle();
        
        if (!bFontLoaded) ofDrawBitmapString(frmrate, cameraView.x + 10, cameraView.y + 10 + cameraView.height + spacing*5);
        else font.drawString(frmrate, (int)cameraView.x + 10, (int) (cameraView.y + 10 + cameraView.height + spacing*5));
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::drawBlobs( float drawWidth, float drawHeight){
        
        float scaleVar = (float) drawWidth/width;
        
        ofFill();
        ofSetHexColor(0x333333);
        ofRect(0,0,drawWidth,drawHeight);
        ofSetHexColor(0xffffff);
        
        ofNoFill();
        
        if (p_Settings->bTrackOpticalFlow){
            ofSetColor(34,151,210);
            //opticalFlow.draw(drawWidth,drawHeight);
        }					
        
        ofPushMatrix();
        ofScale(scaleVar, scaleVar);
        
        // simpler way to draw contours: contourFinder.draw();
        if (trackedPeople.size() > 0){
            map<unsigned int,Person*>::iterator it;    
            for (it=trackedPeople.begin(); it != trackedPeople.end(); ++it){		
                //draw blobs				
                //if //haarFinder is looking at these blobs, draw the area it's looking at
                Person* p = (*it).second;
                
                if (p == NULL){
                    //:(
                    continue;
                }
                
                //draw contours 
                ofPushStyle();
                ofNoFill();
                if (p_Settings->bSendOscContours){
                    ofSetHexColor(0x3abb93);
                } else {
                    ofSetHexColor(0xc4b68e);
                }
                ofBeginShape();
                for( int j=0; j<p->contour.size(); j++ ) {
                    ofVertex( p->contour[j].x, p->contour[j].y );
                }
                ofEndShape();
                ofPopStyle();
                
                if(p_Settings->bTrackOpticalFlow){
                    //purple optical flow arrow
                    ofSetHexColor(0xff00ff);
                    //JG Doesn't really provide any helpful information since its so scattered
        //			ofLine(p->centroid.x, 
        //				   p->centroid.y, 
        //				   p->centroid.x + p->opticalFlowVectorAccumulation.x, 
        //				   p->centroid.y + p->opticalFlowVectorAccumulation.y);
                }
                
                ofSetHexColor(0xffffff);							
                if(p_Settings->bDetectHaar){
                    ofSetHexColor(0xee3523);
                    //draw haar search area expanded 
                    //limit to within data box so it's not confusing
                    /*ofRect(p->boundingRect.x - p_Settings->haarAreaPadding, 
                           p->boundingRect.y - p_Settings->haarAreaPadding, 
                           p->boundingRect.width  + p_Settings->haarAreaPadding*2, 
                           p->boundingRect.height + p_Settings->haarAreaPadding*2);*/
                        
                        ofRectangle haarRect = ofRectangle(p->boundingRect.x - p_Settings->haarAreaPadding, 
                                                           p->boundingRect.y - p_Settings->haarAreaPadding, 
                                                           p->boundingRect.width  + p_Settings->haarAreaPadding*2, 
                                                           p->boundingRect.height + p_Settings->haarAreaPadding*2);
                        if (haarRect.x < 0){
                            haarRect.width += haarRect.x;
                            haarRect.x = 0;					
                        }
                        if (haarRect.y < 0){
                            haarRect.height += haarRect.y;	
                            haarRect.y = 0;
                        }
                        if (haarRect.x + haarRect.width > width) haarRect.width = width-haarRect.x;
                        if (haarRect.y + haarRect.height > height) haarRect.height = height-haarRect.y;
                        ofRect(haarRect.x, haarRect.y, haarRect.width, haarRect.height);
                }
                
                if(p->hasHaarRect()){
                    //draw the haar rect
                    ofSetHexColor(0xee3523);
                    ofRect(p->getHaarRect().x, p->getHaarRect().y, p->getHaarRect().width, p->getHaarRect().height);
                    //haar-detected people get a red square
                    ofSetHexColor(0xfd5f4f);
                }
                else {
                    //no haar gets a yellow square
                    ofSetHexColor(0xeeda00);
                }
                
                //draw person
                ofRect(p->boundingRect.x, p->boundingRect.y, p->boundingRect.width, p->boundingRect.height);
                
                //draw centroid
                ofSetHexColor(0xff0000);
                ofCircle(p->centroid.x, p->centroid.y, 3);
                
                //draw id
                ofSetHexColor(0xffffff);
                char idstr[1024];
                sprintf(idstr, "pid: %d\noid: %d\nage: %d", p->pid, p->oid, p->age );
                ofDrawBitmapString(idstr, p->centroid.x+8, p->centroid.y);													
            }
        }
        ofPopMatrix();
        ofSetHexColor(0xffffff);				
        //ofDrawBitmapString("blobs and optical flow", 5, height - 5 );
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark mouse

    //---------------------------------------------------------------------------
    void PeopleTracker::mousePressed( ofMouseEventArgs &e )
    {
        if (isInsideRect(e.x, e.y, cameraView)){
            activeViewIndex = CAMERA_SOURCE_VIEW;
            cameraView.setActive();
            adjustedView.setActive(false);
            bgView.setActive(false);
            processedView.setActive(false);
            dataView.setActive(false);
        } else if (isInsideRect(e.x, e.y, adjustedView)){
            activeViewIndex = ADJUSTED_CAMERA_VIEW;
            adjustedView.setActive();
            cameraView.setActive(false);
            bgView.setActive(false);
            processedView.setActive(false);
            dataView.setActive(false);
        } else if (isInsideRect(e.x, e.y, bgView)){
            activeViewIndex = REFERENCE_BACKGROUND_VIEW;
            bgView.setActive();
            cameraView.setActive(false);
            adjustedView.setActive(false);
            processedView.setActive(false);
            dataView.setActive(false);
        } else if (isInsideRect(e.x, e.y, processedView)){
            activeViewIndex = PROCESSED_VIEW;
            processedView.setActive();
            cameraView.setActive(false);
            adjustedView.setActive(false);
            bgView.setActive(false);
            dataView.setActive(false);
        } else if (isInsideRect(e.x, e.y, dataView)){
            activeViewIndex = DATA_VIEW;
            dataView.setActive();
            cameraView.setActive(false);
            adjustedView.setActive(false);
            bgView.setActive(false);
            processedView.setActive(false);
        }
    }

    //---------------------------------------------------------------------------
    bool PeopleTracker::isInsideRect(float x, float y, ofRectangle rect){
        return ( x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height );
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark gui extension
    //---------------------------------------------------------------------------
    void PeopleTracker::addSlider(string name, int* value, int min, int max)
    {
        //forward to the gui manager
        gui.addSlider(name, value, min, max);
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::addSlider(string name, float* value, float min, float max)
    {
        gui.addSlider(name, value, min, max);	
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::addToggle(string name, bool* value)
    {
        gui.addToggle(name, value);	
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark accessors

    /**
     * simple public getter for external classes
     */
    //---------------------------------------------------------------------------
    Person* PeopleTracker::personAtIndex(int i)
    {
        return NULL;
        //TO:DO!!!!
        //return trackedPeople[i];
    }

    //---------------------------------------------------------------------------
    int PeopleTracker::totalPeople()
    {
        return trackedPeople.size();
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::enableHaarFeatures(bool doHaar)
    {
        p_Settings->bDetectHaar = doHaar;
    }

    void PeopleTracker::enableOpticalFlow(bool doOpticalFlow)
    {
        p_Settings->bTrackOpticalFlow = doOpticalFlow;
    }

    //---------------------------------------------------------------------------
    // for accessing the OSC sender whose parameters are adjusted in the GUI
    OscSender* PeopleTracker::getOSCsender() {
        return &oscClient;
    }

    //---------------------------------------------------------------------------
    WebSocketSender * PeopleTracker::getWebSocketServer(){
        return &webSocketServer;
    };


    //---------------------------------------------------------------------------
    bool PeopleTracker::useKinect(){
        return p_Settings->bUseKinect;
    };	


    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark background management
    //---------------------------------------------------------------------------
    void PeopleTracker::relearnBackground()
    {
        p_Settings->bLearnBackground = true;
    }

    //JG Disabled this feature
    //void PeopleTracker::enableBackgroundRelearnSmart(bool doSmartLearn)//auto-relearns if there are too many blobs in the scene
    //{
    //	p_Settings->bSmartLearnBackground = doSmartLearn;
    //}

    //---------------------------------------------------------------------------
    void PeopleTracker::enableBackgroundReleaernProgressive(bool doProgressive) //relearns over time using progessive frame averagering
    {
        p_Settings->bLearnBackgroundProgressive = doProgressive;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setRelearnRate(float relearnRate)
    {
        p_Settings->fLearnRate = relearnRate;
    }


    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark image control
    //---------------------------------------------------------------------------
    void PeopleTracker::setThreshold(float thresholdAmount)
    {
        p_Settings->threshold = thresholdAmount;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setMinBlobSize(float minBlobSize)
    {
        p_Settings->minBlob = minBlobSize; 
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setMaxBlobSize(float maxBlobSize)
    {
        p_Settings->maxBlob = maxBlobSize;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::enableSmooth(bool doSmooth)
    {
        p_Settings->bSmooth = doSmooth;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setSmoothAmount(int smoothAmount)
    {
        p_Settings->smooth = smoothAmount;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::enableHighpass(bool doHighpass)
    {
        p_Settings->bHighpass = doHighpass;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setHighpassBlurAmount(int highpassBlurAmount)
    {
        p_Settings->highpassBlur = highpassBlurAmount;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setHighpassNoiseAmount(int highpassNoiseAmount)
    {
        p_Settings->highpassNoise = highpassNoiseAmount;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::enableAmplify(bool doAmp)
    {
        p_Settings->bAmplify = doAmp;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setAmplifyAmount(int amplifyAmount)
    {
        p_Settings->highpassAmp = amplifyAmount;
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark filter controls
    //haar
    //---------------------------------------------------------------------------
    void PeopleTracker::setHaarExpandArea(float haarExpandAmount) //makes the haar rect +area bigger
    {
        p_Settings->haarAreaPadding = haarExpandAmount;
    }

    //JG 1/21/10 disabled this feature to simplify the interface
    //void PeopleTracker::setMinHaarArea(float minArea)
    //{
    //	p_Settings->minHaarArea = minArea;
    //}
    //void PeopleTracker::setMaxHaarArea(float maxArea)
    //{
    //	p_Settings->maxHaarArea = maxArea;
    //}

    //void PeopleTracker::useHaarAsCentroid(bool useHaarCenter)
    //{
    //	p_Settings->bUseHaarAsCenter = useHaarCenter;
    //}

    //blobs
    //---------------------------------------------------------------------------
    void PeopleTracker::enableFindHoles(bool findHoles)
    {
        p_Settings->bFindHoles = findHoles;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::trackDarkBlobs()
    {
        p_Settings->trackType = TRACK_DARK;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::trackLightBlobs()
    {
        p_Settings->trackType = TRACK_LIGHT;	
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setDrawMode(int mode)
    {
        drawMode = mode;
    }

    //---------------------------------------------------------------------------
    int PeopleTracker::getDrawMode() 
    {
        return drawMode;
    }

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark gui customization

    //---------------------------------------------------------------------------
    void PeopleTracker::setActiveView( int viewIndex ){
        int oldActiveView = activeViewIndex;
        activeViewIndex = viewIndex;
        
        if (activeViewIndex == CAMERA_SOURCE_VIEW){
            cameraView.setActive();
            adjustedView.setActive(false);
            bgView.setActive(false);
            processedView.setActive(false);
            dataView.setActive(false);
        } else if (activeViewIndex == ADJUSTED_CAMERA_VIEW){
            adjustedView.setActive();
            cameraView.setActive(false);
            bgView.setActive(false);
            processedView.setActive(false);
            dataView.setActive(false);
        } else if (activeViewIndex == REFERENCE_BACKGROUND_VIEW){
            bgView.setActive();
            cameraView.setActive(false);
            adjustedView.setActive(false);
            processedView.setActive(false);
            dataView.setActive(false);
        } else if (activeViewIndex == PROCESSED_VIEW){
            processedView.setActive();		cameraView.setActive(false);
            adjustedView.setActive(false);
            bgView.setActive(false);
            dataView.setActive(false);
        } else if (activeViewIndex == DATA_VIEW){
            dataView.setActive();
            cameraView.setActive(false);
            adjustedView.setActive(false);
            bgView.setActive(false);
            processedView.setActive(false);
        } else {
            activeViewIndex = oldActiveView;
        }
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setActiveDimensions ( int actWidth, int actHeight){
        activeWidth = actWidth;
        activeHeight = actHeight;
        updateViewRectangles();
    }

    //---------------------------------------------------------------------------
    bool PeopleTracker::loadFont( string fontName, int fontSize){
        bFontLoaded = font.loadFont(fontName, fontSize);
        if (bFontLoaded){
            cameraView.setFont(&font);
            adjustedView.setFont(&font);
            bgView.setFont(&font);
            processedView.setFont(&font);
            dataView.setFont(&font);
        }
        return bFontLoaded;
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::setVideoGrabber(ofBaseVideo* grabber, tspsInputType inputType)
    {
        p_Settings->setVideoGrabber( grabber, inputType );
        if (inputType == TSPS_INPUT_VIDEO){
            gui.enableElement( "open video settings" );
            gui.disableElement( "use kinect" );
        } else if (inputType == TSPS_INPUT_KINECT){
            gui.disableElement( "open video settings" );
            gui.enableElement( "use kinect" );
        }
    }

    //---------------------------------------------------------------------------
    void PeopleTracker::updateViewRectangles(){
        //build all rectangles for drawing views
        ofPoint smallView;
        smallView.x = (activeWidth - GUI_WIDTH - spacing*6)/5.f;
        smallView.y = (height*TRACKING_SCALE_FACTOR) * (smallView.x/(width*TRACKING_SCALE_FACTOR));
        
        activeView.x = GUI_WIDTH + spacing;
        activeView.y = spacing;
        activeView.width = (activeWidth - GUI_WIDTH - spacing*2);
        activeView.height = (height*TRACKING_SCALE_FACTOR)*activeView.width/(width*TRACKING_SCALE_FACTOR);
        
        cameraView.x = GUI_WIDTH + spacing;
        cameraView.y = activeView.y + activeView.height + spacing;
        cameraView.width = smallView.x;
        cameraView.height = smallView.y;
        
        adjustedView.x = cameraView.x + cameraView.width + spacing;
        adjustedView.y = cameraView.y;
        adjustedView.width = smallView.x;
        adjustedView.height = smallView.y;
        
        bgView.x = adjustedView.x + adjustedView.width + spacing;
        bgView.y = cameraView.y;
        bgView.width = smallView.x;
        bgView.height = smallView.y;
        
        processedView.x = bgView.x + bgView.width + spacing;
        processedView.y = cameraView.y;
        processedView.width = smallView.x;
        processedView.height = smallView.y;
        
        dataView.x = processedView.x + processedView.width + spacing;
        dataView.y = cameraView.y;
        dataView.width = smallView.x;
        dataView.height = smallView.y;	
        gui.drawQuadGui( activeView.x, activeView.y, activeView.width, activeView.height );
    }


    //---------------------------------------------------------------------------
    // for accessing Optical Flow within a specific region
    // TO:DO
    ofPoint PeopleTracker::getOpticalFlowInRegion(float x, float y, float w, float h) {
        return 0;//opticalFlow.flowInRegion(x,y,w,h);
    }


    //---------------------------------------------------------------------------
    // for accessing which view is the current view
    bool PeopleTracker::inCameraView() {
        return cameraView.isActive();
    }

    //---------------------------------------------------------------------------
    bool PeopleTracker::inBackgroundView() {
        return bgView.isActive();
    }

    //---------------------------------------------------------------------------
    bool PeopleTracker::inDifferencingView() {
        return processedView.isActive();
    }

    //---------------------------------------------------------------------------
    bool PeopleTracker::inDataView() {
        return dataView.isActive();
    }

    //---------------------------------------------------------------------------
    bool PeopleTracker::inAdjustedView() {
        return adjustedView.isActive();
    }


    // for getting a color version of the adjusted view image
    // NOTE:  only works if the adjusted view is currently in color
    //        (this parameter can be set in the GUI under the 'views' tab)
    ofImage PeopleTracker::getAdjustedImageInColor() {
        if (p_Settings->bAdjustedViewInColor)
            return adjustedView.getColorImage();
    }
}
