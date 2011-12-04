/***********************************************************************
 
 OSCSender.cpp
 TSPSPeopleVision

 Copyright (c) 2009, LAB at Rockwell Group
 http://lab.rockwellgroup.com
 Created by Brett Renfer on 1/14/10.
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 ***********************************************************************/

#include "OscSender.h"

namespace ofxTSPS {
    /***************************************************************
     CONSTRUCTOR + SETUP
     ***************************************************************/

    OscSender::OscSender(){};

    OscSender::OscSender(string _ip, int _port){
        setupSender(_ip, _port);
    };

    void OscSender::setupSender(string _ip, int _port){
        ip = oldip = _ip;
        port = oldport = _port;
        
        stringstream message;
        message<<"SETTING UP SENDER @ "<<ip<<":"<<port;
        ofLog(OF_LOG_VERBOSE, message.str());
        
        setup(ip, port);
    };

    /***************************************************************
     UPDATE
    ***************************************************************/

    void OscSender::update(){
        if (strcmp(oldip.c_str(), ip.c_str()) != 0 || oldport != port){
            oldip = ip;
            oldport = port;
            reroute(ip, port);				
        }
    }

    /***************************************************************
     SEND
    ***************************************************************/


    void OscSender::personEntered ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours ){
        ofxOscMessage m;
        m.setAddress("TSPS/personEntered/");
        m.addIntArg(p->pid);
        m.addIntArg(p->age);
        m.addFloatArg(centroid.x);
        m.addFloatArg(centroid.y);
        m.addFloatArg(p->velocity.x);
        m.addFloatArg(p->velocity.y);
        
        ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
        
        m.addFloatArg(boundingRect.x);
        m.addFloatArg(boundingRect.y);
        m.addFloatArg(boundingRect.width);
        m.addFloatArg(boundingRect.height);
        
        m.addFloatArg(p->opticalFlowVectorAccumulation.x);
        m.addFloatArg(p->opticalFlowVectorAccumulation.y);
        
        if (bSendContours){
            //any args after 9 will be contours
            for (int i=0; i<p->simpleContour.size(); i++){
                m.addFloatArg(p->simpleContour[i].x);
                m.addFloatArg(p->simpleContour[i].y);
            };
        }
        
        send(m);
    };

    void OscSender::personMoved ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours ){
        ofxOscMessage m;
        m.setAddress("TSPS/personMoved/");
        m.addIntArg(p->pid);
        m.addIntArg(p->age);
        m.addFloatArg(centroid.x);
        m.addFloatArg(centroid.y);
        m.addFloatArg(p->velocity.x);
        m.addFloatArg(p->velocity.y);
        
        ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
        
        m.addFloatArg(boundingRect.x);
        m.addFloatArg(boundingRect.y);
        m.addFloatArg(boundingRect.width);
        m.addFloatArg(boundingRect.height);
        
        m.addFloatArg(p->opticalFlowVectorAccumulation.x);
        m.addFloatArg(p->opticalFlowVectorAccumulation.y);
        
        if (bSendContours){
            //any args after 9 will be contours
            for (int i=0; i<p->simpleContour.size(); i++){
                m.addFloatArg(p->simpleContour[i].x);
                m.addFloatArg(p->simpleContour[i].y);
            };
        }
        
        send(m);
    };

    void OscSender::personUpdated ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours ){
        ofxOscMessage m;
        m.setAddress("TSPS/personUpdated/");
        m.addIntArg(p->pid);
        m.addIntArg(p->age);
        m.addFloatArg(centroid.x);
        m.addFloatArg(centroid.y);
        m.addFloatArg(p->velocity.x);
        m.addFloatArg(p->velocity.y);
        
        ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
        
        m.addFloatArg(boundingRect.x);
        m.addFloatArg(boundingRect.y);
        m.addFloatArg(boundingRect.width);
        m.addFloatArg(boundingRect.height);
        
        m.addFloatArg(p->opticalFlowVectorAccumulation.x);
        m.addFloatArg(p->opticalFlowVectorAccumulation.y);
        
        if (bSendContours){
            //any args after 9 will be contours
            for (int i=0; i<p->simpleContour.size(); i++){
                m.addFloatArg(p->simpleContour[i].x);
                m.addFloatArg(p->simpleContour[i].y);
            };
        }
        
        send(m);
    };

    void OscSender::personWillLeave ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours )
    {
        ofxOscBundle b;
        ofxOscMessage m;
        m.setAddress("TSPS/personWillLeave/");
        m.addIntArg(p->pid);
        m.addIntArg(p->age);
        m.addFloatArg(centroid.x);
        m.addFloatArg(centroid.y);
        m.addFloatArg(p->velocity.x);
        m.addFloatArg(p->velocity.y);
        
        ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
        
        m.addFloatArg(boundingRect.x);
        m.addFloatArg(boundingRect.y);
        m.addFloatArg(boundingRect.width);
        m.addFloatArg(boundingRect.height);
        
        m.addFloatArg(p->opticalFlowVectorAccumulation.x);
        m.addFloatArg(p->opticalFlowVectorAccumulation.y);
        
        if (bSendContours){
            //any args after 11 will be contours
            for (int i=0; i<p->simpleContour.size(); i++){
                m.addFloatArg(p->simpleContour[i].x);
                m.addFloatArg(p->simpleContour[i].y);
            };
        }
        
        send(m);	
    };

    /*
     
     BUNDLE SENDING // for when there's a good implementation of bundles in flash + processing
     
     void OscSender::personEntered ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours ){
         ofxOscBundle bundle;
         
         ofxOscMessage person;
         person.setAddress("TSPS/personEntered/");
         person.addIntArg(p->pid);
         person.addIntArg(p->age);
         person.addFloatArg(centroid.x);
         person.addFloatArg(centroid.y);
         person.addFloatArg(p->velocity.x);
         person.addFloatArg(p->velocity.y);
         ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
         person.addFloatArg(boundingRect.x);
         person.addFloatArg(boundingRect.y);
         person.addFloatArg(boundingRect.width);
         person.addFloatArg(boundingRect.height);
         
         bundle.addMessage(person);
         
         ofxOscMessage contours;
         contours.setAddress("TSPS/personEntered/contours/");
         if (bSendContours){
         //any args after 9 will be contours
         for (int i=0; i<p->simpleContour.size(); i++){
         contours.addFloatArg(p->simpleContour[i].x);
         contours.addFloatArg(p->simpleContour[i].y);
         };
         };
         bundle.addMessage(contours);
         
         ofxOscMessage opticalFlow;
         opticalFlow.setAddress("TSPS/personEntered/opticalFlow/");
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.x);
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.y);
         bundle.addMessage(opticalFlow);	
         
         sendBundle(bundle);
     };
     
     void OscSender::personMoved ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours ){
         ofxOscBundle bundle;
         
         ofxOscMessage person;
         person.setAddress("TSPS/personMoved/");
         person.addIntArg(p->pid);
         person.addIntArg(p->age);
         person.addFloatArg(centroid.x);
         person.addFloatArg(centroid.y);
         person.addFloatArg(p->velocity.x);
         person.addFloatArg(p->velocity.y);
         ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
         person.addFloatArg(boundingRect.x);
         person.addFloatArg(boundingRect.y);
         person.addFloatArg(boundingRect.width);
         person.addFloatArg(boundingRect.height);
         
         bundle.addMessage(person);
         
         ofxOscMessage contours;
         contours.setAddress("TSPS/personMoved/contours/");
         if (bSendContours){
         //any args after 9 will be contours
         for (int i=0; i<p->simpleContour.size(); i++){
         contours.addFloatArg(p->simpleContour[i].x);
         contours.addFloatArg(p->simpleContour[i].y);
         };
         };
         bundle.addMessage(contours);
         
         ofxOscMessage opticalFlow;
         opticalFlow.setAddress("TSPS/personMoved/opticalFlow/");
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.x);
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.y);
         bundle.addMessage(opticalFlow);	
         
         sendBundle(bundle);
     }
     
     
     void OscSender::personUpdated ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours ){
         ofxOscBundle bundle;
         
         ofxOscMessage person;
         person.setAddress("TSPS/personUpdated/");
         person.addIntArg(p->pid);
         person.addIntArg(p->age);
         person.addFloatArg(centroid.x);
         person.addFloatArg(centroid.y);
         person.addFloatArg(p->velocity.x);
         person.addFloatArg(p->velocity.y);
         ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
         person.addFloatArg(boundingRect.x);
         person.addFloatArg(boundingRect.y);
         person.addFloatArg(boundingRect.width);
         person.addFloatArg(boundingRect.height);
         
         bundle.addMessage(person);
         
         ofxOscMessage contours;
         contours.setAddress("TSPS/personUpdated/contours/");
         if (bSendContours){
         //any args after 9 will be contours
         for (int i=0; i<p->simpleContour.size(); i++){
         contours.addFloatArg(p->simpleContour[i].x);
         contours.addFloatArg(p->simpleContour[i].y);
         };
         };
         bundle.addMessage(contours);
         
         ofxOscMessage opticalFlow;
         opticalFlow.setAddress("TSPS/personUpdated/opticalFlow/");
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.x);
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.y);
         bundle.addMessage(opticalFlow);	
         
         sendBundle(bundle);
     };
     
     void OscSender::personWillLeave ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool bSendContours )
     {
         ofxOscBundle bundle;
         
         ofxOscMessage person;
         person.setAddress("TSPS/personWillLeave/");
         person.addIntArg(p->pid);
         person.addIntArg(p->age);
         person.addFloatArg(centroid.x);
         person.addFloatArg(centroid.y);
         person.addFloatArg(p->velocity.x);
         person.addFloatArg(p->velocity.y);
         ofRectangle boundingRect = p->getBoundingRectNormalized(cameraWidth,cameraHeight);
         person.addFloatArg(boundingRect.x);
         person.addFloatArg(boundingRect.y);
         person.addFloatArg(boundingRect.width);
         person.addFloatArg(boundingRect.height);
         
         bundle.addMessage(person);
         
         ofxOscMessage contours;
         contours.setAddress("TSPS/personWillLeave/contours/");
         if (bSendContours){
            //any args after 9 will be contours
            for (int i=0; i<p->simpleContour.size(); i++){
                contours.addFloatArg(p->simpleContour[i].x);
                contours.addFloatArg(p->simpleContour[i].y);
            };
         };
         bundle.addMessage(contours);
         
         ofxOscMessage opticalFlow;
         opticalFlow.setAddress("TSPS/personUpdated/opticalFlow/");
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.x);
         opticalFlow.addFloatArg(p->opticalFlowVectorAccumulation.y);
         bundle.addMessage(opticalFlow);	
         
         sendBundle(bundle);
     };
     
     
     */

    void OscSender::send ( ofxOscMessage m ){
        sendMessage(m);
    };

    /***************************************************************
     REROUTE
     ***************************************************************/	

    void OscSender::reroute(string _ip, int _port){
        stringstream portstream;
        portstream<<_port;
        ofLog(OF_LOG_VERBOSE, "OscSender: REROUTING TO "+ip+", "+portstream.str());
        ip = _ip;
        port = _port;
        setup(ip, port);
    };
};
