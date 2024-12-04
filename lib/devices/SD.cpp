#include "Sd.hpp"

Sd::Sd(size_t bufferSize) : isFileOpen(false), initialised(false), maxBufferSize(bufferSize) {}

Sd::~Sd()
{
    // Ensure the file is closed and buffer is flushed upon object destruction
    flushBuffer();
    if (isFileOpen)
    {
        dataFile.close();
    }
}

bool Sd::createFile(String StartMsg, String prefix)
{
    bool success = false;

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

    return success;
}

bool Sd::init(int CS)
{
    if (!initialised)
    {
        // See if the card is present and can be initialized:
        if (!SD.begin(CS))
        {
            DBG("Card failed, or not present");
            initialised = false;
        }
        else
        {
            DBG("Card initialized.");
            initialised = true;
        }
    }
    return initialised;
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

bool Sd::loadGainsFromFile(const char *filename, gainScheduleData &gainSchedule)
{
    bool gainsLoaded = false;

    File file = SD.open(filename, FILE_READ);
    if (!file)
    {
        DBG("Failed to open file: " + String(filename));
    }

    gainSchedule.height = 0;
    char line[64]; // Buffer to hold each line (adjust size as needed)
    while (file.available() && gainSchedule.height < MAX_GAIN_ROWS)
    {
        // Read a line from the file
        size_t len = file.readBytesUntil('\n', line, sizeof(line) - 1);
        line[len] = '\0'; // Null-terminate the string

        // Skip empty lines
        if (len == 0)
        {
            continue;
        }

        // Parse the line into four float values
        char *ptr = line;
        for (uint8_t col = 0; col < gainScheduleData::width; ++col)
        {
            // Skip leading whitespace or commas
            while (*ptr == ' ' || *ptr == '\t' || *ptr == ',')
            {
                ++ptr;
            }

            if (*ptr == '\0')
            {
                DBG("Incomplete data in line");
                break;
            }

            // Parse the float value
            char *endPtr;
            float value = strtof(ptr, &endPtr);

            if (ptr == endPtr)
            {
                Serial.print("Failed to parse float in line: ");
                DBG(line);
                break;
            }

            // Store the value
            gainSchedule.data[gainSchedule.height][col] = value;

            // Move the pointer to the character after the comma
            ptr = endPtr;

            // Skip the comma if it's there
            if (*ptr == ',')
            {
                ++ptr;
            }
        }

        // Increment the row count
        ++gainSchedule.height;
    }

    if (gainSchedule.height > 0)
    {
        gainsLoaded = true;
    }

    file.close();

    return gainsLoaded;
}