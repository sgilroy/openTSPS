//
//  WebSocketSender.h
//  openTSPS
//
//  Created by brenfer on 10/4/11.
//  Copyright 2011 Rockwell Group. All rights reserved.
//

#include "ofxWebSocket.h"
#include "Person.h"

namespace ofxTSPS {
    class WebSocketMessage
    {
        public:
            WebSocketMessage( string message){ msg = message; };
            string msg;
    };

    class WebSocketSender : public ofxWebSocketProtocol
    {
        public:
            WebSocketSender();
            
            void setup( int port=7777 );  
            void close();
            void send();
                  
            int getPort();
        
            void personEntered ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );	
            void personMoved ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );	
            void personUpdated ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );	
            void personWillLeave ( Person * p, ofPoint centroid, int cameraWidth, int cameraHeight, bool sendContours = false );
        
        protected:
            vector<WebSocketMessage> toSend;
            bool bSocketOpened;
        
            int port;
        
            ofxWebSocketReactor * reactor;
            vector<ofxWebSocketConnection *> sockets;
            
            void onopen(ofxWebSocketEvent& args);
            void onclose(ofxWebSocketEvent& args);
            void onmessage(ofxWebSocketEvent& args);
    };
};