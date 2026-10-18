# sys-ftpd

## FW 23 compatibility test build

Fork: https://github.com/hayashikentaro/sys-ftpd

This fork targets HOS 23.x. The local `nxdumptool` fork's FW 23 build was
confirmed on hardware by its user. **This sys-ftpd build still needs its own
hardware validation**; a successful build does not establish compatibility.

Changes in this fork:

- Detect HOS before initializing filesystem, time, HID, and sockets, matching
  libnx's service initialization order. Firmware lookup failures stop startup
  rather than continuing with an unknown version.
- Initialize/update controller pad state before selecting the LED target.
- Make `hid:sys` and LED enumeration failures nonfatal and bound the LED ID loop.
- Release HID services on exit.

### Build

Docker must be running. The pinned image is the locally available devkitA64
image also used for nxdumptool: libnx 4.12.0-1, devkitA64 r29.2-1 (GCC 15.2.0).
The nxdumptool key-generation patch is specific to dumping and is not needed
for an FTP server.

```sh
./build.sh clean
./build.sh -j4
```

Output: `out/atmosphere/contents/420000000000000E/exefs.nsp`, boot flag,
and configuration. GitHub Actions uploads `sys-ftpd-fw23-diagnostic.zip` and the ELF
as build artifacts; it does not publish untested builds as stable releases.

### Diagnostic build

The ZIP now includes `config/sys-ftpd/diagnostics_enabled`. Install this empty
marker alongside the new executable, then reboot into CFW to start diagnostics.
The marker does not change your existing login configuration.

Logs: `config/sys-ftpd/logs/diagnostic.log` and `diagnostic.previous.log`.
The previous boot's current log is preserved at startup; size rotation keeps
approximately 2 MiB per file (plus the last record). Each record is closed to
make it available for FTP download. Remove the marker and reboot to disable.

Records include firmware/build, PASV socket/options/bind/listen/getsockname
results, errno on failure, the last libnx BSD result, socket descriptors,
session IDs/counts, heap allocated/free bytes, response codes, socket closes,
and server restart/exit events. Heap free bytes describe the C allocator arena,
not BSD/kernel free memory. A BSD result is the last result and may be stale on
success. Diagnostic records exclude command arguments and credentials.

The existing optional verbose logger was also fixed to preserve errno, avoid
replacing stdout/stderr, and tolerate an unavailable log file. Its password
command logging is now redacted. The diagnostic marker does not enable this
verbose logger.

Initial hardware checks passed upload/download (including a 1 MiB binary and
an empty file), rename/delete, and 12 reconnects spaced one second apart.
A rapid connection sequence produced a transient PASV 451 and a connection
reset, then recovered without intervention. The cause is **not established**;
this diagnostic build retains socket sizes and connection behavior to gather
evidence. Sleep/wake remains untested.

### Install and validate

1. Preserve your existing `config/sys-ftpd/config.ini`. For an update, copy only
   the ZIP's `atmosphere` directory to the SD root; copy `config` only on a new
   installation, then set a username and password.
2. Reboot into a CFW version compatible with your HOS version.
3. Check that the console boots and FTP authentication works on port 5000.
4. Check directory listing, upload, download (compare file hashes), rename,
   delete, disconnect/reconnect, and a transfer after sleep/wake.
5. Repeat with LED notifications enabled and disabled; record HOS, Atmosphere,
   and any crash result/program ID. This module's ID is `420000000000000E`.

If it prevents boot, remove
`atmosphere/contents/420000000000000E/flags/boot2.flag` from the SD card
using a computer to disable automatic startup.

#### Previously sys-ftpd-light

This is a lightweight FTP server that runs in the background on your Nintendo Switch.

 - It's a lightweight version of mtheall's [ftpd](https://github.com/mtheall/ftpd) app run as a background service (sysmodule).
 - Originally forked from jakibaki's [sys-ftpd](https://github.com/jakibaki/sys-ftpd) in an attempt to improve peformance and stability.  

Since it's lightweight, it occupies less memory on your console at the cost of transferring files a bit slower. If you want to transfer large files, I would suggest you install mtheall's [ftpd](https://github.com/mtheall/ftpd) separately and run it whenever you need to make a large file transfer.

## How to use
1. Go to the [latest release](https://github.com/hayashikentaro/sys-ftpd/actions) and download the `sys-ftpd-fw23-diagnostic` build artifact and extract the enclosed ZIP.
2. Extract the contents of the folder to the root of your Nintendo Switch's SD card. (it should overlap with your existing atmosphere and config folders)
3. Go to config/sys-ftpd/config.ini and set your username and password for the FTP server. (otherwise it won't let you connect)
   - Alternatively you can enable anonymous mode, which will let anyone in the network connect to your FTP server without credentials. (unsafe)
4. Boot/reboot your Nintendo Switch into CFW as usual.
5. Once your console is connected to a network, you can connect to your server with any FTP client (you can find them online) from any computer within the same network.
   - The IP address of your Nintendo Switch can be found in your console's System Settings -> Internet -> Connection Status -> IP address. (it usually looks like 192.168.X.X)
   - The port can be found and modified in the config.ini mentioned above. (it's 5000 by default)
   - The resulting address should look something like `192.168.X.X:5000`, where your username and password are your `user` and `password` from config.ini respectively.
6. You should now be able to enjoy accessing your Nintendo Switch files remotely.

## Other

Sysmodule program ID: **420000000000000E**

---

Config Example (Located on your sd in `sdmc:/config/sys-ftpd/config.ini`):

```
[User]
user:=jeremy

# user:= -> Login username

[Password]
password:=ilovecars

# password:= -> Login password

[Port]
port:=5000

# port:=5000 -> opens the server on port 5000 (using the console's IP address).

[Anonymous]
anonymous:=0

# anonymous:=1 -> Anyone can connect to the server. (dangerous!)
# anonymous:=0 -> Only allows logging into the ftpd server with the correct username and password. user and password (in fields above) must be set.

[LED]
led:=1

# led:=1 -> LED flashes on connect (default)
# led:=0 -> LED does not flash on connect
```
