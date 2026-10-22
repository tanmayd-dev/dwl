/*
 * dwl-close - ask dwl to close the currently focused window.
 *
 * Uses the zdwl_ipc_manager_v2.close request from the dwl-ipc-unstable-v2
 * protocol (available since interface version 3). Intended to be called from
 * a waybar button.
 */
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

#include "dwl-ipc-unstable-v2-client-protocol.h"

static struct zdwl_ipc_manager_v2 *manager;

static void
registry_global(void *data, struct wl_registry *registry, uint32_t name,
		const char *interface, uint32_t version)
{
	if (strcmp(interface, zdwl_ipc_manager_v2_interface.name) == 0) {
		uint32_t v = version < 3 ? version : 3;
		manager = wl_registry_bind(registry, name,
				&zdwl_ipc_manager_v2_interface, v);
	}
}

static void
registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	(void)data;
	(void)registry;
	(void)name;
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

int
main(void)
{
	struct wl_display *display;
	struct wl_registry *registry;
	uint32_t version;

	display = wl_display_connect(NULL);
	if (!display) {
		fprintf(stderr, "dwl-close: failed to connect to wayland display\n");
		return 1;
	}

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (!manager) {
		fprintf(stderr, "dwl-close: dwl-ipc manager not found\n");
		wl_display_disconnect(display);
		return 1;
	}

	version = wl_proxy_get_version((struct wl_proxy *)manager);
	if (version < ZDWL_IPC_MANAGER_V2_CLOSE_SINCE_VERSION) {
		fprintf(stderr, "dwl-close: dwl-ipc manager v%u has no close request (need v3)\n",
				version);
		wl_display_disconnect(display);
		return 1;
	}

	zdwl_ipc_manager_v2_close(manager);
	wl_display_roundtrip(display);

	wl_display_disconnect(display);
	return 0;
}
