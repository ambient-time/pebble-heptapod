# Heptapod source reference

The source of my [Heptapod B](https://datapoems.io/clocks/heptapod/) web clock
is preserved byte for byte in `source.html`, copied September 5, 2026.

SHA-256: `da8fab84c1f14bac107776367179db23fa00e84263261e92d87e5a45c27cdac8`.

The archive is reference material, not a runtime dependency. It includes the
original page's external font and perfect-freehand imports; the native C
watchface uses neither. The MIT license covers the native code and the
original page; third-party libraries retain their own licenses and are not
bundled in this port.

## Preserved behavior

Local hour, minute, and second bearings start at twelve and advance clockwise.
The original six arc definitions, twelve ornamental flourishes, six blots,
and time-tendril length/width/curl ratios are retained. A broken, pressure-
weighted ink ring surrounds an empty center. Its radius breathes and tips sway.

## Native differences

Pressure-expanded native polygons replace perfect-freehand. A half-resolution
blurred ink mask and four drifting cloud banks approximate the original smoke
passes on color and monochrome displays using spatial dithering. Ink is black.
Motion requests a 250 ms interval and consumes fractional seconds; Calm motion
freezes atmospheric phase while the time marks still update each second.
Atmospheric phase uses seconds-of-day rather than page uptime. Global rotation
is omitted to retain the twelve-at-top anchor. Safe margins reduce the radius
on round screens. The native HH:MM label follows the watch's 12/24 preference
and omits the browser's visible seconds; the second tendril still advances.

These are deliberate trade-offs between fidelity, readability and battery
use. The clock is an interpretation inspired by *Arrival*, not an official
product or a translation of the film's fictional language.
