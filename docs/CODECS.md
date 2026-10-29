# Codecs & Playback Guide

WiiMC-SS XE uses a heavily customized MPlayer back-end tailored to the Wii's Broadway PowerPC CPU (729 MHz) and 88 MB of total RAM.

---

## Supported Video Formats

| Video Codec | Containers | Resolution Notes |
| :--- | :--- | :--- |
| **MPEG-2** | `.mpg`, `.vob`, `.m2ts`, `.mkv` | Up to 720x480 / 720x576 at full framerate. Hardware field sync supported for interlaced content. |
| **MPEG-4 Part 2 (Xvid / DivX)** | `.avi`, `.mp4`, `.mkv` | Plays smoothly up to standard 480p/576p resolutions. |
| **H.264 (AVC)** | `.mp4`, `.mkv`, `.dash` | Baseline and Main profiles up to 480p. Use `.dash` extension for heavy files. |
| **VP8** | `.webm`, `.dash` | Standard definition WebM content. |
| **MPEG-1** | `.mpg`, `.dat` | VCD and early digital formats. |

---

## Performance Trick: The `.dash` Extension

H.264 and VP8 videos feature an **in-loop deblocking filter**. While this smooths compression blockiness, it consumes massive CPU power on the Wii's 729 MHz processor, often causing dropped frames on higher bitrate 480p files.

To bypass the deblock filter for maximum speed:
1. Rename your file to include `.dash` before the file extension, or append `.dash` to the filename:
   * Example: `movie.dash.mp4` or `episode.dash.mkv`
2. WiiMC-SS XE detects `.dash` and instructs the decoder to skip the deblocking filter completely.
3. Playback runs significantly faster and stays in sync without frame drops.

---

## Audio Codecs

* **Lossy:** MP3, AAC (LC & HE), AC3 (Dolby Digital), WMA, OGG Vorbis
* **Lossless:** FLAC, ALAC (Apple Lossless), AIFF, WAV
* **Game Audio:** ADX, BRSTM (with authentic looping support)

---

## Subtitle Formats

* **External:** SubRip (`.srt`), MicroDVD (`.sub`)
* **Embedded:** MKV text and ASS/SSA streams.
* Subtitle rendering honors Display Aspect Ratio (DAR) rather than Sample Aspect Ratio (SAR) to avoid distortion on widescreen 16:9 displays.
