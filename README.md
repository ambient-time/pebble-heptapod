# Heptapod for Pebble

A broken ring of ink tells local time. Three tendrils grow from their clock
positions: long for hours, medium for minutes, short for seconds. Twelve stays
at the top as the ring breathes and the brush tips sway.

Native C watchface, version 0.1.2. One PBW and UUID cover Time 2 (Emery),
2 SE (Diorite), Time/Time Steel (Basalt), 2 Duo (Flint), Time Round (Chalk), and
Round 2 (Gabbro). Emulator checks do not measure physical-watch battery use.
It runs offline without settings or external libraries. Animation updates once
per second.

[Pebble Appstore](https://apps.repebble.com/eed88192a6a24a229eceb50c) ·
[PBW](release/heptapod-0.1.2.pbw) · [Source ZIP](release/heptapod-0.1.2-source.zip)
· [Checksums](release/SHA256SUMS) · [Native capture sheet](evidence/contact-sheet.png)
· [Time 2 motion preview](evidence/emery-motion.gif) · [Source notes](reference/README.md)

Release files and native captures accompany the canonical project; the source
ZIP contains only the files needed to build, test, and reproduce those captures.
Version 0.1.1 corrects author metadata and adds portable packaging and natural
rollover checks. It keeps the renderer and UUID from 0.1.0. Version 0.1.2
removes the digital caption at Luke’s request and preserves the glyph layout.

## Adaptation

Source: Luke Steuber's supplied [Heptapod B clock](https://datapoems.io/clocks/heptapod/),
archived locally as [reference/source.html](reference/source.html). This port
keeps its six arcs, twelve ornamental flourishes, six ink blots, and the
proportions of all three time tendrils. Arrival inspired the design; it has no
official affiliation and does not translate the film's fictional language.

Native pressure-expanded ribbons form the brush outline. The port includes no
perfect-freehand code or film glyph assets. Color Pebbles use a light, dithered
vignette; monochrome uses black ink on white. The watch keeps the breathing,
swaying outline at 1 Hz and leaves out the browser's blurred smoke. Holding
the glyph upright keeps twelve in a fixed position. These changes adapt the
browser design to the watch's display and update rate.

Geometry has no hidden clock state. A caller-owned workspace avoids large
stack arrays, firmware trig replaces desktop libm, and small sampled pressure
curves preserve the original taper. The host tests check approximation error,
9,344 time/shape samples, motion with atmosphere frozen, and cardinal angles;
a second run exercises fixed-point angle quantization.
Both runs use AddressSanitizer and UndefinedBehaviorSanitizer.

## Reproduce

Tested toolchain: Pebble Tool 5.0.39 / SDK 4.33.1, a C compiler with sanitizers,
and the Python environment used by pebble-tool (includes `libpebble2` and
`pypng`). Set `PYTHON` if the shell's `python3` uses another environment.
From this directory, including after unpacking the source ZIP into a new folder:

```sh
make build PYTHON=python3
python3 test/package_test.py
python3 test/emulator.py emery --fresh --record 16
python3 test/emulator.py diorite --fresh
python3 test/emulator.py chalk --fresh
python3 test/emulator.py gabbro --fresh
python3 test/emulator.py basalt --fresh
python3 test/emulator.py flint --fresh
make package PYTHON=python3
```

`--fresh` creates a new isolated profile without erasing any existing flash.
The harness installs the production PBW, waits for logged confirmation of
each requested time, verifies second-to-second image changes, observes natural
noon and midnight transitions, and captures native screenshots. It never
connects to a physical watch or cloud relay. Reports pin the PBW's SHA-256;
packaging checks each payload's size and STM32 CRC before staging the release.
The source ZIP excludes emulator profiles and build caches.
`make icon` regenerates the 25×25 menu icon
from the same geometry, using ImageMagick only to rasterize it.

## Physical-watch follow-up

Test the PBW on Time 2 and 2 SE for outdoor readability and day-long battery
use. Review the dithered backdrop at actual watch size. The Pebble store release
uses the locally built, emulator-tested package. Garmin remains a separate port.
