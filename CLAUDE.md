# PicoProbe - I2C Bus Sniffer

Passive I2C bus analyzer for Pico 2W. Sniffs I2C traffic without interfering with the bus.

**GitHub:** (create with `gh repo create PicoProbe --public --source=. --push`)

## Quick Start

```bash
# Build and upload
cd D:/CodeLab/PicoProbe
pio run -t upload

# Monitor
pio device monitor
```

## Wiring

```
Pico 2W          Target I2C Bus
--------         --------------
GPIO4  --------> SDA
GPIO5  --------> SCL
GND    --------> GND
```

**IMPORTANT:** This is passive sniffing. Don't connect 3V3 - just GND and the signal lines.

## Usage

Connect to the target I2C bus and open serial monitor at 115200 baud.

**Output format:**
```
[timestamp] ADDR W/R: DATA...
[    1234] 0x50 W: 00 10       <- Write to addr 0x50
[    1240] 0x50 R: 12 34 56    <- Read from addr 0x50
```

**Commands:**
- `r` - Reset/clear buffer and stats
- `s` - Show statistics
- `h` - Toggle hex/decimal output
- `p` - Pause/resume capture

## Hardware

- **Board:** Pico 2W (RP2350 + WiFi)
- **Pins:** GPIO4 (SDA), GPIO5 (SCL) - configurable in main.cpp
- **Speed:** Should handle 100kHz and 400kHz I2C

## Use Case: OREI HDBaseT Extender

Sniff I2C between N32G435 MCU and GSV5100-HD video codec to:
1. Find I2C addresses used
2. Capture configuration sequence on power-up
3. Identify registers that control video routing
4. Potentially find debug/parallel output enable commands

## BEFORE YOU CODE - Check ForgeRepo!

```bash
python D:/CodeLab/ForgeRepo/capabilities_registry.py --search "I2C"
```

## Session Checklist

**End of session:**
```bash
git add -A && git commit -m "description" && git push
python D:/CodeLab/ForgeRepo/scripts/session_end.py
```
