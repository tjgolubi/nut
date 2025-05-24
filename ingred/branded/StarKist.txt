: starkist

[fish]

: in_water = light tuna in water
: pouch_in_water = pouch $in_water $$
    56    60    60  13      0.5    0      0        $pouch_in_water
*   74     0    70 "$pouch_in_water"               2.6-oz $pouch_in_water
*  181     0   194 "$pouch_in_water"               6.4-oz $pouch_in_water
*  312     0   334 "$pouch_in_water"               11-oz $pouch_in_water

: drained = $in_water drained
: canned_drained = canned $drained $$
: can_in_water   = can $in_water $$
: can_drained    = can $drained $$
  4_OZ   105    89  20      0.5    0      0        $canned_drained
*   79     0    70 "$canned_drained"               3-oz $can_drained
* 3_OZ     0    70 "$canned_drained"               3-oz $can_in_water
* 4_OZ   105    89 "$canned_drained"               5-oz $can_drained
* 5_OZ     0    89 "$canned_drained"               5-oz $can_in_water
* 9_OZ   240   210 "$canned_drained"               12-oz $can_drained
*12_OZ     0   210 "$canned_drained"               12-oz $can_in_water
*  =               "5-oz $can_drained"             $can_drained
*  =               "5-oz $can_in_water"            $can_in_water
   =               "$can_in_water"                 canned $in_water $$

: in_water = albacore in water
: pouch_in_water = pouch $in_water $$
  3_OZ    80    90  19      1.5    0      0        $pouch_in_water
*   74     0    80 "$pouch_in_water"               2.6-oz $pouch_in_water
*  181     0   180 "$pouch_in_water"               6.4-oz $pouch_in_water

: drained  = $in_water drained
: canned_drained = canned $drained $$
: can_in_water   = can $in_water $$
: can_drained    = can $drained $$
  4_OZ   105   100  22      1.5    0      0        $canned_drained
*   79     0    80 "$canned_drained"               3-oz $can_drained
* 3_OZ     0    80 "$canned_drained"               3-oz $can_in_water
* 4_OZ   105   100 "$canned_drained"               5-oz $can_drained
* 5_OZ     0   100 "$canned_drained"               5-oz $can_in_water
* 9_OZ   240   237 "$canned_drained"               12-oz $can_drained
*12_OZ     0   237 "$canned_drained"               12-oz $can_in_water
*  =               "5-oz $can_drained"             $can_drained
*  =               "5-oz $can_in_water"            $can_in_water
   =               "$can_in_water"                 canned $in_water $$

: in_water = solid albacore in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
: can_in_water   = can $in_water $$
: can_drained    = can $drained $$
  4_OZ   105   110  26      1      0      0        $canned_drained
*   79     0    80 "$canned_drained"               3-oz $can_drained
* 3_OZ     0    80 "$canned_drained"               3-oz $can_in_water
* 4_OZ   105   110 "$canned_drained"               5-oz $can_drained
* 5_OZ     0   110 "$canned_drained"               5-oz $can_in_water
* 9_OZ   240   270 "$canned_drained"               12-oz $can_drained
*12_OZ     0   270 "$canned_drained"               12-oz $can_in_water
*  =               "5-oz $can_drained"             $can_drained
*  =               "5-oz $can_in_water"            $can_in_water
   =               "$can_in_water"                 canned $in_water $$

: in_water = pink salmon in water
: pouch_in_water = pouch $in_water $$
    74     0    70  15      1      1      0.5      $pouch_in_water
*   =              "$pouch_in_water"               2.6-oz $pouch_in_water

: drained = $in_water drained
: canned_drained = canned $drained $$
: can_drained    = can $drained $$
: can_in_water   = can $in_water $$
  3_OZ     0    90  19      1      0      0        $canned_drained
* 3_OZ     0    90  "$canned_drained"              5-oz $can_drained
* 5_OZ     0    90  "$canned_drained"              5-oz $can_in_water
*  =                "5-oz $can_drained"            $can_drained
*  =                "5-oz $can_in_water"           $can_in_water
   =                "$can_in_water"                canned $in_water $$
