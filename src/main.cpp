#include "telemetry_reader.h"
#include "display_manager.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

// Global objects for signal handling
TelemetryReader* g_telemetry_reader = nullptr;
DisplayManager* g_display_manager = nullptr;

void signalHandler(int signal) {
    std::cout << "\nShutting down..." << std::endl;

    if (g_display_manager) {
        g_display_manager->stop();
    }

    if (g_telemetry_reader) {
        g_telemetry_reader->disconnect();
    }

    exit(0);
}

int main(int argc, char* argv[]) {
    std::string connection_url = "udp://:14540";

    // Parse command line arguments
    if (argc > 1) {
        connection_url = argv[1];
    }

    std::cout << "Drone Telemetry Dashboard" << std::endl;
    std::cout << "Connecting to: " << connection_url << std::endl;

    // Setup signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Create components
    TelemetryReader telemetry_reader;
    DisplayManager display_manager;

    // Set global pointers for signal handler
    g_telemetry_reader = &telemetry_reader;
    g_display_manager = &display_manager;

    // Setup callback
    telemetry_reader.setDataCallback([&display_manager](const TelemetryData& data) {
        display_manager.updateDisplay(data);
    });

    // Connect to drone
    if (!telemetry_reader.connect(connection_url)) {
        std::cerr << "Failed to connect to drone!" << std::endl;
        return 1;
    }

    // Start display
    display_manager.start();

    std::cout << "Dashboard started. Press 'q' in the display to quit." << std::endl;

    // Keep main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Check if display is still running
        // (In a real implementation, you'd have a better way to check this)
    }

    return 0;
}
