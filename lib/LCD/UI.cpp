#include <Arduino.h>
#include "UI.h"
#include "image_data.h"
#include "Debug.hpp"

UI::UI()
{
}

void UI::begin()
{
    uint16_t ID = tft.readID();
    if (ID == 0xD3D3)
        ID = 0x9486; // write-only shield
    tft.begin(ID);
    tft.setRotation(LANDSCAPE); // PORTRAIT
    tft.fillScreen(BLACK);

    state = START;
}

void UI::command()
{
    switch (state)
    {
    case START:
        startPage();
        break;
    case SETTINGS:
        settingsPage();
        break;
    case CALIBRATE:
        calibrationPage();
        break;
    case CREATE:
        createPage();
        break;
    case UPLOAD:
        uploadPage();
        break;
    case POINT:
        pointPage();
        break;
    case RUN:
        runPage();
        break;
    case MOTOR:
        motorPage();
        break;
    case FILTERING:
        filteringPage();
        break;
    default:
        startPage();
        break;
    }
}

bool UI::Touch_getXY()
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT); // restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH); // because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed)
    {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

bool UI::checkButton(Adafruit_GFX_Button &btn, bool down)
{
    bool isPressed = down && btn.contains(pixel_x, pixel_y);
    btn.press(isPressed);

    if (btn.justReleased())
    {
        btn.drawButton();
    }

    bool justPressed = btn.justPressed();

    if (justPressed)
    {
        btn.drawButton(true);
        delay(touch_delay);
    }

    return justPressed;
}

// ************************************************** PAGES **************************************************

// ************************ START ************************

void UI::startPage()
{
    bool devicesStatus = controller.initDevices();
    Adafruit_GFX_Button create_btn, upload_btn, settings_btn;

    create_btn.initButton(&tft, 160, 150, 200, 100, WHITE, WHITE, BLACK, (char *)"CREATE", 3);
    upload_btn.initButton(&tft, 160, 330, 200, 100, WHITE, WHITE, BLACK, (char *)"UPLOAD", 3);
    settings_btn.initButton(&tft, 300, 20, 40, 40, BLACK, RED, BLACK, (char *)"*", 3);

    create_btn.drawButton(false);
    upload_btn.drawButton(false);
    settings_btn.drawButton(false);

    bool loop = true;

    while (loop)
    {
        if (!devicesStatus)
        {
            showError(true, "Devices not found");
            delay(250);
            showError(false);
            delay(250);

            devicesStatus = controller.initDevices();

            if (devicesStatus)
            {
                showError(false);
            }
        }

        bool down = Touch_getXY();

        if (checkButton(create_btn, down))
        {
            state = CREATE;
            loop = false;
        }
        if (checkButton(upload_btn, down))
        {
            loop = false;
        }
        if (checkButton(settings_btn, down))
        {
            state = SETTINGS;
            loop = false;
        }
    }
    DBG("EXITING START PAGE");

    // clear page before going to the next
    tft.fillScreen(BLACK);
}

// ************************ SETTINGS ************************

void UI::settingsPage()
{
    Adafruit_GFX_Button back_btn, calibrate_btn, change_filter_btn;
    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    calibrate_btn.initButton(&tft, 160, 150, 200, 100, WHITE, WHITE, BLACK, (char *)"CALIBRATE", 3);
    change_filter_btn.initButton(&tft, 160, 280, 200, 100, WHITE, WHITE, BLACK, (char *)"FILTER", 3);

    back_btn.drawButton(false);
    calibrate_btn.drawButton(false);
    change_filter_btn.drawButton(false);

    bool loop = true;

    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(back_btn, down))
        {

            state = START;
            loop = false;
        }

        if (checkButton(calibrate_btn, down))
        {
            state = CALIBRATE;
            loop = false;
        }

        if (checkButton(change_filter_btn, down))
        {
            state = FILTERING;
            loop = false;
        }
    }

    DBG("EXITING SETTINGS PAGE");

    // clear page before going to the next
    tft.fillScreen(BLACK);
}

void UI::drawRectWithText(int16_t yPos, int16_t width, uint16_t colour, String text)
{
    width = constrain(width, 0, SCREEN_WIDTH); // Ensure width is within screen boundaries
    int16_t xPos = (SCREEN_WIDTH - width) / 2;

    uint8_t fontSize = 2;
    uint8_t horizPadding = 10;
    uint8_t vertPadding = 10;
    uint8_t charWidth = 6 * fontSize;  // font 1 = 6x8
    uint8_t charHeight = 8 * fontSize; // font 1 = 6x8
    uint8_t charsPerLine = (uint8_t)floor((float)(width - (2 * horizPadding)) / (float)(charWidth));

    uint8_t numLines = (uint8_t)ceil((float)text.length() / (float)charsPerLine);
    uint8_t rectHeight = vertPadding + ((charHeight + vertPadding) * numLines);

    String currentLineStr = "";
    uint8_t currentLineNum = 1;
    String currentWord = "";
    uint8_t numWords = 0;

    tft.fillRect(xPos, yPos, width, rectHeight, colour);
    tft.setTextColor(BLACK);   // Set text color to white
    tft.setTextSize(fontSize); // Set text size (adjust as needed).

    text += " "; // Add a space at the end to ensure the last word is processed

    for (size_t i = 0; i < text.length(); ++i)
    {
        if (text[i] == ' ')
        {
            numWords++;
            bool exceedLineWidth = (((currentLineStr.length() + currentWord.length()) + 1) > charsPerLine);
            bool endOfText = ((i + 1) == text.length());

            if (endOfText || exceedLineWidth)
            {
                if (endOfText)
                {
                    if (numWords > 1)
                    {
                        currentLineStr += " ";
                    }

                    currentLineStr += currentWord;
                }

                uint16_t textWidth = currentLineStr.length() * charWidth;
                uint16_t fontXPos = (width - textWidth) / 2;
                tft.setCursor(xPos + fontXPos, (yPos + ((currentLineNum - 1) * (charHeight + vertPadding)) + vertPadding)); // Position the text (adjust x and y coordinates as needed)
                tft.print(currentLineStr);                                                                                  // Display the text

                currentLineStr = currentWord;
                currentWord = "";
                currentLineNum++;
            }
            else
            {
                if (numWords > 1)
                {
                    currentLineStr += " ";
                }
                currentLineStr += currentWord;
                currentWord = "";
            }
        }
        else
        {
            currentWord += text[i];
        }
    }
}

void UI::progressBar(String text, float progress, int16_t yPos)
{
    float progressWidth = SCREEN_WIDTH * progress;

    tft.fillRect(progressWidth, yPos, SCREEN_WIDTH - progressWidth, 40, WHITE);
    tft.fillRect(0, yPos, progressWidth, 40, PURPLE_2);

    tft.setTextColor(BLACK);      // Set text color to white
    tft.setTextSize(2);           // Set text size (adjust as needed)
    tft.setCursor(10, yPos + 15); // Position the text (adjust x and y coordinates as needed)
    tft.print(text);              // Display the text
}

// ************************ CALIBRATION ************************

void UI::calibrationPage()
{
    Adafruit_GFX_Button back_btn, begin_btn, stop_btn;
    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    begin_btn.initButton(&tft, 160, 380, 200, 100, WHITE, WHITE, BLACK, (char *)"BEGIN", 3);
    stop_btn.initButton(&tft, 160, 380, 200, 100, WHITE, WHITE, BLACK, (char *)"STOP", 3);
    back_btn.drawButton(false);
    begin_btn.drawButton(false);

    drawRectWithText(60, SCREEN_WIDTH, PURPLE_4, "1. Step input to 5km");
    drawRectWithText(105, SCREEN_WIDTH, PURPLE_4, "2. Leaking to 0km");

    progressBar("", 0, 260);

    bool loop = true;

    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(back_btn, down))
        {
            state = SETTINGS;

            loop = false;
        }

        bool eStop = false;

        if (checkButton(begin_btn, down) && !eStop)
        {
            float pressureSetPoint = ROCKET_SIM::altitudeToPressure(3000);                // 3km setpoint
            bool calibrateInitialised = controller.initCalibrateSystem(pressureSetPoint); // setpoint for calibration

            stop_btn.drawButton(true);

            if (calibrateInitialised)
            {
                DBG("Calibration initialised");

                controller.startCalibrateSystem();

                // delay to get some atmospheric data

                uint8_t numIterations = 50;

                for (int i = 1; i <= numIterations; i++)
                {
                    if (controller.calibrateIterate())
                    {
                        progressBar("Loading...", ((float)i / (float)numIterations), 260);
                        delay(20);
                    }
                    else
                    {
                        calibrateInitialised = false;
                        break;
                    }
                }

                stop_btn.drawButton(false);

                // main calibration loop
                while (controller.calibrateIterate())
                {
                    down = Touch_getXY();

                    if (checkButton(stop_btn, down))
                    {
                        controller.stop();
                        controller.setCalibrationProgress(0);
                        loop = false;
                        eStop = true;
                        calibrateInitialised = false;

                        break;
                    }

                    float progress = controller.getCalibrationProgress();

                    bool currentState = (progress < 0.5);
                    static bool lastState = !currentState;

                    if (currentState != lastState)
                    {
                        lastState = currentState;

                        if (progress < 0.5)
                        {
                            drawRectWithText(60, SCREEN_WIDTH, PURPLE_3, "1. Step input to 5km");
                            drawRectWithText(105, SCREEN_WIDTH, PURPLE_4, "2. Leaking to 0km");
                        }
                        else
                        {
                            drawRectWithText(105, SCREEN_WIDTH, PURPLE_3, "2. Leaking to 0km");
                            drawRectWithText(60, SCREEN_WIDTH, PURPLE_4, "1. Step input to 5km");
                        }
                    }

                    progressBar("Calibrating...", progress, 260);
                }

                progressBar("", 0, 260);
                drawRectWithText(105, SCREEN_WIDTH, PURPLE_4, "2. Leaking to 0km");
                drawRectWithText(60, SCREEN_WIDTH, PURPLE_4, "1. Step input to 5km");
            }
            else
            {
                DBG("Calibration not initialised");
                showError(true, "Calibration not init");
                delay(500);
                showError(false);
            }
        }
    }

    DBG("EXITING CALIBRATION PAGE");

    // clear page before going to the next
    tft.fillScreen(BLACK);
}

// ************************ CREATE ************************

void UI::createPage()
{
    Adafruit_GFX_Button point_btn, motor_btn, back_btn;

    tft.drawRGBBitmap(0, 0, create_page, 320, 480);

    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    point_btn.initButton(&tft, 80, 455, 160, 50, BLACK, ORANGE, BLACK, (char *)"POINT", 2);
    motor_btn.initButton(&tft, 240, 455, 160, 50, BLACK, ORANGE, BLACK, (char *)"MOTOR", 2);

    back_btn.drawButton(false);
    point_btn.drawButton(false);

    // disabling this page for now
    motor_btn.drawButton(true);

    bool loop = true;
    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(back_btn, down))
        {
            state = START;

            loop = false;
        }

        if (checkButton(point_btn, down))
        {
            // tft.fillRect(40, 80, 160, 80, GREEN);
            state = POINT;
            loop = false;
        }
        // if (checkButton(motor_btn, down))
        // {
        //     state = MOTOR;
        //     loop = false;
        // }
    }
    DBG("EXITING CREATE PAGE");
    // clear page before going to the next
    tft.fillScreen(BLACK);
}

// ************************ UPLOAD ************************

void UI::uploadPage()
{
    Adafruit_GFX_Button back_btn;

    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    back_btn.drawButton(false);

    bool loop = true;

    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(back_btn, down))
        {
            state = START;

            loop = false;
        }
    }

    DBG("EXITING UPLOAD PAGE");

    // clear page before going to the next
    tft.fillScreen(BLACK);
}

// ************************ POINT ************************

void UI::pointPage()
{
    Adafruit_GFX_Button back_btn, save_btn;

    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    save_btn.initButton(&tft, 160, 430, 300, 50, BLACK, ORANGE, BLACK, (char *)"SAVE", 2);
    back_btn.drawButton(false);
    save_btn.drawButton(false);

    // Draw initial graph and sliders
    drawGraph(sliderApogee.sliderValue, sliderBurnTime.sliderValue);
    drawSlider(sliderApogee);
    drawSlider(sliderBurnTime);
    // drawSliders();

    bool loop = true;

    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(back_btn, down))
        {
            state = CREATE;

            loop = false;
        }

        if (checkButton(save_btn, down))
        {
            // tft.fillRect(40, 80, 160, 80, GREEN);

            state = RUN;
            loop = false;
        }

        bool slider1Touched = handleSliderTouch(sliderApogee, down);
        bool slider2Touched = handleSliderTouch(sliderBurnTime, down);

        if (slider1Touched || slider2Touched)
        {
            DBG("Slider touched");
            drawGraph(sliderApogee.sliderValue, sliderBurnTime.sliderValue);
        }
    }

    DBG("EXITING POINT PAGE");
    tft.fillScreen(BLACK);
}

void UI::drawGraph(float apogee, float burnout_time)
{
    // Clear the previous graph area
    tft.fillRect(0, GRAPH_TOP, SCREEN_WIDTH, GRAPH_HEIGHT, BLACK);

    // Run simulation
    ROCKET_SIM sim(burnout_time, apogee, -10.0f);
    data = sim.runSimulation();

    // Define scaling factors to fit the data within the screen
    float total_time = data.time[data.num_points - 1];
    float x_scale = float(SCREEN_WIDTH - 1) / total_time; // Scale x-axis based on total time
    float y_scale = float(GRAPH_HEIGHT - 1) / apogee;     // Scale y-axis based on max altitude

    // Initialize previous coordinates for line drawing
    int prev_x = 0;
    int prev_y = GRAPH_TOP + GRAPH_HEIGHT - 1 - int(data.altitude[0] * y_scale);

    // Loop through data points to plot the graph
    for (int i = 1; i < data.num_points; i++)
    {
        // Calculate screen coordinates
        int x = int(data.time[i] * x_scale);
        int y = GRAPH_TOP + GRAPH_HEIGHT - 1 - int(data.altitude[i] * y_scale);

        // Constrain x and y to screen boundaries
        x = constrain(x, 0, SCREEN_WIDTH - 1);
        y = constrain(y, GRAPH_TOP, GRAPH_TOP + GRAPH_HEIGHT - 1);

        // Draw line from previous point to the current point
        tft.drawLine(prev_x, prev_y, x, y, WHITE);

        // Update previous points
        prev_x = x;
        prev_y = y;
    }

    // Display apogee information
    updateTextBox("Apogee=" + String(int(data.apogee)) + "m @T=" + String(data.time_at_apogee) + "s");

    delay(50);
}

float UI::mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void UI::updateTextBox(String text)
{
    // Clear the text area where you want to update the apogee value
    tft.fillRect(20, 170, 280, 22, WHITE); // Adjust width/height as needed to clear text area
    tft.setCursor(25, 175);
    tft.setTextSize(2);
    tft.print(text);
}

void UI::drawSlider(sliderObj &slider)
{
    slider.sliderValue = constrain(slider.sliderValue, slider.minSliderValue, slider.maxSliderValue);
    tft.fillRect(20, slider.yPos, SLIDER_WIDTH, SLIDER_HEIGHT, WHITE);
    tft.fillRect(20, slider.yPos, mapFloat(slider.sliderValue, 0, slider.maxSliderValue, 25, SLIDER_WIDTH), SLIDER_HEIGHT, ORANGE);
    tft.setTextSize(2);
    tft.setCursor(50, slider.yPos + 20);
    tft.print(slider.text);
}

// Function to handle slider touch
bool UI::handleSliderTouch(sliderObj &slider, bool down)
{
    if (down)
    {
        // Check if touch is within the bounds of slider 1
        if (pixel_y > slider.yPos && pixel_y < slider.yPos + SLIDER_HEIGHT)
        {
            slider.sliderValue = mapFloat(pixel_x, 10, 10 + SLIDER_WIDTH, slider.minSliderValue, slider.maxSliderValue);
            drawSlider(slider);
            delay(50);
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

// ************************ RUN ************************

void UI::runPage()
{
    GRAPH_HEIGHT *= 2;

    tft.fillRect(0, GRAPH_TOP, SCREEN_WIDTH, GRAPH_HEIGHT, BLACK);

    Adafruit_GFX_Button back_btn, start_btn, stop_btn;
    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    start_btn.initButton(&tft, 160, 380, 280, 50, BLACK, ORANGE, BLACK, (char *)"START", 2);
    stop_btn.initButton(&tft, 160, 430, 280, 50, BLACK, RED, BLACK, (char *)"STOP", 2);

    back_btn.drawButton(false);
    start_btn.drawButton(false);

    stop_btn.drawButton(true);

    bool loop = (data.num_points > 0);

    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(start_btn, down))
        {
            // check devices are still connected
            bool devicesStatus = controller.initDevices();

            if (!devicesStatus)
            {
                showError(true, "Devices not found");
                delay(500);
                showError(false);

                loop = false;
                break;
            }

            int prev_x = 0;

            float maxVal = data.apogee * 1.1; // 10% more than max incase we overshoot

            float y_scale = float(GRAPH_HEIGHT - 1) / maxVal; // Scale y-axis based on max
            float x_scale = float(SCREEN_WIDTH - 1) / data.time[data.num_points - 1];
            int prev_real_y = GRAPH_TOP + GRAPH_HEIGHT - 1;
            int prev_target_y = GRAPH_TOP + GRAPH_HEIGHT - 1;

            controller.setAlpha(sliderFilter.sliderValue);
            bool initialisedController = controller.initData(data); // will have a delay for calibrating the sensor

            if (initialisedController)
            {
                DBG("Controller initialised");

                controller.initPID();

                bool run = controller.run();

                while (run)
                {
                    // RUN SIMULATION
                    bool iterated = controller.iterate();

                    if (iterated)
                    {
                        // loop time is about 550us or 1.8KHz

                        float time = controller.getLatestTime();

                        float real_pressure = controller.getLatestPressure();
                        float target_pressure = controller.getLatestSetpoint();

                        // DBG(real_pressure);

                        float real_y = ROCKET_SIM::pressureToAltitude(real_pressure);     // convert to altitude
                        float target_y = ROCKET_SIM::pressureToAltitude(target_pressure); // convert to altitude

                        // DBG(real_y);

                        real_y = GRAPH_TOP + GRAPH_HEIGHT - 1 - int(real_y * y_scale);
                        target_y = GRAPH_TOP + GRAPH_HEIGHT - 1 - int(target_y * y_scale);

                        int x = constrain(int(time * x_scale), 0, SCREEN_WIDTH - 1);

                        real_y = constrain(real_y, GRAPH_TOP, GRAPH_TOP + GRAPH_HEIGHT - 1);
                        target_y = constrain(target_y, GRAPH_TOP, GRAPH_TOP + GRAPH_HEIGHT - 1);

                        // update graph with live data
                        tft.drawLine(prev_x, prev_real_y, x, real_y, WHITE);   // draw line for real_y
                        tft.drawLine(prev_x, prev_target_y, x, target_y, RED); // draw line for target_y

                        prev_x = x;
                        prev_real_y = real_y;
                        prev_target_y = target_y;
                    }
                    else
                    {
                        run = false;
                    }

                    // Check if the stop button is pressed
                    bool down = Touch_getXY();

                    if (checkButton(stop_btn, down))
                    {
                        state = START;
                        loop = false;
                        run = false;

                        controller.stop();

                        // Reset sliders
                        sliderApogee.sliderValue = 1000;
                        sliderBurnTime.sliderValue = 1;
                    }
                }
            }
            else
            {
                DBG("Controller not initialised");
                showError(true, "Controller cannot init");
                delay(500);
                showError(false);
            }
        }

        if (checkButton(back_btn, down))
        {
            state = START;

            loop = false;
        }
    }

    DBG("EXITING RUN PAGE");
    // clear page before going to the next
    tft.fillScreen(BLACK);

    GRAPH_HEIGHT /= 2;
}

// ************************ MOTOR ************************

void UI::filteringPage()
{
    Adafruit_GFX_Button back_btn, change_filter_btn;
    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    change_filter_btn.initButton(&tft, 160, 150, 200, 100, WHITE, WHITE, BLACK, (char *)"SET", 3);

    back_btn.drawButton(false);
    change_filter_btn.drawButton(false);

    sliderFilter.sliderValue = controller.getAlpha();

    drawSlider(sliderFilter);
    drawRectWithText(360, 100, ORANGE, String(sliderFilter.sliderValue));

    bool loop = true;

    while (loop)
    {
        bool down = Touch_getXY();

        if (handleSliderTouch(sliderFilter, down))
        {
            drawRectWithText(360, 100, ORANGE, String(sliderFilter.sliderValue));
        }

        if (checkButton(back_btn, down))
        {
            state = SETTINGS;

            loop = false;
        }

        if (checkButton(change_filter_btn, down))
        {
            controller.setAlpha(sliderFilter.sliderValue);
            state = SETTINGS;
            loop = false;
        }
    }

    DBG("EXITING FILTERING PAGE");

    // clear page before going to the next
    tft.fillScreen(BLACK);
}

// ************************ MOTOR ************************

void UI::motorPage()
{
    Adafruit_GFX_Button back_btn;
    back_btn.initButton(&tft, 20, 20, 40, 40, BLACK, RED, BLACK, (char *)"<", 3);
    back_btn.drawButton(false);

    bool loop = true;

    while (loop)
    {
        bool down = Touch_getXY();

        if (checkButton(back_btn, down))
        {
            back_btn.drawButton(true);
            state = CREATE;

            loop = false;
        }
    }

    DBG("EXITING MOTOR PAGE");
    // clear page before going to the next
    tft.fillScreen(BLACK);
}

// ************************************************** PAGES **************************************************

void UI::showError(bool show, String msg)
{
    if (show)
    {
        if (!errorShowing)
        {
            String errorMsg = "ERROR"; //  + msg; // having some issue with removing so just show error
            drawRectWithText(0, 100, RED, errorMsg);
            errorShowing = true;
        }
    }
    else
    {
        drawRectWithText(0, 100, BLACK, "ERROR"); // black on black to hide for now :/
        errorShowing = false;
    }
}