#include "Sd.hpp"

Sd::Sd(size_t bufferSize) : isFileOpen(false), maxBufferSize(bufferSize) {}

Sd::~Sd()
{
    // Ensure the file is closed and buffer is flushed upon object destruction
    flushBuffer();
    if (isFileOpen)
    {
        dataFile.close();
    }
}

bool Sd::init(int CS, String StartMsg, String prefix)
{
    bool success = true;

    // See if the card is present and can be initialized:
    if (!SD.begin(CS))
    {
        success = false;
    }
    else
    {
        DBG("Card initialized.");
        fileName = createUniqueLogFile(prefix);
        DBG("File name: " + fileName);
        dataFile = SD.open(fileName.c_str(), FILE_WRITE);
        if (dataFile)
        {
            dataFile.println(StartMsg);
            dataFile.flush();
            isFileOpen = true;
        }
        else
        {
            success = false;
        }
    }

    return success;
}

bool Sd::writeToBuffer(String dataString)
{
    buffer += dataString; // Add newline for each entry

    // Check if buffer size exceeds the maximum size
    if (buffer.length() >= maxBufferSize)
    {
        flushBuffer(); // Write to SD card if buffer is full
    }

    return true;
}

void Sd::flushBuffer()
{
    if (isFileOpen && buffer.length() > 0)
    {
        dataFile.print(buffer); // Write buffer content to file
        buffer = "";            // Clear the buffer
        dataFile.flush();       // Ensure data is written to the card
    }
}

String Sd::createUniqueLogFile(String prefix)
{
    String uniqueFileName;
    uint32_t currentLogIndex = 0;

    // Generate a unique file name
    do
    {
        uniqueFileName = "/" + String(prefix) + "_" + String(currentLogIndex++) + ".txt";
    } while (SD.exists(uniqueFileName.c_str())); // Check if the file already exists

    return uniqueFileName;
}

bool Sd::isInitialized()
{
    return isFileOpen;
}