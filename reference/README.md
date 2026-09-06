# Heptapod source reference

Luke Steuber's [Heptapod B](https://datapoems.io/clocks/heptapod/) source is
preserved byte-for-byte in `source.html`, copied September 5, 2026 from
`clocks/heptapod/index.html`.

SHA-256: `da8fab84c1f14bac107776367179db23fa00e84263261e92d87e5a45c27cdac8`.

The archive is reference material, not a runtime dependency. It includes the
original page's external font and perfect-freehand imports; the native C
watchface uses neither. Luke Steuber's MIT license covers the native code and
original page; third-party libraries retain their own licenses and are not
bundled in this port.

## Preserved behavior

Local hour, minute, and second bearings start at twelve and advance clockwise.
The original six arc definitions, twelve ornamental flourishes, six blots,
and time-tendril length/width/curl ratios are retained. A broken, pressure-
weighted ink ring surrounds an empty center. Its radius breathes and tips sway.

## Native differences

Pressure-expanded native polygons replace perfect-freehand and smoke-blur
compositing. Color screens use static white/gray dithering; monochrome uses
white. Ink is black. The silhouette updates once per second, with atmospheric
phase derived from UTC seconds-of-day rather than page uptime. Global rotation
is omitted to retain the twelve-at-top anchor. Safe margins reduce the radius
on round screens. The native HH:MM label follows the watch's 12/24 preference
and omits the browser's visible seconds; the second tendril still advances.

These are explicit fidelity/readability/battery tradeoffs, not claims that
every omitted effect is impossible on Pebble. Physical-watch daylight
readability and sustained battery use remain unmeasured. The clock is an
Arrival-inspired interpretation, not an official product or fictional-language
translation.
