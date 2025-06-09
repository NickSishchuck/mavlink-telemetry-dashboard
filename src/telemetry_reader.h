#pragma once

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>


struct TelemetryData {
    // Position data
    double latitude = 0.0;
    double longitude = 0.0;
    float altitude_msl = 0.0f;
    float altitude_rel = 0.0f;

    // Attitude data
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    // Velocity data
    float velocity_north = 0.0f;
    float velocity_east = 0.0f;
    float velocity_down = 0.0f;

    // Battery data
    float battery_voltage = 0.0f;
    float battery_current = 0.0f;
    float battery_remaining = 0.0f;

    // GPS data
    int gps_satellites = 0;
    int gps_fix_type = 0;

    // Flight mode
    std::string flight_mode = "UNKNOWN";
    bool armed = false;

    // Connection status
    bool connected = false;
    uint64_t timestamp = 0;
};

class TelemetryReader {
public:
    TelemetryReader();
    ~TelemetryReader();

    bool connect(const std::string& connection_url = "udp://:14540");
    void disconnect();

    void setDataCallback(std::function<void(const TelemetryData&)> callback);

    TelemetryData getCurrentData() const;

private:
    std::unique_ptr<mavsdk::Mavsdk> _mavsdk;
    std::shared_ptr<mavsdk::System> _system;
    std::unique_ptr<mavsdk::Telemetry> _telemetry;

    TelemetryData _current_data;
    mutable std::mutex _data_mutex;

    std::function<void(const TelemetryData&)> _data_callback;

    void setupSubscriptions();
    void updateData();
};
