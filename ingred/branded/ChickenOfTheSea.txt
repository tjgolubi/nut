: chicken of the sea

#define 2_5_OZ 70.87

[fish]

: packet_in_water = packet light tuna in water $$
 2_5_OZ    0    70  17      0      0      0    $packet_in_water
*2_5_OZ    0    70 "$packet_in_water"          2.5-oz $packet_in_water
* 5_OZ     0   140 "$packet_in_water"          5-oz $packet_in_water

: packet_in_water = packet albacore in water $$
 2_5_OZ    0    80  17      1      0      0    $packet_in_water
*2_5_OZ    0    80 "$packet_in_water"          2.5-oz $packet_in_water
* 5_OZ     0   160 "$packet_in_water"          5-oz $packet_in_water

: packet_in_water = packet smoked pink salmon in water $$
* 3_OZ     0   100  20      1.5    1      0    3-oz $packet_in_water
*   =              "3-oz $packet_in_water"     $packet_in_water

: packet_in_water = packet pink salmon in water $$
 2_5_OZ    0    70  15      1      0      0    $packet_in_water
*2_5_OZ    0    70 "$packet_in_water"          2.5-oz $packet_in_water
* 5_OZ     0    70 "$packet_in_water"          5-oz $packet_in_water

: in_water = light tuna in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ     0   100  23      0.5    0      0    $canned_drained

* 3_OZ     0    60 "$canned_drained"           3-oz can $in_water $$
* 4_OZ     0   100 "$canned_drained"           5-oz can $drained $$
* 5_OZ     0   100 "$canned_drained"           5-oz can $in_water $$
*12_OZ     0   240 "$canned_drained"           12-oz can $in_water $$
*  =               "5-oz can $drained $$"      can $drained $$
*  =               "5-oz can $in_water $$"     can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

: in_water = albacore in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ     0   100  20      1      0      0    $canned_drained
* 4_OZ     0   100 "$canned_drained"           5-oz can $drained $$
* 5_OZ     0   100 "$canned_drained"           5-oz can $in_water $$
*  =               "5-oz can $drained $$"      can $drained $$
*  =               "5-oz can $in_water $$"     can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

: in_water = solid $in_water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ     0   130  29      1      0      0    $canned_drained
* 4_OZ     0   130 "$canned_drained"           5-oz can $drained $$
* 5_OZ     0   130 "$canned_drained"           5-oz can $in_water $$
*12_OZ     0   312 "$canned_drained"           12-oz can $in_water $$
* 1885     0  1730 "$canned_drained"           66.5-oz can $in_water $$
*  =               "5-oz can $drained $$"      can $drained $$
*  =               "5-oz can $in_water $$"     can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

: in_water = pink salmon in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ     0   110  22      2      0      0    $canned_drained
* 4_OZ     0   110 "$canned_drained"           5-oz can $drained $$
* 5_OZ     0   110 "$canned_drained"           5-oz can $in_water $$
*  418     0   492 "$canned_drained"           14.75-oz can $in_water $$
*  =               "5-oz can $drained $$"      can $drained $$
*  =               "5-oz can $in_water $$"     can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

[shellfish]

: in_water = lump crabmeat in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
   120     0    70  14      0      2      0    $canned_drained
*  120     0    70 "$canned_drained"           6-oz can $drained $$
* 6_OZ     0    70 "$canned_drained"           6-oz can $in_water $$
*  =               "6-oz can $drained $$"      can $drained $$
*  =               "6-oz can $in_water $$"     can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

: in_water = white crabmeat in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ     0    60  13      0      2      0    $canned_drained
* 4_OZ     0    60 "$canned_drained"           6-oz can $drained $$
* 6_OZ     0    60 "$canned_drained"           6-oz can $in_water $$
*  =               "6-oz can $drained $$"      can $drained $$
*  =               "6-oz can $in_water $$"     can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

: in_water = chopped clams in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  3_OZ     0    45   8      0      3      0    $canned_drained
* 3_OZ     0    90 "$canned_drained"           6.5-oz can $drained $$
*  184     0    90 "$canned_drained"           6.5-oz can $in_water $$
*  =               "6.5-oz can $drained $$"    can $drained $$
*  =               "6.5-oz can $in_water $$"   can $in_water $$
   =               "can $in_water $$"          canned $in_water $$

: in_water = baby clams in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  5_OZ     0   130  23      1.5    5      0    $canned_drained
* 5_OZ     0   130 "$canned_drained"           10-oz can $drained $$
*10_OZ     0   130 "$canned_drained"           10-oz can $in_water $$
*  =               "10-oz can $drained $$"     can $drained $$
*  =               "10-oz can $in_water $$"    can $in_water $$
   =               "can $in_water $$"          canned $in_water $$
