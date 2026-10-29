# FAQ & Troubleshooting

Answers to common questions and solutions for common issues in **WiiMC-SS XE**.

---

## General Questions

### How does WiiMC-SS XE differ from WiiFin-XE?
* **WiiMC-SS XE** is a standalone, local media center designed to play video files, audio collections, and DVDs directly off SD cards, USB hard drives, or discs without a server.
* **WiiFin-XE** is a streaming client that connects to a [Jellyfin](https://jellyfin.org) server over your home Wi-Fi or LAN with a modern Netflix-style UI.
* Both apps work seamlessly together: you can jump from WiiMC to WiiFin via **Settings > Jellyfin (WiiFin)**, and returning from WiiFin's Local Media feature brings you right back to WiiMC.

### Why won't my Wii play commercial DVD discs?
Nintendo revised the internal optical drive in late 2009 (often referred to as D3-2 and D4 drives). These newer drives have modified drive controller boards that reject DVD-Video commands at the hardware level. If you have a newer Wii or a Wii U in vWii mode, DVD playback is physically impossible. You can, however, rip your DVDs to `.mkv` or `.mp4` on a PC and play them from an SD card or USB drive.

---

## Storage & Drives

### My USB hard drive is not detected. What should I do?
1. Ensure your USB drive is formatted as **FAT32** with a cluster size of 32 KB. WiiMC does not support NTFS or exFAT.
2. Connect your drive to **USB Port 0** (the bottom port when the Wii is laying flat, or the right port when standing upright).
3. If using an external 2.5" hard drive without its own power plug, use a **USB Y-cable** to draw sufficient power from both USB ports.

---

## Playback Issues

### Video is stuttering or dropping frames. How can I fix it?
* Rename the file to include `.dash` in the name (e.g. `video.dash.mp4`). This tells WiiMC to disable the CPU-heavy in-loop deblocking filter on H.264/VP8 files.
* Ensure the video resolution does not exceed 720x480 (NTSC) or 720x576 (PAL). The Wii cannot downscale HD 720p or 1080p video in real time.

### Audio or radio stream disconnected.
WiiMC-SS XE includes automatic reconnect logic for internet radio streams. It will retry connecting after 60 seconds of silence or network interruption.
