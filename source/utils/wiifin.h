/* wiifin.h: WiiFin, the Jellyfin client, started from here (Settings >
 * Jellyfin), and the way back to it when it started WiiMC-SS XE (its
 * Local Media: wiifin=<its DOL> among the arguments) */
#ifndef _WIIFIN_H_
#define _WIIFIN_H_

extern char wiifinPath[256];   /* wiifin=: WiiFin's DOL, started from it */
extern bool LaunchWiiFin;      /* Settings > Jellyfin */

/* WiiFin there (wiifin=, apps/WiiFin, or an app whose meta.xml names it
 * WiiFin; the SD card, then USB): asked before leaving for it, so WiiMC
 * stays open with a notice when it isn't */
bool WiiFinFound();
/* WiiFin's DOL read (the devices mounted still); false if not there */
bool WiiFinLoad();
/* the system shut down and WiiFin started; returns only if not loaded */
void WiiFinGo();

#endif
