/***********************************************************************

 OSCSender.h
 TSPSPeopleVision

 Copyright (c) 2009, LAB at Rockwell Group
 http://lab.rockwellgroup.com
 Created by Brett Renfer on 1/14/10.

 address: TSPS/person/ + ordered id (i.e. TSPS/person/0)

 argument 0: pid;
 argument 1: age;
 argument 2: centroid.x;
 argument 3: centroid.y;
 argument 4: velocity.x;
 argument 5: velocity.y;
 argument 6: boundingRect.x;
 argument 7: boundingRect.y;
 argument 8: boundingRect.width;
 argument 9: boundingRect.height;
 argument 10: opticalFlowVectorAccumulation.x;
 argument 11: opticalFlowVectorAccumulation.y;
 argument 12+ : contours (if enabled)


 ***********************************************************************/

#pragma once

#include "ofxOsc.h"
#include "Person.h"

namespace ofxTSPS {
    class OscSender : public ofxOscSender
    {
        public :

        //"old" variables are to check for reroute
        string	ip, oldip;
        int		port, oldport;

        OscSender();
        OscSender(string _ip, int _port);
        void setupSender(string _ip, int _port);
        void update();
        void personEntered ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );
        void personMoved ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );
        void personUpdated ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );
        void personWillLeave ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );

        void send ( ofxOscMessage m );
        void reroute(string _ip, int _port);

    };
};

