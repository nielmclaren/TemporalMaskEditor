#include "testApp.h"
#include "ofAppGlutWindow.h"

int main(){
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1300, 1000, OF_WINDOW);
	ofRunApp(new testApp());
}
