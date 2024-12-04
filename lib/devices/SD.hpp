#ifndef SD_HPP
#define SD_HPP

#include <SD.h>
#include <SPI.h>
#include "Arduino.h"
#include "Debug.hpp"

class Sd
{
public:
    Sd(size_t bufferSize = 512);
    ~Sd();

    bool init(int CS, String StartMsg, String prefix);

    bool writeToBuffer(String dataString);
    void flushBuffer();
    bool isInitialized();
    bool checkDevice();

    String createUniqueLogFile(String prefix);

private:
    File dataFile;
    String fileName;
    bool isFileOpen;
    String buffer;
    size_t maxBufferSize;
};

#endif