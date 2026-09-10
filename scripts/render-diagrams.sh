#!/usr/bin/env bash
# scripts/render-diagrams.sh
# Renders the SVG diagram masters (assets/diagrams/*.svg) to the checked-in
# transparent PNGs the frontends bundle (assets/diagrams/png/*.png, 1280 px
# wide = 2x of the ~640 px logical size).
#
# The PNGs are generated artifacts but ARE checked in, so consumers and CI
# never need librsvg. Re-run only when a master changes, and commit the
# regenerated PNGs in the same change set.
#
# Prerequisite: rsvg-convert (brew install librsvg / apt install librsvg2-bin)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/assets/diagrams"
OUT="$SRC/png"

command -v rsvg-convert >/dev/null || {
    echo "error: rsvg-convert not found (brew install librsvg)" >&2; exit 1; }

mkdir -p "$OUT"
shopt -s nullglob
count=0
for svg in "$SRC"/*.svg; do
    name="$(basename "${svg%.svg}")"
    rsvg-convert -w 1280 "$svg" -o "$OUT/$name.png"
    count=$((count + 1))
done
echo "rendered $count diagram(s) to $OUT"

# Typeset formula snippets (assets/formulas/*.svg → assets/formulas/png/).
# Formulas vary in aspect ratio, so render at a fixed 2x zoom rather than a
# fixed width; frontends show them at half the pixel size (2x/retina).
FSRC="$REPO_ROOT/assets/formulas"
FOUT="$FSRC/png"
mkdir -p "$FOUT"
fcount=0
for svg in "$FSRC"/*.svg; do
    name="$(basename "${svg%.svg}")"
    rsvg-convert --zoom 2 "$svg" -o "$FOUT/$name.png"
    fcount=$((fcount + 1))
done
echo "rendered $fcount formula(s) to $FOUT"
