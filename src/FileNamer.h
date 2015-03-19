#pragma once

#include "ofMain.h"

class FileNamer {
public:
  FileNamer() {
    initialized = false;
  }

  void setup(string _prefix, string _extension) {
    currIndex = 0;
    prefix = _prefix;
    extension = _extension;
    initialized = true;
  }

  string curr() {
    if (!verifyInitialized()) return;
    return getFilename(currIndex);
  }

  string next() {
    if (!verifyInitialized()) return;

    ofFile file;
    while (file.doesFileExist(getFilename(currIndex)) && currIndex < 1000) {
      currIndex++;
    }
    return getFilename(currIndex);
  }

private:
  bool initialized;
  int currIndex;
  string prefix;
  string extension;

  bool verifyInitialized() {
    if (!initialized) {
      cout << "Not initialized. Call setup() first." << endl;
    }
    return initialized;
  }

  string getFilename(int n) {
    string s = prefix + ofToString(n, 4, '0');
    if (extension == "") return s;
    if (extension == "/") return s + extension;
    return s + "." + extension;
  }
};

