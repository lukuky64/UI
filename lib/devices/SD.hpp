#ifndef SD_HPP
#define SD_HPP

#include <SD.h>
#include <SPI.h>
#include "Arduino.h"
#include "Debug.hpp"
#include "gainScheduleData.h"

class Sd
{
public:
    Sd(size_t bufferSize = 512);
    ~Sd();

    bool init(int CS);
    bool createFile(String StartMsg, String prefix);

    bool writeToBuffer(String dataString);
    void flushBuffer();
    bool isInitialized();
    bool checkDevice();
    bool loadGainsFromFile(const char *filename, gainScheduleData &gainSchedule);

    String createUniqueLogFile(String prefix);
    bool createNestedDirectories(String prefix);

private:
    File dataFile;
    String fileName;
    bool isFileOpen;
    String buffer;
    size_t maxBufferSize;
    bool initialised;
};

#endif