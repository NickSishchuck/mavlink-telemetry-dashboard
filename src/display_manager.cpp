#include "display_manager.h"
#include <iomanip>
#include <sstream>
#include <chrono>

DisplayManager::DisplayManager() {
}

DisplayManager::~DisplayManager() {
    stop();
}

void DisplayManager::start() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    // Enable colors if available
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Connected
        init_pair(2, COLOR_RED, COLOR_BLACK);    // Disconnected
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Warning
        init_pair(4, COLOR_CYAN, COLOR_BLACK);   // Headers
    }

    _running = true;
    _display_thread = std::thread(&DisplayManager::displayLoop, this);
}

void DisplayManager::stop() {
    _running = false;
    if (_display_thread.joinable()) {
        _display_thread.join();
    }
    endwin();
}

void DisplayManager::updateDisplay(const TelemetryData& data) {
    std::lock_guard<std::mutex> lock(_display_mutex);
    _latest_data = data;
}

void DisplayManager::displayLoop() {
    while (_running) {
        drawFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Check for quit key
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            _running = false;
            break;
        }
    }
}

void DisplayManager::drawFrame() {
    clear();

    std::lock_guard<std::mutex> lock(_display_mutex);

    drawTelemetryData();
    drawStatusBar();

    refresh();
}

void DisplayManager::drawTelemetryData() {
    int row = 1;

    // Title
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(row++, 2, "=== DRONE TELEMETRY DASHBOARD ===");
    attroff(COLOR_PAIR(4) | A_BOLD);
    row++;

    // Connection status
    if (_latest_data.connected) {
        attron(COLOR_PAIR(1));
        mvprintw(row++, 2, "Status: CONNECTED");
        attroff(COLOR_PAIR(1));
    } else {
        attron(COLOR_PAIR(2));
        mvprintw(row++, 2, "Status: DISCONNECTED");
        attroff(COLOR_PAIR(2));
    }
    row++;

    // Flight status
    attron(COLOR_PAIR(4));
    mvprintw(row++, 2, "FLIGHT STATUS:");
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 4, "Mode: %s", _latest_data.flight_mode.c_str());
    mvprintw(row++, 4, "Armed: %s", _latest_data.armed ? "YES" : "NO");
    row++;

    // Position
    attron(COLOR_PAIR(4));
    mvprintw(row++, 2, "POSITION:");
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 4, "Latitude:  %s°", formatDouble(_latest_data.latitude).c_str());
    mvprintw(row++, 4, "Longitude: %s°", formatDouble(_latest_data.longitude).c_str());
    mvprintw(row++, 4, "Alt (MSL): %s m", formatFloat(_latest_data.altitude_msl).c_str());
    mvprintw(row++, 4, "Alt (REL): %s m", formatFloat(_latest_data.altitude_rel).c_str());
    row++;

    // Attitude
    attron(COLOR_PAIR(4));
    mvprintw(row++, 2, "ATTITUDE:");
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 4, "Roll:  %s°", formatFloat(_latest_data.roll).c_str());
    mvprintw(row++, 4, "Pitch: %s°", formatFloat(_latest_data.pitch).c_str());
    mvprintw(row++, 4, "Yaw:   %s°", formatFloat(_latest_data.yaw).c_str());
    row++;

    // Velocity
    attron(COLOR_PAIR(4));
    mvprintw(row++, 2, "VELOCITY (NED):");
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 4, "North: %s m/s", formatFloat(_latest_data.velocity_north).c_str());
    mvprintw(row++, 4, "East:  %s m/s", formatFloat(_latest_data.velocity_east).c_str());
    mvprintw(row++, 4, "Down:  %s m/s", formatFloat(_latest_data.velocity_down).c_str());
    row++;

    // Battery
    attron(COLOR_PAIR(4));
    mvprintw(row++, 2, "BATTERY:");
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 4, "Voltage:   %s V", formatFloat(_latest_data.battery_voltage).c_str());
    mvprintw(row++, 4, "Current:   %s A", formatFloat(_latest_data.battery_current).c_str());
    mvprintw(row++, 4, "Remaining: %s%%", formatFloat(_latest_data.battery_remaining).c_str());
    row++;

    // GPS
    attron(COLOR_PAIR(4));
    mvprintw(row++, 2, "GPS:");
    attroff(COLOR_PAIR(4));
    mvprintw(row++, 4, "Satellites: %d", _latest_data.gps_satellites);
    mvprintw(row++, 4, "Fix Type:   %d", _latest_data.gps_fix_type);
}

void DisplayManager::drawStatusBar() {
    int height, width;
    getmaxyx(stdscr, height, width);

    attron(COLOR_PAIR(3));
    mvprintw(height - 1, 2, "Press 'q' to quit | Update rate: 10Hz");
    attroff(COLOR_PAIR(3));
}

std::string DisplayManager::formatDouble(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string DisplayManager::formatFloat(float value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}