# Maintainer: tanmay <tanmay@localhost>
#
# PKGBUILD for this fork. It packages the checked-out local/main tree in this
# directory (no remote sources), so uncommitted working-tree changes are
# included and no network access is needed.
#
#   makepkg -si        # build and install (deps from the official repos)
#   makepkg            # build only, leaves dwl-tanmay-git-*.pkg.tar.zst
#   makepkg --nodeps   # deps already installed
#
# Run it from the repository root. The Makefile generates config.h from the
# tracked config.local.h because a checkout has no config.h.
#
# Package names below target Arch Linux (extra): wlroots 0.20 is packaged as
# wlroots0.20, libxkbcommon as libxkbcommon and xcb-icccm as xcb-util-wm.

pkgname=dwl-tanmay-git
pkgver=r$(git -C "$startdir" rev-list --count HEAD).$(git -C "$startdir" rev-parse --short HEAD)
pkgrel=1
pkgdesc="dwl fork (local/main): wlroots 0.20, IPC close, scratchpads, window swallowing, tablet mapping"
arch=('x86_64')
url="https://github.com/tanmayd-dev/dwl"
license=('MIT')
depends=(
	'wlroots0.20'
	'wayland'
	'libinput'
	'libxkbcommon'
	'pixman'
	'libxcb'
	'xcb-util-wm'
)
makedepends=('git' 'pkgconf' 'wayland-protocols')
provides=('dwl')
conflicts=('dwl' 'dwl-git')
optdepends=('xorg-xwayland: XWayland support (enabled in config.mk)')
source=()
sha256sums=()

build() {
	cd "$startdir"
	make
}

package() {
	cd "$startdir"
	make install \
		DESTDIR="$pkgdir" \
		PREFIX=/usr \
		MANDIR=/usr/share/man \
		DATADIR=/usr/share
	install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
