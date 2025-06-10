
## Quick Launch Guide

### Terminal 1: Start SITL
```bash
# Activate environment
source ~/mavlink_stack/mavlink_env/bin/activate

# Navigate and start SITL
cd ~/ardupilot
./Tools/autotest/sim_vehicle.py -v ArduCopter --no-mavproxy
```
Wait for: "bind port 5760 for 0" and "Serial port 0 on TCP port 5760"

### Terminal 2: Start MAVProxy
```bash
# Activate environment
source ~/mavlink_stack/mavlink_env/bin/activate

# Start MAVProxy with dual outputs
mavproxy.py --master tcp:127.0.0.1:5760 --out udp:127.0.0.1:14550 --out udp:127.0.0.1:14551
```
Wait for: Connection confirmation and vehicle data

### Terminal 3: Start Telemetry Dashboard
```bash
./telemetry_dashboard udpin://0.0.0.0:14550
```

### Start QGroundControl
Type: UDP
Server adress: 127.0.0.1:14551

## Port Configuration Reference

## Port Reference Chart

| Port | Component | Direction | Purpose | Protocol |
|------|-----------|-----------|---------|----------|
| **5760** | ArduCopter SITL | TCP Server | Primary flight controller connection | TCP |
| **5762** | ArduCopter SITL | TCP Server | Secondary serial port | TCP |
| **5763** | ArduCopter SITL | TCP Server | Third serial port | TCP |
| **14550** | MAVProxy Output | UDP Broadcast | Standard MAVLink telemetry | UDP |
| **14551** | MAVProxy Output | UDP Broadcast | Secondary telemetry stream | UDP |
| **5501** | SITL Physics | UDP | Physics simulation data | UDP |


### Connection Directions
- **tcpout://IP:PORT** = Connect TO a server (client mode) ✅
- **tcpin://IP:PORT** = Listen for connections (server mode) ❌ conflicts with SITL
- **udpin://:PORT** = Listen for UDP packets on PORT
- **udpout://IP:PORT** = Send UDP packets to IP:PORT
