# dwl - dwm for Wayland

Join us on our [Discord server]  
Or Matrix: [#dwl-official:matrix.org]  
Or on our IRC channel: [#dwl on Libera Chat]

dwl is a compact, hackable compositor for [Wayland] based on [wlroots]. It is
intended to fill the same space in the Wayland world that [dwm] does in X11,
primarily in terms of functionality, and secondarily in terms of
philosophy. Like [dwm], dwl is:

- Easy to understand, hack on, and extend with patches
- One C source file configurable via `config.h`
- Tied to as few external dependencies as possible

## This fork (`local/main`)

This repository is a personal fork of upstream dwl. On top of the upstream
tree it tracks the complete configuration, a `PKGBUILD` and a build check so
the patched compositor can be rebuilt reproducibly from a clean checkout:

- `config.local.h` is the tracked configuration (current keybinds, colors,
  terminal, scratchpad and swallowing settings). It is byte-for-byte what the
  live session uses.
- `config.h` remains the per-machine file and stays in `.gitignore`. `make`
  creates it from `config.local.h` only when it is missing, so a clean checkout
  builds and an existing `config.h` is never overwritten.
- `PKGBUILD` builds `local/main` and installs `dwl`, `dwl-close`, the man page
  and the desktop file.
- `scripts/build-check.sh` compiles the working tree in a temporary directory
  without `config.h`.

### Reproducible configuration

`config.def.h` is the upstream reference and intentionally lags this fork's
choices (for example its scratchpad keys are still `MOD+z`). The source of
truth for this machine is `config.local.h`.

Edit `config.h` (the live file) as usual, then promote the change to the
tracked template:

    cp config.h config.local.h
    git diff -- config.local.h

On a fresh checkout:

    make                # config.h missing -> copied from config.local.h
    make clean && make

`config.h` is never committed; do not add it to the repository.

Key differences from upstream `config.def.h`: `MODKEY` is `LOGO`, `TERMINAL`
is `footclient` (shared `foot --server`), the menu is `fuzzel`, only the tile
layout is enabled, scratchpads use `MOD+Tab` / `MOD+Shift+Tab` instead of
`MOD+z`, and the local `toggletabletmap`, `resizefloat` and `movestack`
bindings are present. See `config.local.h` for the complete list.

## Patch set

Upstream base: `4ff9db2`, the fork point shared with `fork/main` (current
`origin/main` is a few commits ahead at `4ecb2a9`). Everything listed below is
local to this fork and is **not** in upstream dwl; the local patches are the 11
commits in `4ff9db2..HEAD`.

| Feature | Commits | Origin | Summary |
| --- | --- | --- | --- |
| dwl-ipc + close request | `244a87b`, `e815068`, `a7d6569` | Local, adapted from the somebar/raphi IPC patch set (not upstream) | `dwl-ipc-unstable-v2` manager v3, per-output status for waybar, `close` request and the `dwl-close` client |
| Scratchpads | `2e18c8c`, `efa93c1`, `6b0af07` | Local, based on julmajustus' `simple_scratchpad` patch | Three slots, configurable centered size, slot selection, IPC-aware |
| Window swallowing | `33f357d`, `e77d1a1` | Local | Hide a terminal when it spawns a GUI child and restore it afterwards |
| Tablet support | `10dea62` | Local | Tablet v2 pad/buttons and runtime pen-to-window mapping (`toggletabletmap`) |
| Float/stack helpers | `a6172d6`, `2e18c8c` | Local | `resizefloat`, `movestack`, `togglefocusfloat`; XWayland enabled in `config.mk` |
| Reproducible build | this change | Local | `config.local.h`, `PKGBUILD`, `scripts/build-check.sh`, this documentation |

### Window swallowing

When a managed client maps, its PID is read and its `/proc` PPid ancestry is
walked (up to `swallow_max_depth`). The first already-mapped ancestor whose
app-id is in `swallow_allowlist` (currently `foot` and `footclient`) is hidden;
the child inherits its monitor, tags and tiling slot. The Xwayland server PID
is skipped so unrelated X11 clients are never treated as terminals. When the
child unmaps or dies the parent is restored exactly, including its foreign
toplevel handle. Swallowed parents are excluded from `VISIBLEON()` and from
IPC client counts, and the IPC identifier is shown as `terminal->child`.

- `swallow_enabled` is the master switch in `config.local.h`.
- `MOD+Ctrl+X` (`toggleswallow`) breaks or re-arms the pairing for the focused
  window; a manually suspended pair keeps its pairing so the toggle reverses.
- Scratchpad toggles act on the visible child of a swallowed pair.

### IPC close (`dwl-close`)

The IPC patch exposes `zdwl_ipc_manager_v2`/`zdwl_ipc_output_v2` for status
bars. Manager version 3 adds a `close` request; `dwl-close` connects to the
running compositor, finds the manager and asks dwl to close the focused
client. It is used by the waybar close button. `make` builds both `dwl` and
`dwl-close`, and `make install` installs both.

### Scratchpads

`SCRATCHPAD_COUNT` (3) slots hold floating clients registered by app-id rules
(`sp-math` maps to slot 0 in `config.local.h`). A shown scratchpad is centered
and sized to `scratchpad_width_fraction` x `scratchpad_height_fraction` of the
monitor, so it does not inherit the terminal's default size.

- `MOD+Tab` (`togglescratchpad`) shows/hides the selected slot.
- `MOD+Shift+Tab` (`addscratchpad`) moves the focused client into the slot.
- `MOD+Alt+1` .. `MOD+Alt+3` (`setscratchpad`) select the active slot.
  `MODKEY` is `LOGO` here, so `MOD+Alt` is used instead of the upstream
  `MOD+z`-style binding to avoid colliding with `TAGKEYS`.
- `removescratchpad` unbinds a client from its slot.

The startup spawn for the `sp-math` scratchpad lives in `dwl-autostart` (outside
this repository), not in dwl: spawning `footclient` before the shared
`foot --server` socket exists makes it die at startup.

### Tablet support

Tablet pad buttons and stylus buttons are configurable through
`tabletbuttons`/`tabletpadbuttons` in the configuration. `MOD+t`
(`toggletabletmap`) switches at runtime between mapping the pen to the whole
monitor and confining it to the focused window; `tablet_area` sets the active
tablet region. The express-key binding used here also spawns the external
`handwrite-input-client` overlay.

## Getting Started:

### Latest semi-stable [release]
This is probably where you want to start. This builds against the [wlroots]
versions currently shipping in major distributions. If your
distribution's `wlroots` version is older, use an earlier dwl [release].
The `wlroots` version against which a given `dwl` release builds is specified
with each release on the [release] page

### Development branch [main]
Active development progresses on the `main` branch. The `main` branch is built
against the latest release of [wlroots]. PRs should target this branch unless they
depend on functionality that is not in the current release of `wlroots`.

### Preview branch [wlroots-next]
The `wlroots-next` branch is built against the git version of [wlroots], which
is unstable and changes frequently. PRs requiring functionality from the git
version of `wlroots` should target this branch.

### Building dwl
dwl has the following dependencies:
- libinput
- wayland
- wlroots (compiled with the libinput backend)
- xkbcommon
- pixman
- wayland-protocols (compile-time only)
- pkg-config (compile-time only)

On Arch Linux the corresponding packages are `libinput`, `wayland`,
`libxkbcommon`, `pixman`, `libxcb`, `xcb-util-wm` and `wlroots0.20`, with
`pkgconf` and `wayland-protocols` as build dependencies. `fcft` is **not**
used by dwl (it belongs to the status bar, not the compositor) and is not
pulled in by the Makefile.

dwl has the following additional dependencies if XWayland support is enabled:
- libxcb
- libxcb-wm
- wlroots (compiled with X11 support)
- Xwayland (runtime only)

Install these (and their `-devel` versions if your distro has separate
development packages) and run `make`. If you wish to build against a released
version of wlroots (*you probably do*), use a [release] or a [0.x branch]. If
you want to use the unstable development `main` branch, you need to use the git
version of [wlroots].

To enable XWayland, you should uncomment its flags in `config.mk`.

## Configuration

All configuration is done by editing `config.h` and recompiling, in the same
manner as [dwm]. There is no way to separately restart the window manager in
Wayland without restarting the entire display server, so any changes will take
effect the next time dwl is executed.

On this fork `config.h` is generated from the tracked `config.local.h` when it
does not exist; edit `config.h` for the live machine and copy it back to
`config.local.h` when the change should be reproducible (see
[Reproducible configuration](#reproducible-configuration)).

As in the [dwm] community, we encourage users to share patches they have
created. Check out the [dwl-patches] repository!

## Running dwl

dwl can be run on any of the backends supported by wlroots. This means you can
run it as a separate window inside either an X11 or Wayland session, as well as
directly from a VT console. Depending on your distro's setup, you may need to
add your user to the `video` and `input` groups before you can run dwl on a
VT. If you are using `elogind` or `systemd-logind` you need to install polkit;
otherwise you need to add yourself in the `seat` group and enable/start the
seatd daemon.

When dwl is run with no arguments, it will launch the server and begin handling
any shortcuts configured in `config.h`. There is no status bar or other
decoration initially; these are instead clients that can be run within the
Wayland session. Do note that the default background color is grey. This can be
modified in `config.h`.

If you would like to run a script or command automatically at startup, you can
specify the command using the `-s` option. This command will be executed as a
shell command using `/bin/sh -c`.  It serves a similar function to `.xinitrc`,
but differs in that the display server will not shut down when this process
terminates. Instead, dwl will send this process a SIGTERM at shutdown and wait
for it to terminate (if it hasn't already). This makes it ideal for execing into
a user service manager like [s6], [anopa], [runit], [dinit], or [`systemd
--user`].

Note: The `-s` command is run as a *child process* of dwl, which means that it
does not have the ability to affect the environment of dwl or of any processes
that it spawns. If you need to set environment variables that affect the entire
dwl session, these must be set prior to running dwl. For example, Wayland
requires a valid `XDG_RUNTIME_DIR`, which is usually set up by a session manager
such as `elogind` or `systemd-logind`.  If your system doesn't do this
automatically, you will need to configure it prior to launching `dwl`, e.g.:

    export XDG_RUNTIME_DIR=/tmp/xdg-runtime-$(id -u)
    mkdir -p $XDG_RUNTIME_DIR
    dwl

### Status information

Information about selected layouts, current window title, app-id, and
selected/occupied/urgent tags is written to the stdin of the `-s` command (see
the `STATUS INFORMATION` section in `_dwl_(1)`).  This information can be used to
populate an external status bar with a script that parses the
information. Failing to read this information will cause dwl to block, so if you
do want to run a startup command that does not consume the status information,
you can close standard input with the `<&-` shell redirection, for example:

    dwl -s 'foot --server <&-'

If your startup command is a shell script, you can achieve the same inside the
script with the line

    exec <&-

To get a list of status bars that work with dwl consult our [wiki].

### (Known) Java nonreparenting WM issue
Certain IDEs don't display correctly unless an environmental variable for Java AWT
indicates that the WM is nonreparenting.

For some Java AWT-based IDEs, such as Xilinx Vivado and Microchip MPLAB X, the
following environment variable needs to be set before running the IDE or dwl:

    export _JAVA_AWT_WM_NONREPARENTING=1

## Packaging and build checks

### PKGBUILD

`PKGBUILD` builds the checked-out `local/main` tree and installs `dwl`,
`dwl-close`, `dwl.1` and `dwl.desktop` under `/usr`:

    makepkg -si

It has no remote sources, so it packages the current working tree (including
uncommitted edits) and needs no network access. Run it from the repository
root; the build generates `config.h` from the tracked `config.local.h` because
the checkout does not contain one.

The package dependencies are the pkg-config modules the Makefile actually
queries:

| pkg-config module | Arch package | Use |
| --- | --- | --- |
| `wlroots-0.20` | `wlroots0.20` | wlroots compositor library |
| `wayland-server` | `wayland` | server API |
| `xkbcommon` | `libxkbcommon` | keymaps |
| `libinput` | `libinput` | input devices |
| `pixman-1` | `pixman` | rendering library used by wlroots |
| `xcb`, `xcb-icccm` | `libxcb`, `xcb-util-wm` | XWayland support (enabled in `config.mk`) |
| `wayland-scanner` | `wayland` | protocol code generation |
| `wayland-protocols` | `wayland-protocols` | protocol XML (build only) |
| pkg-config | `pkgconf` | build only |

### Build check

    scripts/build-check.sh

The script copies the tracked files plus other unignored source files (with
uncommitted working-tree edits, but never the gitignored `config.h`) into a
temporary directory, runs `make` there with the same toolchain, and verifies
that `config.h` was generated from `config.local.h`. It never installs,
restarts or modifies anything in the repository.

The same check can run in a disposable Arch container without secrets or host
state:

    podman run --rm -v "$PWD:/src:ro" -w /src archlinux:base-devel sh -c '
        pacman -Syu --noconfirm --needed git pkgconf wayland-protocols \
            wayland libinput libxkbcommon pixman libxcb xcb-util-wm wlroots0.20 &&
        git config --global --add safe.directory /src &&
        scripts/build-check.sh'

## Replacements for X applications

You can find a [list of useful resources on our wiki].

## Background

dwl is not meant to provide every feature under the sun. Instead, like [dwm], it
sticks to features which are necessary, simple, and straightforward to implement
given the base on which it is built. Implemented default features are:

- Any features provided by [dwm]/Xlib: simple window borders, tags, keybindings,
  client rules, mouse move/resize. Providing a built-in status bar is an
  exception to this goal, to avoid dependencies on font rendering and/or drawing
  libraries when an external bar could work well.
- Configurable multi-monitor layout support, including position and rotation
- Configurable HiDPI/multi-DPI support
- Idle-inhibit protocol which lets applications such as mpv disable idle
  monitoring
- Provide information to external status bars via stdout/stdin
- Urgency hints via xdg-activate protocol
- Support screen lockers via ext-session-lock-v1 protocol
- Various Wayland protocols
- XWayland support as provided by wlroots (can be enabled in `config.mk`)
- Zero flickering - Wayland users naturally expect that "every frame is perfect"
- Layer shell popups (used by Waybar)
- Damage tracking provided by scenegraph API

Given the Wayland architecture, dwl has to implement features from [dwm] **and**
the xorg-server. Because of this, it is impossible to maintain the original
project goal of 2000 SLOC and have a reasonably complete compositor with
features comparable to [dwm]. However, this does not mean that the code will grow
indiscriminately. We will try to keep the code as small as possible.

Features under consideration (possibly as patches) are:

- Protocols made trivial by wlroots

Feature *non-goals* for the main codebase include:

- Client-side decoration (any more than is necessary to tell the clients not to)
- Client-initiated window management, such as move, resize, and close, which can
  be done through the compositor
- Animations and visual effects

## Acknowledgements

dwl began by extending the TinyWL example provided (CC0) by the sway/wlroots
developers. This was made possible in many cases by looking at how sway
accomplished something, then trying to do the same in as suckless a way as
possible.

Many thanks to suckless.org and the [dwm] developers and community for the
inspiration, and to the various contributors to the project, including:

- **Devin J. Pohly for creating and nurturing the fledgling project**
- Alexander Courtis for the XWayland implementation
- Guido Cella for the layer-shell protocol implementation, patch maintenance,
  and for helping to keep the project running
- Stivvo for output management and fullscreen support, and patch maintenance

Also, thanks to our current lead developers:

- @thanatos
- @guidocella

[wlroots]: https://gitlab.freedesktop.org/wlroots
[dwm]: https://dwm.suckless.org/
[`systemd --user`]: https://wiki.archlinux.org/title/Systemd/User
[#dwl on Libera Chat]: https://web.libera.chat/?channels=#dwl
[0.7-rc1]: https://codeberg.org/dwl/dwl/releases/tag/v0.7-rc1
[0.x branch]: https://codeberg.org/dwl/dwl/branches
[anopa]: https://jjacky.com/anopa/
[dinit]: https://davmac.org/projects/dinit/
[dwl-patches]: https://codeberg.org/dwl/dwl-patches
[list of useful resources on our wiki]: https://codeberg.org/dwl/dwl/wiki/Home#migrating-from-x
[main]: https://codeberg.org/dwl/dwl/src/branch/main
[wlroots-next]: https://codeberg.org/dwl/dwl/src/branch/wlroots-next
[release]: https://codeberg.org/dwl/dwl/releases
[runit]: http://smarden.org/runit/faq.html#userservices
[s6]: https://skarnet.org/software/s6/
[wlroots]: https://gitlab.freedesktop.org/wlroots/wlroots/
[wiki]: https://codeberg.org/dwl/dwl/wiki/Home#compatible-status-bars
[Discord server]: https://discord.gg/jJxZnrGPWN
[Wayland]: https://wayland.freedesktop.org/
[#dwl-official:matrix.org]: https://matrix.to/#/#dwl-official:matrix.org
