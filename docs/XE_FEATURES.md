# What's New in XE

WiiMC-SS XE builds upon **WiiMC-SS** by SuperrSonic and the foundational work of **WiiMC** by Tantric and rodries. It introduces seamless integration with our companion Jellyfin streaming app, **WiiFin-XE**, along with polished usability improvements.

---

## WiiFin-XE Integration

### Direct Launch from Settings
You can jump straight into **WiiFin-XE** from inside WiiMC:
1. Open the top toolbar and click **Settings** (or press the Settings button).
2. Select **Jellyfin (WiiFin)**.
3. WiiMC-SS XE boots directly into WiiFin without needing to return to the Homebrew Channel or Wii Menu.

### Local Media Handoff
When launched from WiiFin-XE's **Local Media** feature, WiiMC-SS XE remembers its caller. When you are finished watching or listening and choose **Exit**, you are immediately returned to WiiFin-XE. Exiting from a normal standalone launch still safely exits to HBC or Wii Menu as configured.

### Fail-Safe DOL Validation
Switching between applications is guarded by strict safety checks:
- Verifies storage device availability on both SD card (`sd:/apps/WiiFin/boot.dol`) and USB drive (`usb:/apps/WiiFin/boot.dol`).
- Reads and validates the DOL executable header magic bytes before unloading the GUI.
- If WiiFin-XE cannot be found or the file is corrupted, WiiMC-SS XE presents an on-screen notification and cleanly keeps WiiMC running instead of hanging or crashing to a black screen.

---

## Core WiiMC-SS Enhancements

WiiMC-SS XE incorporates all performance optimizations and format enhancements developed in WiiMC-SS:

* **Tiled 720x480p Video Rendering:** Native widescreen output utilizing MPlayer CE's high-speed video backplane.
* **Deblock Skip via `.dash`:** Append `.dash` to heavy H.264 / VP8 files to skip the in-loop deblocking filter, drastically reducing CPU overhead for smooth playback.
* **Dynamic & Embedded Cover Art:** Renders embedded album art in MP3, AAC, FLAC, OGG, WMA, and AIFF. Displays dynamic cover art updates for radio streams (TuneIn, Yggdrasil Radio, ANISON.FM).
* **Game Audio Looping:** Correct loop point handling for game sound formats including Nintendo ADX and BRSTM.
* **Interlaced MPEG-2 Sync:** Superior handling for interlaced DVD and TV rips by synchronizing field order.
* **Comprehensive Controller Support:** Navigate with the Wii Remote, Classic Controller, GameCube controller, USB keyboard, or Nintendo 3DS.
