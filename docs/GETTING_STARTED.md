# Getting Started

Get up and running with **WiiMC-SS XE** on your Nintendo Wii or Wii U (vWii) in just a few minutes.

---

## Requirements

* A Nintendo Wii or Wii U in Virtual Wii mode with the **Homebrew Channel** installed.
* An SD card (standard SD or SDHC) or a USB mass storage drive formatted as **FAT32**.
* (Optional) **WiiFin-XE** installed in `apps/WiiFin/` if you want two-way switching between local playback and Jellyfin network streaming.

---

## Installation

1. Download the latest `WiiMC-SS-XE.zip` from our GitHub releases.
2. Extract the archive contents to the root of your SD card or USB drive.
3. Verify that the files are placed as follows:
   ```text
   sd:/ (or usb:/)
   └── apps/
       └── wiimc/
           ├── boot.dol
           ├── meta.xml
           ├── icon.png
           └── settings.xml (created on first run)
   ```

---

## Launching

1. Insert your SD card or connect your USB drive to **USB Port 0** (the bottom port on a horizontal Wii, or outer port on a vertical Wii).
2. Power on your console and open the **Homebrew Channel**.
3. Select **WiiMC-SS XE** and press **Start**.

---

## Optional Arguments in `meta.xml`

You can customize launch behavior by editing the `<arguments>` block in `apps/wiimc/meta.xml`:

```xml
<arguments>
  <!-- Randomize the menu color theme on startup -->
  <arg>theme="random"</arg>

  <!-- Force a custom IOS (e.g. IOS202 for specific USB modules) -->
  <!-- <arg>ios=202</arg> -->

  <!-- Force 32kHz audio output -->
  <!-- <arg>32khz=1</arg> -->

  <!-- Enable Nintendo 3DS controller input listener -->
  <!-- <arg>3ds=1</arg> -->
</arguments>
```
