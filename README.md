# dgcf-hacktools

Data extraction/injection tools that could be used in a translation of the PC visual novel "Di Gi Charat Fantasy".

Massively incomplete at the moment, please be patient.

Tool compilation prerequisites are zlib and libpng.

You need the [flat assembler](https://flatassembler.net/) to assemble the patched game executable.

You also need to copy over the unmodified files from an installed game to a folder called `DFantasy` (SHA-1 checksums for the files are in `sha1sums.txt`).

## To-do

- Tool improvements
  - Make packing modified files more convenient. Some kind of pack list instead of just packing everything in a dir
- Game hacking
  - Support ASCII dialogue in scripts, possibly with variable-width fonts
  - Use on-disk audio files instead of CD-DA BGM