/* Taken from https://github.com/djpohly/dwl/issues/466 */
#define COLOR(hex)    { ((hex >> 24) & 0xFF) / 255.0f, \
                        ((hex >> 16) & 0xFF) / 255.0f, \
                        ((hex >> 8) & 0xFF) / 255.0f, \
                        (hex & 0xFF) / 255.0f }
/* appearance */
/* Tablet pen mapping. Runtime-toggled by the toggletabletmap keybind (MODKEY+t).
 * 0 = pen maps to the whole monitor (can reach the bar), 1 = pen is confined
 * to the focused window's surface (area fits the window; better for drawing). */
static int tabletmaptosurface              = 0;
static const int sloppyfocus               = 1;  /* focus follows mouse */
static const int bypass_surface_visibility = 0;  /* 1 means idle inhibitors will disable idle tracking even if it's surface isn't visible  */
static const unsigned int borderpx         = 1;  /* border pixel of windows */
static const unsigned int snap             = 32; /* snap pixel */
static const float rootcolor[]             = COLOR(0x222222ff);
static const float bordercolor[]           = COLOR(0x444444ff);
static const float focuscolor[]            = COLOR(0x005577ff);
static const float urgentcolor[]           = COLOR(0xff0000ff);
/* This conforms to the xdg-protocol. Set the alpha to zero to restore the old behavior */
static const float fullscreen_bg[]         = {0.0f, 0.0f, 0.0f, 1.0f}; /* You can also use glsl colors */
/* Pen active area: value 0.0 => 1.0         { x,    y,    width, height } */
static const double tablet_area[4]         = { 0.0,  0.0,  1.0,   1.0 }; /* whole tablet */

/* tagging - TAGCOUNT must be no greater than 31 */
#define TAGCOUNT (9)

/* scratchpad - number of scratchpad slots */
#define SCRATCHPAD_COUNT 3
/* Scratchpad size as a fraction of the monitor's usable area. The window is
 * centered on show; set both to 1.0f for a fullscreen-sized scratchpad. */
static const float scratchpad_width_fraction  = 0.7f;
static const float scratchpad_height_fraction = 0.75f;

/* Window swallowing */
static const int  swallow_enabled   = 1;     /* master toggle */
static const int  swallow_max_depth = 24;    /* max PPid-walk depth */
static const char *swallow_allowlist[] = {   /* terminal app_ids eligible for swallowing */
	"foot", "footclient",
};

/* logging */
static int log_level = WLR_ERROR;

static const Rule rules[] = {
	/* app_id             title       tags mask     isfloating   monitor   scratchpad (0 = none, else slot+1) */
	{ "Gimp_EXAMPLE",     NULL,       0,            1,           -1,       0 }, /* Start on currently visible tags floating, not tiled */
	{ "firefox_EXAMPLE",  NULL,       1 << 8,       0,           -1,       0 }, /* Start on ONLY tag "9" */
	{ "sp-math",          NULL,       0,            1,           -1,       1 }, /* Default scratchpad slot 0: terminal running mathypad, hidden */
    /* default/example rule: can be changed but cannot be eliminated; at least one rule must exist */
};

/* layout(s) */
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },
};

/* monitors */
/* (x=-1, y=-1) is reserved as an "autoconfigure" monitor position indicator
 * WARNING: negative values other than (-1, -1) cause problems with Xwayland clients due to
 * https://gitlab.freedesktop.org/xorg/xserver/-/issues/899 */
static const MonitorRule monrules[] = {
   /* name        mfact  nmaster scale layout       rotate/reflect                x    y
    * example of a HiDPI laptop monitor:
    { "eDP-1",    0.5f,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 }, */
	{ NULL,       0.55f, 1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
	/* default monitor rule: can be changed but cannot be eliminated; at least one monitor rule must exist */
};

/* keyboard */
static const struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
	.options = NULL,
};

static const int repeat_rate = 25;
static const int repeat_delay = 600;

/* Trackpad */
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int natural_scrolling = 1;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int middle_button_emulation = 0;
/* You can choose between:
LIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;

/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* If you want to use the windows key for MODKEY, use WLR_MODIFIER_LOGO */
#define MODKEY WLR_MODIFIER_LOGO

#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_SHIFT, KEY,            tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,KEY,toggletag,  {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* Terminal used by every keybind below that opens one.
 * Swap it here once and all spawns follow. dwl-autostart runs `foot
 * --server', so this is `footclient' and every spawn reuses the running
 * daemon instead of forking a fresh instance - noticeably faster window
 * opens. foot notes: the command is passed as plain argv after the options
 * (no need for alacritty's "-e"), and "--class" from alacritty becomes
 * "--app-id" on Wayland. */
#define TERMINAL "footclient"

/* commands */
static const char *termcmd[] = { TERMINAL, NULL };
static const char *menucmd[] = { "fuzzel", NULL };
/* Toggle the handwrite-input overlay (see ~/.local/src/handwrite-input). */
static const char *handwritecmd[] = { "handwrite-input-client", "toggle", NULL };
/* Default scratchpad slot 0: terminal running mathypad.
 * Its app_id ("sp-math") is matched by a rule above that
 * assigns it to scratchpad slot 0, hidden. */
static const char *scratchpadcmd[] = { TERMINAL, "--app-id", "sp-math", "mathypad", NULL };

static const Key keys[] = {
	/* modifier                  key                  function          argument */

	/* --- A --- */
	/* { MODKEY,                    XKB_KEY_a,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_a,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_a,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_a, ... }, */

	/* --- B --- */
	/* { MODKEY,                    XKB_KEY_b,           togglebar,        {0} }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_b,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_b,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_b, ... }, */

	/* --- C --- */
	/* { MODKEY,                    XKB_KEY_c,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_c,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_c,           killclient,       {0} }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_c, ... }, */

	/* --- D --- */
	/* { MODKEY,                    XKB_KEY_d,           incnmaster,       {.i = -1} }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_d,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_d,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_d, ... }, */

	/* --- E --- */
	{ MODKEY,                    XKB_KEY_e,           spawn,            SHCMD("emacsclient -c -a ''") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_e,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_e,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_e, ... }, */

	/* --- F --- */
	{ MODKEY,                    XKB_KEY_f,           togglefullscreen, {0} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_f,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_f,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_f, ... }, */

	/* --- G --- */
	/* { MODKEY,                    XKB_KEY_g,           zoom,             {0} }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_g,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_g,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_g, ... }, */

	/* --- H --- */
	{ MODKEY,                    XKB_KEY_h,           setmfact,         {.f = -0.05f} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_h,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_h,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_h, ... }, */

	/* --- I --- */
	{ MODKEY,                    XKB_KEY_i,           spawn,            SHCMD(TERMINAL " nmtui") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_i,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_i,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_i, ... }, */

	/* --- J --- */
	{ MODKEY,                    XKB_KEY_j,           focusstack,       {.i = +1} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_j,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_j,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_j, ... }, */

	/* --- K --- */
	{ MODKEY,                    XKB_KEY_k,           focusstack,       {.i = -1} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_k,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_k,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_k, ... }, */

	/* --- L --- */
	{ MODKEY,                    XKB_KEY_l,           setmfact,         {.f = +0.05f} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_l,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_l,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_l, ... }, */

	/* --- M --- */
	/* { MODKEY,                    XKB_KEY_m,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_m,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_m,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_m, ... }, */

	/* --- N --- */
	/* { MODKEY,                    XKB_KEY_n,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_n,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_n,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_n, ... }, */

	/* --- O --- */
	{ MODKEY,                    XKB_KEY_o,           spawn,            SHCMD(TERMINAL " --working-directory $HOME/.local/src opencode $HOME/.local/src") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_o,           ... }, */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_o,           spawn,            SHCMD("$HOME/.local/bin/oc-select") },
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_o, ... }, */

	/* --- P --- */
	{ MODKEY,                    XKB_KEY_p,           spawn,            SHCMD("playerctl play-pause") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_p,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_p,           spawn,            SHCMD("$HOME/.local/bin/cc-toggle") }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_p, ... }, */

	/* --- Q --- */
	{ MODKEY,                    XKB_KEY_q,           killclient,       {0} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_q,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_q,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_q, ... }, */

	/* --- R --- */
	/* { MODKEY,                    XKB_KEY_r,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_r,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_r,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_r, ... }, */

	/* --- S --- */
	{ MODKEY,                    XKB_KEY_s,           spawn,            SHCMD("$HOME/.local/bin/screenshot region") },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_s,           spawn,            SHCMD("$HOME/.local/bin/screenshot window") },
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_s, spawn, SHCMD("$HOME/.local/bin/screenshot screen") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_s,           ... }, */

	/* --- T --- */
	{ MODKEY,                    XKB_KEY_t,           toggletabletmap,  {0} },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_t,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_t,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_t, ... }, */

	/* --- U --- */
	/* { MODKEY,                    XKB_KEY_u,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_u,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_u,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_u, ... }, */

	/* --- V --- */
	{ MODKEY,                    XKB_KEY_v,           spawn,            SHCMD("$HOME/.local/bin/clipmenu") },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_v,           spawn,            SHCMD("$HOME/.local/bin/clipmenu delete") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_v,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_v, ... }, */

	/* --- W --- */
	{ MODKEY,                    XKB_KEY_w,           spawn,            SHCMD("zen-browser") },
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_w,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_w,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_w, ... }, */

	/* --- X --- */
	/* { MODKEY,                    XKB_KEY_x,           ... }, */
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_x,           toggleswallow,    {0} },
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_x,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_x, ... }, */

	/* --- Y --- */
	/* { MODKEY,                    XKB_KEY_y,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_y,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_y,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_y, ... }, */

	/* --- Z --- */
	/* { MODKEY,                    XKB_KEY_z,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_z,           ... }, */
	/* { MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_z,           ... }, */
	/* { MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT, XKB_KEY_z, ... }, */

	/* --- Modkey Other Keys --- */
	{ MODKEY,                    XKB_KEY_Return,      spawn,            {.v = termcmd} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Return,      spawn,            {.v = menucmd} },
	{ MODKEY,                    XKB_KEY_BackSpace,   spawn,            SHCMD("$HOME/.local/bin/sysmenu") },
	{ MODKEY,                    XKB_KEY_space,       togglefocusfloat, {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space,       togglefloating,   {0} },
	/* { MODKEY,                    XKB_KEY_Tab,         view,             {0} }, */
	{ MODKEY,                    XKB_KEY_comma,       focusmon,         {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY,                    XKB_KEY_period,      focusmon,         {.i = WLR_DIRECTION_RIGHT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_comma,       tagmon,           {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_period,      tagmon,           {.i = WLR_DIRECTION_RIGHT} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_BackSpace, killclient, {0} },

	/* --- Floating window resize (Super+Ctrl+Arrows) --- */
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Left,        resizefloat,      {.i = 0} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Right,       resizefloat,      {.i = 1} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Up,          resizefloat,      {.i = 2} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Down,        resizefloat,      {.i = 3} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_h,           resizefloat,      {.i = 0} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_l,           resizefloat,      {.i = 1} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_k,           resizefloat,      {.i = 2} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_j,           resizefloat,      {.i = 3} },

	/* --- Move window up/down the stack (Super+Shift+Arrows) --- */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Up,          movestack,        {.i = -1} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Down,        movestack,        {.i = +1} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_k,           movestack,        {.i = -1} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_j,           movestack,        {.i = +1} },

	/* --- Numbers & Tags --- */
	{ MODKEY,                    XKB_KEY_0,           view,             {.ui = ~0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_0,           tag,              {.ui = ~0} },
	TAGKEYS(                     XKB_KEY_1,           0),
	TAGKEYS(                     XKB_KEY_2,           1),
	TAGKEYS(                     XKB_KEY_3,           2),
	TAGKEYS(                     XKB_KEY_4,           3),
	TAGKEYS(                     XKB_KEY_5,           4),
	TAGKEYS(                     XKB_KEY_6,           5),
	TAGKEYS(                     XKB_KEY_7,           6),
	TAGKEYS(                     XKB_KEY_8,           7),
	TAGKEYS(                     XKB_KEY_9,           8),

	/* --- Scratchpad slot selection --- */
	/* TAGKEYS already claims MODKEY with and without CTRL/SHIFT for 1-9,
	 * and MODKEY is WLR_MODIFIER_LOGO, so MODKEY|WLR_MODIFIER_LOGO would
	 * collapse to LOGO|LOGO. Use MODKEY|ALT to pick the active slot. */
	{ MODKEY|WLR_MODIFIER_ALT,  XKB_KEY_1,           setscratchpad,    {.i = 0} },
	{ MODKEY|WLR_MODIFIER_ALT,  XKB_KEY_2,           setscratchpad,    {.i = 1} },
	{ MODKEY|WLR_MODIFIER_ALT,  XKB_KEY_3,           setscratchpad,    {.i = 2} },

	/* scratchpad */
	{ MODKEY,                    XKB_KEY_Tab,         togglescratchpad, {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Tab,         addscratchpad,    {0} },

	/* --- Hardware / Media Keys --- */
	{ 0, XKB_KEY_XF86AudioRaiseVolume, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+") },
	{ 0, XKB_KEY_XF86AudioLowerVolume, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-") },
	{ 0, XKB_KEY_XF86AudioMute,        spawn, SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle") },
	{ 0, XKB_KEY_XF86MonBrightnessUp,   spawn, SHCMD("brightnessctl set +5%") },
	{ 0, XKB_KEY_XF86MonBrightnessDown, spawn, SHCMD("brightnessctl set 5%-") },
	{ MODKEY,                    XKB_KEY_minus,       spawn,            SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-") },
	{ MODKEY,                    XKB_KEY_equal,       spawn,            SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+") },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_minus,       spawn,            SHCMD("brightnessctl set 5%-") },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_equal,       spawn,            SHCMD("brightnessctl set +5%") },

	/* --- Typing shortcuts (needs wtype) --- */
	{ WLR_MODIFIER_ALT,          XKB_KEY_minus,       spawn,            SHCMD("wtype —") },

	/* Ctrl-Alt-Fx used to switch VT, see below (no quit binding) */
	/* Ctrl-Alt-Fx is used to switch to another VT, if you don't know what a VT is
	 * do not remove them.
	 */
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_F##n, chvt, {.ui = (n)} }
	CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
	CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
};

static const Button buttons[] = {
	{ MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
	{ MODKEY|WLR_MODIFIER_SHIFT, BTN_LEFT, moveresize, {.ui = CurResize} },
	/* { MODKEY, BTN_MIDDLE, togglefloating, {0} }, */
	{ MODKEY, BTN_RIGHT,  moveresize,     {.ui = CurResize} },
};

static const Axis axes[] = {
	/* example of volume control:
	{ MODKEY, AxisUp,   spawn, SHCMD("volume-up_EXAMPLE") },
	{ MODKEY, AxisDown, spawn, SHCMD("volume-down_EXAMPLE") }, */
	{ 0, 0, NULL, {0} },
	/* does nothing, but the array cannot be empty */
};

static const TabletButton tabletbuttons[] = {
	/* button       state                  function    argument */
/*	{ BTN_STYLUS,   WLR_BUTTON_PRESSED,    spawn,      {.v = termcmd} }, */
/*	{ BTN_STYLUS2,  WLR_BUTTON_PRESSED,    killclient, {0} }, */
	{ 0,            0,                     NULL,       {0} }
};

static const TabletPadButton tabletpadbuttons[] = {
	/* button  state                  function          argument */
	/* libinput numbers pad buttons from 0, so the tablet's four express
	 * keys (labelled 1-4) are 0-3 here. */
	{ 2,       WLR_BUTTON_PRESSED,    toggletabletmap,  {0} },  /* button 3 */
	{ 3,       WLR_BUTTON_PRESSED,    spawn,            {.v = handwritecmd} },  /* button 4 */
	{ 0,            0,                     NULL,       {0} }
};
