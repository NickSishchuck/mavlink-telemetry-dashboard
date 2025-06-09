#!/bin/bash
# Kills all drone-related processes and frees up ports

set -e  # Exit on any error

echo " Drone Development Cleanup Script"
echo "=================================="

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to kill processes by name with feedback
kill_processes() {
    local process_name=$1
    local display_name=$2

    print_status "Stopping $display_name processes..."

    # Find PIDs of the process
    local pids=$(pgrep -f "$process_name" 2>/dev/null || true)

    if [ -z "$pids" ]; then
        print_status "No $display_name processes found"
        return 0
    fi

    echo "  Found PIDs: $pids"

    # Try graceful termination first (SIGTERM)
    echo "  Sending SIGTERM..."
    pkill -f "$process_name" 2>/dev/null || true

    # Wait a bit for graceful shutdown
    sleep 2

    # Check if any are still running
    local remaining_pids=$(pgrep -f "$process_name" 2>/dev/null || true)

    if [ ! -z "$remaining_pids" ]; then
        print_warning "Some $display_name processes still running, forcing termination..."
        echo "  Remaining PIDs: $remaining_pids"
        pkill -9 -f "$process_name" 2>/dev/null || true
        sleep 1
    fi

    # Final check
    local final_pids=$(pgrep -f "$process_name" 2>/dev/null || true)
    if [ -z "$final_pids" ]; then
        print_success "$display_name processes stopped"
    else
        print_error "Failed to stop some $display_name processes: $final_pids"
    fi
}

# Function to check and report port status
check_ports() {
    print_status "Checking port status..."

    local ports=("5760" "5762" "5763" "14550" "14551" "5501")
    local any_in_use=false

    for port in "${ports[@]}"; do
        local usage=$(ss -tulpn | grep ":$port " || true)
        if [ ! -z "$usage" ]; then
            print_warning "Port $port still in use:"
            echo "  $usage"
            any_in_use=true
        fi
    done

    if [ "$any_in_use" = false ]; then
        print_success "All drone ports are free"
    fi
}

# Main cleanup sequence
main() {
    echo
    print_status "Starting cleanup process..."
    echo

    # Kill each type of process
    kill_processes "sim_vehicle" "SITL sim_vehicle"
    kill_processes "arducopter" "ArduCopter"
    kill_processes "ArduCopter" "ArduCopter (alternative pattern)"
    kill_processes "mavproxy" "MAVProxy"
    kill_processes "telemetry_dashboard" "Telemetry Dashboard"
    kill_processes "qgroundcontrol" "QGroundControl"

    echo
    print_status "Waiting for processes to fully terminate..."
    sleep 3

    echo
    check_ports

    echo
    print_success "Cleanup complete! 🎉"
    echo
    print_status "You can now start fresh development sessions."
}

# Handle script arguments
case "${1:-}" in
    -h|--help)
        echo "Usage: $0 [OPTIONS]"
        echo
        echo "Drone Development Cleanup Script"
        echo "Kills all drone-related processes and frees up ports"
        echo
        echo "Options:"
        echo "  -h, --help     Show this help message"
        echo "  -v, --verbose  Show detailed process information"
        echo "  -f, --force    Force kill without graceful termination"
        echo
        echo "Processes cleaned up:"
        echo "  • SITL sim_vehicle"
        echo "  • ArduCopter"
        echo "  • MAVProxy"
        echo "  • Telemetry Dashboard"
        echo "  • QGroundControl"
        echo
        echo "Ports checked:"
        echo "  • 5760, 5762, 5763 (SITL)"
        echo "  • 14550, 14551 (MAVProxy)"
        echo "  • 5501 (Physics sim)"
        exit 0
        ;;
    -v|--verbose)
        set -x  # Enable verbose mode
        ;;
    -f|--force)
        print_warning "Force mode enabled - using SIGKILL immediately"
        # Override the kill function to use SIGKILL immediately
        kill_processes() {
            local process_name=$1
            local display_name=$2
            print_status "Force stopping $display_name processes..."
            pkill -9 -f "$process_name" 2>/dev/null || true
            sleep 1
            local remaining=$(pgrep -f "$process_name" 2>/dev/null || true)
            if [ -z "$remaining" ]; then
                print_success "$display_name processes stopped"
            else
                print_error "Failed to stop $display_name: $remaining"
            fi
        }
        ;;
    "")
        # No arguments, proceed normally
        ;;
    *)
        print_error "Unknown argument: $1"
        echo "Use -h or --help for usage information"
        exit 1
        ;;
esac

# Run main cleanup
main
