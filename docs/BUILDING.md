# Building WiiMC-SS XE

Instructions for compiling **WiiMC-SS XE** from source.

---

## Toolchain & Dependencies

WiiMC-SS XE requires devkitPro's PowerPC toolchain:

* **devkitPPC** (r40 or later recommended)
* **libogc**
* Custom compiled portlibs: `libmad`, `libvorbisidec`, `libpng`, `libjpeg`, `freetype`, `mxml`

---

## Compiling

1. Clone the repository recursively:
   ```bash
   git clone --recursive https://github.com/CalyxOfHeld/WiiMC-SS-XE.git
   cd WiiMC-SS-XE
   ```

2. Ensure your devkitPro environment variables are exported:
   ```bash
   export DEVKITPRO=/opt/devkitpro
   export DEVKITPPC=$DEVKITPRO/devkitPPC
   ```

3. Build the DOL executable:
   ```bash
   make
   ```

4. The compiled binary will be generated at:
   ```text
   apps/WiiMC/boot.dol
   ```

---

## Contributing

Pull requests and issues are welcome! Feel free to report bugs or submit feature enhancements on our [GitHub repository](https://github.com/CalyxOfHeld/WiiMC-SS-XE).
