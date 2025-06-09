#pragma once

#include "telemetry_reader.h"
#include <ncurses.h>
#include <atomic>
#include <thread>
#include <mutex>

class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    void start();
    void stop();

    void updateDisplay(const TelemetryData& data);

private:
    std::atomic<bool> _running{false};
    std::thread _display_thread;
    TelemetryData _latest_data;
    mutable std::mutex _display_mutex;

    void displayLoop();
    void drawFrame();
    void drawTelemetryData();
    void drawStatusBar();

    std::string formatDouble(double value, int precision = 6);
    std::string formatFloat(float value, int precision = 2);
};
