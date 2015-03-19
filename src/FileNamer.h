#pragma once

#include "ofMain.h"

class FileNamer {
public:
  FileNamer() {
    prefix = "export";
    extension = "";
    currIndex = getFirstIndex();
  }

  FileNamer(string _prefix, string _extension) {
    setup(_prefix, _extension);
  }

  void setup(string _prefix, string _extension) {
    prefix = _prefix;
    extension = _extension;
    currIndex = getFirstIndex();
  }

  string curr() {
    return getFilename(currIndex);
  }

  string next() {
    ofFile file;
    while (file.doesFileExist(getFilename(currIndex)) && currIndex < 1000) {
      currIndex++;
    }
    return getFilename(currIndex);
  }

private:
  string prefix;
  string extension;
  int currIndex;

  int getFirstIndex() {
    ofFile file;
    int i = 1000;
    while (!file.doesFileExist(getFilename(i)) && i > 0) {
      i--;
    }
    return i;
  }

  string getFilename(int n) {
    string s = prefix + ofToString(n, 4, '0');
    if (extension == "") return s;
    if (extension == "/") return s + extension;
    return s + "." + extension;
  }
};

