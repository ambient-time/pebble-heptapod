# Heptapod for Pebble

The current revision targets round Chalk and Gabbro watches. Older releases and their target declarations remain archived.

A broken ring of ink tells local time. Three tendrils grow from their clock
positions: long for hours, medium for minutes, short for seconds. Twelve stays
at the top as the ring breathes and the brush tips sway.

Native C watchface, version 0.1.4. One PBW and UUID cover Time Round (Chalk)
and Round 2 (Gabbro). Emulator checks do not measure physical-watch battery use.
It tells time offline. Local phone settings can hide the bottom digital clock
or select Calm motion. The clock is shown by default and follows the watch’s
12/24-hour preference. Atmospheric motion requests four updates per second;
Calm motion freezes the mist and updates time once per second. Drawing pauses
when another app takes focus.

[Pebble Appstore](https://apps.repebble.com/eed88192a6a24a229eceb50c) ·
[PBW](release/heptapod-0.1.3.pbw) · [Source ZIP](release/heptapod-0.1.3-source.zip)
· [Checksums](release/SHA256SUMS) · [Native capture sheet](evidence/0.1.3/options-contact-sheet.png)
· [Time 2 motion preview](evidence/0.1.3/emery-motion-0.1.3.gif) · [Source notes](reference/README.md)

Release files and native captures accompany the canonical project; the source
ZIP contains only the files needed to build, test, and reproduce those captures.
Version 0.1.1 corrects author metadata and adds portable packaging and natural
rollover checks. It keeps the renderer and UUID from 0.1.0. Version 0.1.2
removed the digital caption. Version 0.1.3 restores it by default, makes it
optional, recenters the glyph when hidden, and restores moving mist. This
update is a local review candidate; the store link may show the earlier release.

## Adaptation

Source: Luke Steuber's supplied [Heptapod B clock](https://datapoems.io/clocks/heptapod/),
archived locally as [reference/source.html](reference/source.html). This port
keeps its six arcs, twelve ornamental flourishes, six ink blots, and the
proportions of all three time tendrils. Arrival inspired the design; it has no
official affiliation and does not translate the film's fictional language.

Native pressure-expanded ribbons form the brush outline. The port includes no
perfect-freehand code or film glyph assets. Both color and monochrome
Pebbles use drifting, dithered clouds and a blurred ink halo. A bounded
half-resolution mask supplies the smoke; the native outline stays sharp enough
to preserve the small tendrils. Brush tips sway further than in the earlier
port. Holding the glyph upright keeps twelve in a fixed position.

Geometry has no hidden clock state. A caller-owned workspace avoids large
stack arrays, firmware trig replaces desktop libm, and small sampled pressure
curves preserve the original taper. The host tests check approximation error,
18,688 time/shape/clock-option samples, motion with atmosphere frozen, and cardinal angles;
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
python3 test/emulator.py chalk --fresh
python3 test/emulator.py gabbro --fresh
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

Test the PBW on Time Round and Round 2 for outdoor readability and day-long battery
use. Review the dithered backdrop at actual watch size. The Pebble store release
uses the locally built, emulator-tested package. Garmin remains a separate port.
