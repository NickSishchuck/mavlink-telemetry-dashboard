# Drone Telemetry Development Setup Guide

## 1. Development Environment Setup

### Initial Setup (one-time)
```bash
# Navigate to ArduPilot directory
cd ~/ardupilot

# Activate the Python virtual environment
source ~/ardupilot-venv/bin/activate

# Verify environment
which python  # Should show: /home/nick/ardupilot-venv/bin/python
```

### Daily Development Routine
```bash
# 1. Activate virtual environment
source ~/ardupilot-venv/bin/activate

# 2. Navigate to ArduPilot
cd ~/ardupilot

# 3. Build telemetry dashboard (if code changed)
cd ~/path/to/your/telemetry/project
make  # or whatever your build command is

# 4. Ready to start SITL and test!
```

## 2. Port Configuration Reference

| Port | Service | Purpose | Connection Type |
|------|---------|---------|----------------|
| **5760** | ArduCopter SITL | Primary flight controller | TCP Server |
| **5762** | ArduCopter SITL | Secondary serial port | TCP Server |
| **5763** | ArduCopter SITL | Third serial port | TCP Server |
| **14550** | MAVProxy Output | Standard MAVLink broadcast | UDP |
| **14551** | Custom | Alternative UDP port | UDP |
| **5501** | SITL Physics | Physics simulation | UDP |

### Connection Directions
- **tcpout://IP:PORT** = Connect TO a server (client mode) ✅
- **tcpin://IP:PORT** = Listen for connections (server mode) ❌ conflicts with SITL
- **udpin://:PORT** = Listen for UDP packets on PORT
- **udpout://IP:PORT** = Send UDP packets to IP:PORT

## 3. Connection Scenarios

### Scenario A: Direct SITL Connection (Simplest)

**Use when:** Testing telemetry dashboard only, no ground control needed

```bash
# 1. Start SITL without MAVProxy
cd ~/ardupilot
./Tools/autotest/sim_vehicle.py -v ArduCopter --no-mavproxy

# 2. Connect telemetry dashboard directly
./telemetry_dashboard tcpout://127.0.0.1:5760
```

**Connections:**
```
ArduCopter SITL (5760) ←── telemetry_dashboard
```

### Scenario B: SITL + QGroundControl (No MAVProxy)

**Use when:** Need manual control + telemetry monitoring

```bash
# 1. Start SITL without MAVProxy
cd ~/ardupilot
./Tools/autotest/sim_vehicle.py -v ArduCopter --no-mavproxy

# 2. Connect telemetry dashboard to primary port
./telemetry_dashboard tcpout://127.0.0.1:5760

# 3. Connect QGroundControl to secondary port
# In QGC: TCP connection to 127.0.0.1:5762
```

**Connections:**
```
ArduCopter SITL (5760) ←── telemetry_dashboard
ArduCopter SITL (5762) ←── QGroundControl
```

### Scenario C: Full Setup with MAVProxy

**Use when:** Need command-line control + multiple ground stations

```bash
# 1. Start SITL with MAVProxy
cd ~/ardupilot
./Tools/autotest/sim_vehicle.py -v ArduCopter

# 2. Connect telemetry dashboard to MAVProxy UDP output
./telemetry_dashboard udpin://:14550

# 3. Connect QGroundControl to MAVProxy
# In QGC: UDP connection to 127.0.0.1:14550
```

**Connections:**
```
ArduCopter SITL (5760) ←── MAVProxy ──→ UDP (14550) ──→ telemetry_dashboard
                                     ──→ UDP (14550) ──→ QGroundControl
```

### Scenario D: MAVProxy + Custom UDP Port

**Use when:** Multiple applications need telemetry data

```bash
# 1. Start SITL without MAVProxy
cd ~/ardupilot
./Tools/autotest/sim_vehicle.py -v ArduCopter --no-mavproxy

# 2. Start MAVProxy with custom output
mavproxy.py --master tcp:127.0.0.1:5760 --out udp:127.0.0.1:14551

# 3. Connect telemetry dashboard to custom port
./telemetry_dashboard udpin://:14551

# 4. Connect QGroundControl to standard port
# In QGC: UDP connection to 127.0.0.1:14550
```

## 4. Troubleshooting

### Common Issues

**"Address already in use" errors:**
```bash
# Kill all related processes
pkill -f sim_vehicle
pkill -f arducopter
pkill -f mavproxy

# Wait for ports to be released
sleep 3

# Verify ports are free
ss -tulpn | grep -E "(5760|14550)"
```

**"Connection refused" errors:**
- Check if SITL is actually running and listening
- Verify you're using correct connection direction (tcpout vs tcpin)
- Try different ports (5762, 5763)

**MAVProxy connection issues:**
- Make sure SITL started successfully first
- Check for bind port errors in SITL output
- Try connecting MAVProxy to different SITL ports

### Debug Commands
```bash
# Check what's listening on ports
ss -tulpn | grep -E "(5760|5762|14550)"

# Check active connections
netstat -an | grep -E "(5760|5762|14550)"

# Find processes using specific ports
lsof -i :5760
```

## 5. Quick Reference Commands

### Essential Commands
```bash
# Activate environment
source ~/ardupilot-venv/bin/activate

# Start basic SITL
cd ~/ardupilot && ./Tools/autotest/sim_vehicle.py -v ArduCopter --no-mavproxy

# Connect dashboard
./telemetry_dashboard tcpout://127.0.0.1:5760

# Emergency cleanup
pkill -f sim_vehicle && pkill -f arducopter
```

### MAVProxy Commands (when connected)
```bash
# Arm vehicle
arm throttle

# Change flight mode
mode GUIDED

# Takeoff
takeoff 10

# Land
land

# Disarm
disarm
```

## 6. Recommended Development Workflow

1. **Start with Scenario A** for basic telemetry testing
2. **Move to Scenario B** when you need to control the vehicle
3. **Use Scenario C** for advanced multi-client development
4. **Always verify connections** with `ss -tulpn` before troubleshooting
