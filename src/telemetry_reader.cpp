#include "telemetry_reader.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

TelemetryReader::TelemetryReader()
    : _mavsdk(std::make_unique<mavsdk::Mavsdk>(mavsdk::Mavsdk::Configuration{mavsdk::ComponentType::GroundStation})) {
}

TelemetryReader::~TelemetryReader() {
    disconnect();
}

bool TelemetryReader::connect(const std::string& connection_url) {
    // Add connection
    auto connection_result = _mavsdk->add_any_connection(connection_url);
    if (connection_result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << std::endl;
        return false;
    }

    std::cout << "Waiting for system to connect..." << std::endl;

    // Use the modern first_autopilot method instead of subscribe_on_new_system
    auto system = _mavsdk->first_autopilot(10.0);
    if (!system) {
        std::cerr << "Timeout waiting for system" << std::endl;
        return false;
    }

    _system = system.value();
    _telemetry = std::make_unique<mavsdk::Telemetry>(_system);

    // Wait for system to be ready
    while (!_telemetry->health_all_ok()) {
        std::cout << "Waiting for system to be ready..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    setupSubscriptions();

    {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.connected = true;
    }

    std::cout << "Connected to system!" << std::endl;
    return true;
}

void TelemetryReader::disconnect() {
    if (_system) {
        {
            std::lock_guard<std::mutex> lock(_data_mutex);
            _current_data.connected = false;
        }
        _system.reset();
        _telemetry.reset();
    }
}

void TelemetryReader::setDataCallback(std::function<void(const TelemetryData&)> callback) {
    _data_callback = callback;
}

TelemetryData TelemetryReader::getCurrentData() const {
    std::lock_guard<std::mutex> lock(_data_mutex);
    return _current_data;
}

void TelemetryReader::setupSubscriptions() {
    // Position subscription
    _telemetry->subscribe_position([this](mavsdk::Telemetry::Position position) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.latitude = position.latitude_deg;
        _current_data.longitude = position.longitude_deg;
        _current_data.altitude_msl = position.absolute_altitude_m;
        _current_data.altitude_rel = position.relative_altitude_m;
        updateData();
    });

    // Attitude subscription
    _telemetry->subscribe_attitude_euler([this](mavsdk::Telemetry::EulerAngle attitude) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.roll = attitude.roll_deg;
        _current_data.pitch = attitude.pitch_deg;
        _current_data.yaw = attitude.yaw_deg;
        updateData();
    });

    // Velocity subscription
    _telemetry->subscribe_velocity_ned([this](mavsdk::Telemetry::VelocityNed velocity) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.velocity_north = velocity.north_m_s;
        _current_data.velocity_east = velocity.east_m_s;
        _current_data.velocity_down = velocity.down_m_s;
        updateData();
    });

    // Battery subscription
    _telemetry->subscribe_battery([this](mavsdk::Telemetry::Battery battery) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.battery_voltage = battery.voltage_v;
        _current_data.battery_current = battery.current_battery_a;
        _current_data.battery_remaining = battery.remaining_percent;
        updateData();
    });

    // GPS subscription
    _telemetry->subscribe_gps_info([this](mavsdk::Telemetry::GpsInfo gps) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.gps_satellites = gps.num_satellites;
        _current_data.gps_fix_type = static_cast<int>(gps.fix_type);
        updateData();
    });

    // Flight mode subscription
    _telemetry->subscribe_flight_mode([this](mavsdk::Telemetry::FlightMode mode) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.flight_mode = flightModeToString(mode);
        updateData();
    });

    // Armed subscription
    _telemetry->subscribe_armed([this](bool armed) {
        std::lock_guard<std::mutex> lock(_data_mutex);
        _current_data.armed = armed;
        updateData();
    });
}

std::string TelemetryReader::flightModeToString(mavsdk::Telemetry::FlightMode mode) {
    switch (mode) {
        case mavsdk::Telemetry::FlightMode::Unknown:
            return "Unknown";
        case mavsdk::Telemetry::FlightMode::Ready:
            return "Ready";
        case mavsdk::Telemetry::FlightMode::Takeoff:
            return "Takeoff";
        case mavsdk::Telemetry::FlightMode::Hold:
            return "Hold";
        case mavsdk::Telemetry::FlightMode::Mission:
            return "Mission";
        case mavsdk::Telemetry::FlightMode::ReturnToLaunch:
            return "Return to Launch";
        case mavsdk::Telemetry::FlightMode::Land:
            return "Land";
        case mavsdk::Telemetry::FlightMode::Offboard:
            return "Offboard";
        case mavsdk::Telemetry::FlightMode::FollowMe:
            return "Follow Me";
        case mavsdk::Telemetry::FlightMode::Manual:
            return "Manual";
        case mavsdk::Telemetry::FlightMode::Altctl:
            return "Altitude Control";
        case mavsdk::Telemetry::FlightMode::Posctl:
            return "Position Control";
        case mavsdk::Telemetry::FlightMode::Acro:
            return "Acro";
        case mavsdk::Telemetry::FlightMode::Stabilized:
            return "Stabilized";
        case mavsdk::Telemetry::FlightMode::Rattitude:
            return "Rattitude";
        default:
            return "Unknown";
    }
}

void TelemetryReader::updateData() {
    _current_data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    if (_data_callback) {
        _data_callback(_current_data);
    }
}
