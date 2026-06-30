pkgname=nyxwm
pkgver=1.1.0
pkgrel=1
pkgdesc="A minimal, high performance tiling window manager for X11, written in C."
arch=('x86_64')
license=('MIT')
makedepends=('gcc' 'make' 'libx11' 'base-devel')
depends=('pamixer' 'brightnessctl' 'xorg' 'xorg-xinit')
optdepends=('alacritty' 'kitty')
source=()
sha256sums=()

build() {
    cd "$startdir"
    make NAME="$pkgname"
}

package() {
    cd "$startdir"
    install -Dm755 "build/bin/$pkgname" "$pkgdir/usr/bin/$pkgname"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
    install -Dm644 readme.md "$pkgdir/usr/share/docs/$pkgname/README.md"
    cp -r writeups "$pkgdir/usr/share/docs/$pkgname/writeups"
    echo "Run 'make purge' to clean makepkg residues..."
}
