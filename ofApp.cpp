/************************************************************
************************************************************/
#include "ofApp.h"

#include <time.h>

/* for dir search */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <dirent.h>
#include <string>

using namespace std;

/* */


/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp()
: d_INTERVAL_MIN(6.0)
, d_INTERVAL_MAX(30.0)
, t_LastSound(0)
, id(0)
, Osc("127.0.0.1", 12348, 12349)
, State(STATE_PLAY)
, vol_max(1.0)
, now(0)
{
	srand((unsigned int)time(NULL));
	
	font.load("font/RictyDiminished-Regular.ttf", 12, true, true, true);
	
	vol_down_speed	= (vol_max - 0) / 1.5; // 1.5secで
	vol_up_speed	= (vol_max - 0) / 1.5;
}

/******************************
******************************/
ofApp::~ofApp()
{
	/********************
	********************/
	for(int i = 0; i < Sounds.size(); i++){
		delete Sounds[i];
	}
	
	for(int i = 0; i < SoundFileNames.size(); i++){
		delete SoundFileNames[i];
	}
}

/******************************
******************************/
void ofApp::exit()
{
	printf("Good-bye\n");
}

//--------------------------------------------------------------
void ofApp::setup(){
	/********************
	********************/
	ofSetBackgroundAuto(true);
	
	ofSetWindowTitle("Kinobi:AmbientSound");
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetWindowShape(WINDOW_WIDTH, WINDOW_HEIGHT);
	ofSetEscapeQuitsApp(false);
	
	ofEnableAlphaBlending();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	// ofEnableSmoothing();
	
	/********************
	********************/
	makeup_music_table();
	load_music_table();
	
	prep_NextSoundInfo();
	
	load_and_start_backMusic();
}

/******************************
******************************/
void ofApp::makeup_music_table()
{
	/********************
	********************/
	const string dirname = "../../../data";
	
	DIR *pDir;
	struct dirent *pEnt;
	struct stat wStat;
	string wPathName;

	pDir = opendir( dirname.c_str() );
	if ( NULL == pDir ) { ERROR_MSG(); std::exit(1); }
	
	pEnt = readdir( pDir );
	while ( pEnt ) {
		// .と..は処理しない
		if ( strcmp( pEnt->d_name, "." ) && strcmp( pEnt->d_name, ".." ) ) {
		
			wPathName = dirname + "/" + pEnt->d_name;
			
			// ファイルの情報を取得
			if ( stat( wPathName.c_str(), &wStat ) ) {
				printf( "Failed to get stat %s \n", wPathName.c_str() );
				break;
			}
			
			if ( S_ISDIR( wStat.st_mode ) ) {
				// nothing.
			} else {
			
				vector<string> str = ofSplitString(pEnt->d_name, ".");
				if(str[str.size()-1] == "mp3"){
					// string str_NewFileName = wPathName;
					// string str_NewFileName = pEnt->d_name;
					string* str_NewFileName = new string(pEnt->d_name);
					
					SoundFileNames.push_back(str_NewFileName);
				}
			}
		}
		
		pEnt = readdir( pDir ); // 次のファイルを検索する
	}

	closedir( pDir );
	
	/********************
	********************/
	if(SoundFileNames.size() == 0){
		ERROR_MSG();
		std::exit(1);
	}
 }

/******************************
******************************/
void ofApp::load_music_table()
{
	/********************
	********************/
	if(SoundFileNames.size() <= 0)	return;

	/********************
	********************/
	printf("> load Sound Files\n");
	Sounds.resize(SoundFileNames.size());
	for(int i = 0; i < Sounds.size(); i++){
		Sounds[i] = new ofSoundPlayer;
		
		Sounds[i]->loadSound(SoundFileNames[i]->c_str());
		Sounds[i]->setLoop(false);
		Sounds[i]->setMultiPlay(true);
		Sounds[i]->setVolume(vol_max);
		
		printf("%3d:%s\n", i, SoundFileNames[i]->c_str());
	}
	printf("--------------------\n");
}

/******************************
******************************/
void ofApp::load_and_start_backMusic()
{
	/********************
	load時間短縮のため、mp3->wav としておくこと : ffmpeg
		https://qiita.com/suzutsuki0220/items/43c87488b4684d3d15f6
		> ffmpeg -i "input.mp3" -vn -ac 2 -ar 44100 -acodec pcm_s16le -f wav "output.wav"
	********************/
	backSound.loadSound("BackMusic/ServerRoom.wav");
	if(!backSound.isLoaded()) { ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	backSound.setLoop(true);
	backSound.setMultiPlay(true);
	backSound.setVolume(vol_max);
	
	backSound.play();
}

/******************************
******************************/
void ofApp::prep_NextSoundInfo()
{
	d_interval = ofRandom(d_INTERVAL_MIN, d_INTERVAL_MAX);
	
	int LastId = id;
	id = int ( ((double)rand() / ((double)RAND_MAX + 1)) * Sounds.size() );
	if(Sounds.size() <= id){
		WARNING_MSG();
		id = 0;
	}
	if(id == LastId){
		id++;
		if(Sounds.size() <= id) id = 0;
	}
	
	t_LastSound = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofApp::update(){
	/********************
	********************/
	if(Sounds.size() <= 0)	return;
	
	/********************
	********************/
	now = ofGetElapsedTimef();
	
	ofSoundUpdate();
	
	/********************
	********************/
	while(Osc.OscReceive.hasWaitingMessages()){
		ofxOscMessage m_receive;
		Osc.OscReceive.getNextMessage(&m_receive);
		
		if(m_receive.getAddress() == "/Quit"){
			int dummy = m_receive.getArgAsInt32(0);
			ofExit();
			return;
			
		}else if(m_receive.getAddress() == "/Play"){
			int dummy = m_receive.getArgAsInt32(0);
			play();
			
		}else if(m_receive.getAddress() == "/Stop"){
			int dummy = m_receive.getArgAsInt32(0);
			stop();
			
		}
	}
	
	/********************
	********************/
	StateChart();
	
	/********************
	********************/
	t_LastUpdate = now;
}

/******************************
******************************/
void ofApp::StateChart(){
	switch(State){
		case STATE_STOP:
			vol_down();
			if(backSound.getVolume() <= 0){
				if(backSound.isPlaying()) backSound.stop();
			}
			break;
			
		case STATE_PLAY:
			vol_up();
			
			if(d_interval < now - t_LastSound){
				if(Sounds[id]->isLoaded()) Sounds[id]->play();
				prep_NextSoundInfo();
			}
			break;
	}
}
	
/******************************
******************************/
void ofApp::vol_down(){
	float LastVol = backSound.getVolume();
	float vol = LastVol;
	
	vol -= vol_down_speed * (now - t_LastUpdate);
	if(vol < 0) vol = 0;
	if(vol_max < vol) vol = vol_max;
	
	if(vol != LastVol) backSound.setVolume(vol);
}

/******************************
******************************/
void ofApp::vol_up(){
	float LastVol = backSound.getVolume();
	float vol = LastVol;
	
	vol += vol_up_speed * (now - t_LastUpdate);
	if(vol < 0) vol = 0;
	if(vol_max < vol) vol = vol_max;
	
	if(vol != LastVol) backSound.setVolume(vol);
}

/******************************
******************************/
void ofApp::play(){
	switch(State){
		case STATE_STOP:
			if(!backSound.isPlaying()) backSound.play();
			backSound.setPaused(false);
			prep_NextSoundInfo();
			break;
			
		case STATE_PLAY:
			break;
	}
	
	State = STATE_PLAY;
}
	
/******************************
******************************/
void ofApp::stop(){
	State = STATE_STOP;
}
	
//--------------------------------------------------------------
void ofApp::draw(){
	/********************
	********************/
	ofBackground(30);
	ofSetColor(255);
	
	/********************
	********************/
	const int point_x = 10;
	int point_y = 20;
	const int ofs_y = 20;
	
	char buf[BUF_SIZE];
	if(State == STATE_PLAY)	sprintf(buf, "> NextId=%3d: %7.2f [sec]", id, d_interval - (now - t_LastSound));
	else					sprintf(buf, "> -----");
	// point_x = ofGetWidth()/2 - font.stringWidth(buf)/2;
	
	// font.drawStringAsShapes(buf, WIDTH__GRAPH_TIMEBASED - 1 - font[FONT_S].stringWidth(buf) - 10, -y); // y posはマイナス
	font.drawString(buf, point_x, point_y);
	point_y += ofs_y;
	
	PrintToBuf_musicTime(buf);
	font.drawString(buf, point_x, point_y);
	point_y += ofs_y;
	
	if(State == STATE_STOP)			sprintf(buf, "> STATE_STOP");
	else if(State == STATE_PLAY)	sprintf(buf, "> STATE_PLAY");
	else							sprintf(buf, "> STATE_xxx");
	font.drawString(buf, point_x, point_y);
	point_y += ofs_y;
	
	sprintf(buf, "> vol = %4.2f", backSound.getVolume());
	font.drawString(buf, point_x, point_y);
	point_y += ofs_y;
}

/******************************
******************************/
void ofApp::PrintToBuf_musicTime(char* buf)
{
	int pos_ms = backSound.getPositionMS();
	int min	= pos_ms / 1000 / 60;
	int sec	= pos_ms / 1000 - min * 60;
	int ms	= pos_ms % 1000;
	
	sprintf(buf, "> %5d:%5d:%5d (%7.2f)", min, sec, ms, backSound.getPosition());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			id = key - '0';
			if(id < Sounds.size()){
				Sounds[id]->play();
			}
			break;
			
		case 'k':
			backSound.setPosition(0.98);
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
