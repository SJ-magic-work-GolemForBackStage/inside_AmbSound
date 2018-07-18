/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"

#include "sjCommon.h"
#include "ofxOsc_BiDirection.h"

/************************************************************
************************************************************/

class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		WINDOW_WIDTH	= 300,
		WINDOW_HEIGHT	= 150,
	};
	
	enum STATE{
		STATE_STOP,
		STATE_PLAY,
	};
	
	/****************************************
	****************************************/
	/********************
	********************/
	STATE State;
	const float vol_max;
	float vol_down_speed;
	float vol_up_speed;
	
	/********************
	********************/
	vector<ofSoundPlayer*> Sounds;
	vector<string*> SoundFileNames;
	ofSoundPlayer backSound;
	
	int id;
	
	const float d_INTERVAL_MIN;
	const float d_INTERVAL_MAX;
	float d_interval;
	
	float t_LastSound;
	
	/********************
	********************/
	ofTrueTypeFont font;
	float now;
	float t_LastUpdate;
	
	/********************
	********************/
	OSC_TARGET Osc;
	
	/****************************************
	****************************************/
	void load_and_start_backMusic();
	void makeup_music_table();
	void load_music_table();
	void prep_NextSoundInfo();
	void PrintToBuf_musicTime(char* buf);
	void StateChart();
	void vol_down();
	void vol_up();
	void play();
	void stop();
	
public:
	/****************************************
	****************************************/
	ofApp();
	~ofApp();
	void exit();

	void setup();
	void update();
	void draw();
	

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};
