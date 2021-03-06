Compiling: Mac OS X
------------
*note:* the TSPS standalone app is Mac-only for now as we work on porting to OF 007. Please feel free to take a 
crack at the standalone app for other platforms!

1. clone openframeworks from here: [https://github.com/openframeworks/openFrameworks](https://github.com/openframeworks/openFrameworks )
2. clone the TSPS directory into openframeworks/
3. open the XCode project in openTSPS/standalone/basic
4. compile and run!

Release Notes
------------
1.2 (1/31/12 - Mac Only)

* Updated OSC Message Sending format
* Added leading slash 
* Removed "PersonMoved" message, no only PersonUpdated is sent on every frame
* Added a "Use Legacy OSC" button to revert to old format for backwards compatability

1.1 (11/18/11 - Mac Only)

* WebSocket communication
* Kinect support
* Minimize: run TSPS in lean, mean, mini mode (tiny window, all on-screen drawing skipped)
* OF 007 compatible

Troubleshooting
------------
* If you have trouble compiling, experience crashes, or need any general help, please visit the [TSPS Forum] (http://opentsps.com/forum)

WebSockets
------------

* To turn on WebSocket communication (on by default in 1.1), select the Communication panel in the GUI and edit the WebSockets panel at the very bottom.
* To test your WebSocket, simply compile/run openTSPS, then navigate your browser to localhost:PORT, where PORT is the port that is specified in the GUI. By default it's localhost:7681.
* To have extra fun, open that same URL in a bunch of browser windows.
* To have the most fun, open up another computer to http://YOUR_IP_ADDRESS:PORT
* The html/js/css that is served is in bin/data/web. It's very barebones... Hack away at it, please.
* Making your own JS TSPS clients is easy (and they don't have to live in the bin/data/web directory); the TSPS WebSocket sender sends JSON, so you don't even have to use our JS files if you don't want to.
