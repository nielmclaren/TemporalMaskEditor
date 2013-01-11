#include "testApp.h"
#include "ofAppGlutWindow.h"

int main(){
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1940, 1616, OF_WINDOW);
	ofRunApp(new testApp());
}
