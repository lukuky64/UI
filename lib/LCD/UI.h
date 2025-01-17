#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include "ROCKET_SIM.h"
#include "Controller.h"

struct sliderObj
{
    uint16_t yPos;
    String text;
    float sliderValue;
    float minSliderValue;
    float maxSliderValue;
};

class UI
{
public:
    UI();
    void begin();

    // Pages
    void startPage();
    void settingsPage();
    void drawRectWithText();
    void calibrationPage();
    void createPage();
    void uploadPage();
    void pointPage();
    void runPage();
    void motorPage();
    void endPage();
    void filteringPage();
    void sensorPlotterPage();
    void gainSelectPage();

    // Creating objects
    void showError(bool show, String msg = "");
    void drawRectWithText(int16_t yPos, int16_t width, uint16_t colour, String text);
    void progressBar(String text, float progress, int16_t yPos, uint16_t colour);
    void drawSlider(sliderObj &slider);
    void drawGraph(float apogee, float finishTime);

    // Event handling
    bool handleSliderTouch(sliderObj &slider, bool down);
    bool checkButton(Adafruit_GFX_Button &btn, bool down);
    bool Touch_getXY();
    void command();

private:
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
    float compute_a_b(double g, double t_b, double S_a);
    void updateTextBox(String text);

    sim_data data;
    Controller controller;

    bool errorShowing = false;

    const int MINPRESSURE = 200;
    const int MAXPRESSURE = 1000;

    const int touch_delay = 200;

    // 320x480 ID=0x6814
    const int XP = 7;
    const int XM = CONFIG_A1;
    const int YP = CONFIG_A2;
    const int YM = 6;

    const int TS_LEFT = 100;
    const int TS_RT = 905;
    const int TS_TOP = 69;
    const int TS_BOT = 927;

    const uint16_t BLACK = 0x0000;
    const uint16_t BLUE = 0x001F;
    const uint16_t RED = 0xF800;
    const uint16_t GREEN = 0x07E0;
    const uint16_t PURPLE_1 = 0x3048;
    const uint16_t PURPLE_2 = 0x78f5;
    const uint16_t PURPLE_3 = 0xaa5b;
    const uint16_t PURPLE_4 = 0xee9e;
    const uint16_t CYAN = 0x07FF;
    const uint16_t MAGENTA = 0xF81F;
    const uint16_t YELLOW = 0xFFE0;
    const uint16_t WHITE = 0xFFFF;
    const uint16_t ORANGE = 0xFDA0;

    // Constants for slider positions and graph area
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 480;
    const int GRAPH_TOP = 80;
    int GRAPH_HEIGHT = 100;
    const int SLIDER_Y = (SCREEN_HEIGHT / 2 + 10);
    const int SLIDER_WIDTH = 280;
    const int SLIDER_HEIGHT = 60;

    sliderObj sliderApogee = {250, "Apogee", 200, 25, 2000};
    sliderObj sliderBurnTime = {320, "Burn time", 2, 0.05, 4};
    sliderObj sliderFilter = {250, "Alpha", 0.5, 0, 0.99};

    // Touch_getXY() updates global vars
    int16_t pixel_x;
    int16_t pixel_y;

    enum pageState
    {
        START,
        SETTINGS,
        CALIBRATE,
        CREATE,
        UPLOAD,
        POINT,
        RUN,
        MOTOR,
        FILTERING,
        GAIN_SELECT
    };

    pageState state;

    bool LANDSCAPE = false;

    MCUFRIEND_kbv tft;
    TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
};