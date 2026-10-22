/* ************************************************************************** */
/*                                                 @@@            @@@@@@@@    */
/*                                                  @@@          @@@@@@@@@@   */
/*                                                   @@!         @@!   @@@@   */
/*                                                    !@!        !@!  @!@!@   */
/*   simple_scratchpad.h                               @!!       @!@ @! !@!   */
/*                                                      !!!      !@!!!  !!!   */
/*   By: julmajustus <julmajustus@tutanota.com>          !!:     !!:!   !!!   */
/*                                                        ::!    :!:    !:!   */
/*   Created: 2024/12/19 19:35:02 by jmakkone           ::    ::::::: ::   */
/*   Updated: 2026/09/20 02:14:02 by julmajustus            : :   : : :  :    */
/*                                                                            */
/* ************************************************************************** */

static struct wl_list scratchpad_clients[SCRATCHPAD_COUNT];
static int scratchpad_visible[SCRATCHPAD_COUNT];
static int scratchpad_sel = 0;

/* Center a scratchpad client on the monitor and size it to a fraction of the
 * usable area, so it does not open at the terminal's small default size. */
static void
scratchpadplace(Client *c, Monitor *m)
{
	int w = (int)(m->w.width * scratchpad_width_fraction);
	int h = (int)(m->w.height * scratchpad_height_fraction);

	if (w <= 0 || h <= 0)
		return;
	resize(c, (struct wlr_box){
		.x = m->w.x + (m->w.width - w) / 2,
		.y = m->w.y + (m->w.height - h) / 2,
		.width = w,
		.height = h,
	}, 0);
}

void
addscratchpad(const Arg *arg)
{
	Client *cc, *c = focustop(selmon);

	if (!c)
		return;
	/* Check if the added client is already a scratchpad client */
	for (int i = 0; i < SCRATCHPAD_COUNT; i++) {
		wl_list_for_each(cc, &scratchpad_clients[i], link_temp) {
			if (cc == c)
				return;
		}
	}
	if (!c->isfloating) {
		setfloating(c, 1);
	}
	wl_list_insert(&scratchpad_clients[scratchpad_sel], &c->link_temp);
}

void
togglescratchpad(const Arg *arg)
{
	Client *c, *last = NULL;
	Monitor *m = selmon;

	if (!m)
		return;
	scratchpad_visible[scratchpad_sel] = !scratchpad_visible[scratchpad_sel];
	if (scratchpad_visible[scratchpad_sel]) {
		/* Show every client in the slot on the current tagset, then focus
		 * the most recently added one once, after the loop. */
		wl_list_for_each(c, &scratchpad_clients[scratchpad_sel], link_temp) {
			/* If this scratchpad client is swallowed, the visible one is
			 * its child: show the child, leave the hidden parent alone. */
			Client *target = (c->isswallowed && c->swallowedby) ? c->swallowedby : c;
			target->mon = m;
			target->tags = m->tagset[m->seltags];
			if (target == c)
				scratchpadplace(c, m);
			else
				c->mon = m; /* keep the hidden parent on the same monitor */
			last = target;
		}
		arrange(m);
		/* Empty slot: keep current focus instead of clearing it */
		if (last)
			focusclient(last, NULL, 1);
	} else {
		wl_list_for_each(c, &scratchpad_clients[scratchpad_sel], link_temp) {
			/* Hide the visible client: the swallowed child if there is one. */
			Client *target = (c->isswallowed && c->swallowedby) ? c->swallowedby : c;
			target->tags = 0;
		}
		arrange(m);
		focusclient(focustop(m), NULL, 1);
	}
	printstatus();
}

void
removescratchpad(const Arg *arg)
{
	Client *sc, *c = focustop(selmon);
	if (!c)
		return;
	for (int i = 0; i < SCRATCHPAD_COUNT; i++) {
		/* Check if c is in scratchpad_clients */
		wl_list_for_each(sc, &scratchpad_clients[i], link_temp) {
			if (sc == c) {
				wl_list_remove(&c->link_temp);
				return;
			}
		}
	}
}

void
setscratchpad(const Arg *arg)
{
	int idx = arg->i;

	if (idx < 0)
		idx = 0;
	else if (idx >= SCRATCHPAD_COUNT)
		idx = SCRATCHPAD_COUNT - 1;
	scratchpad_sel = idx;
}
