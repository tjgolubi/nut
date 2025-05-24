: bumble bee

#define 2_2_OZ  62.37
#define 48_OZ   1361
#define 66_5_OZ 1885

[fish]

: in_water = light tuna in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ   105   110  23      1      1      0    $canned_drained

*2_2_OZ    0    60 "$canned_drained"       3-oz can $drained $$
* 3_OZ     0    60 "$canned_drained"       3-oz can $in_water $$
* 4_OZ     0   110 "$canned_drained"       5-oz can $drained $$
* 5_OZ     0   110 "$canned_drained"       5-oz can $in_water $$
* 9_OZ     0   240 "$canned_drained"       12-oz can $drained $$
*12_OZ     0   240 "$canned_drained"       12-oz can $in_water $$
*48_OZ     0  1280 "$canned_drained"       66.5-oz can $drained $$
*66_5_OZ   0  1280 "$canned_drained"       66.5-oz can $in_water $$
* =                "5-oz can $drained $$"  can $drained $$
* =                "5-oz can $in_water $$" can $in_water $$
  =                "can $in_water $$"      canned $in_water $$

: in_water = albacore in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ   105   120  23      2.5    1      0    $canned_drained

*2_2_OZ    0    70 "$canned_drained"      3-oz can $drained $$
* 3_OZ     0    70 "$canned_drained"      3-oz can $in_water $$
* 4_OZ     0   120 "$canned_drained"      5-oz can $drained $$
* 5_OZ     0   120 "$canned_drained"      5-oz can $in_water $$
* 9_OZ     0   270 "$canned_drained"      12-oz can $drained $$
*12_OZ     0   270 "$canned_drained"      12-oz can $in_water $$
*48_OZ     0  1440 "$canned_drained"      66.5-oz can $drained $$
*66_5_OZ   0  1440 "$canned_drained"      66.5-oz can $in_water $$
*  =               "5-oz can $drained $$"    can $drained $$
*  =               "5-oz can $in_water $$"            can $in_water $$
   =               "can $in_water $$"                 canned $in_water $$

: in_water = solid albacore in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ   105   130  29      1.5    0      0    $canned_drained

*2_2_OZ    0    70 "$canned_drained"   3-oz can $drained $$
* 3_OZ     0    70 "$canned_drained"   3-oz can $in_water $$
* 4_OZ     0   130 "$canned_drained"   5-oz can $drained $$
* 5_OZ     0   130 "$canned_drained"   5-oz can $in_water $$
*  144     0   170 "$canned_drained"   7-oz can $drained $$
*  198     0   170 "$canned_drained"   7-oz can $in_water $$
* 9_OZ     0   300 "$canned_drained"   12-oz can $drained $$
*12_OZ     0   300 "$canned_drained"   12-oz can $in_water $$
*48_OZ     0  1600 "$canned_drained"   66.5-oz can $drained $$
*66_5_OZ   0  1600 "$canned_drained"   66.5-oz can $in_water $$
*  =               "5-oz can $drained $$" can $drained $$
*  =               "5-oz can $in_water $$"         can $in_water $$
   =               "can $in_water $$"              canned $in_water $$

: in_water = prime albacore in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ   105   140  32      1.5    0      0    $canned_drained

* 4_OZ     0   140 "$canned_drained"   5-oz can $drained $$
* 5_OZ     0   140 "$canned_drained"   5-oz can $in_water $$
*  =               "5-oz can $drained $$" can $drained $$
*  =               "5-oz can $in_water $$"         can $in_water $$
   =               "can $in_water $$"              canned $in_water $$

: in_water = pink salmon in water
: drained  = $in_water drained
*   92    80   100  20      2.5    0      0  5-oz can $drained $$
* 5_OZ     0   100 "5-oz can $drained $$"    5-oz can $in_water $$
*  213   200   275  48     10      1      0  7.5-oz can $in_water $$
  3_OZ    80   120  18      5      0      0  canned $in_water $$
*  418     0   600 "canned $in_water $$"     14.75-oz can $in_water $$
*  =               "5-oz can $drained $$"    can $drained $$
*  =               "5-oz can $in_water $$"   can $in_water $$
   =               "can $drained $$"         canned $drained $$

: in_water = sockeye salmon in water
: drained  = $in_water drained
*   92     0   110  21      3      0      0 5-oz can $drained $$
* 5_OZ     0   110 "5-oz can $drained $$"   5-oz can $in_water $$
  3_OZ    80   130  20      6      0      0 canned $in_water $$
*  213   200   325 "canned $in_water $$"    7.5-oz can $in_water $$
*  418   400   650 "canned $in_water $$"    14.75-oz can $in_water $$
*  =               "5-oz can $drained $$"   can $drained $$
*  =               "5-oz can $in_water $$"  can $in_water $$
   =               "can $drained $$"        canned $drained $$

: in_water = atlantic salmon in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ   120   220  24     14      0      0    $canned_drained

* 4_OZ     0   220 "$canned_drained"       5-oz can $drained $$
* 5_OZ     0   220 "$canned_drained"       5-oz can $in_water $$
*  =               "5-oz can $drained $$"  can $drained $$
*  =               "5-oz can $in_water $$" can $in_water $$
   =               "can $in_water $$"      canned $in_water $$

[shellfish]

: in_water = shrimp in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
  4_OZ     0   110  22      2      1      0    $canned_drained

* 4_OZ     0   110 "$canned_drained"              6-oz can tiny $drained $$
* 6_OZ     0   110 "$canned_drained"              6-oz can tiny $in_water $$
*  =               "6-oz can tiny $drained $$"    can tiny $drained $$
*  =               "6-oz can tiny $in_water $$"   can tiny $in_water $$
   =               "can tiny $in_water $$"        canned tiny $in_water $$
* 4_OZ     0   110 "$canned_drained"              6-oz can medium $drained $$
* 6_OZ     0   110 "$canned_drained"              6-oz can medium $in_water $$
*  =               "6-oz can medium $drained $$"  can medium $drained $$
*  =               "6-oz can medium $in_water $$" can medium $in_water $$
*  =               "can medium $drained $$"       can $drained $$
*  =               "can medium $in_water $$"      can $in_water $$
   =               "can $in_water $$"             canned $in_water $$

: in_water = crabmeat in water
: drained  = $in_water drained
: canned_drained = canned $drained $$
   120     0    80  16      1.5    1      0    $canned_drained

*  120     0    80 "$canned_drained"             6-oz can white $drained $$
* 6_OZ     0    80 "$canned_drained"             6-oz can white $in_water $$
*  =               "6-oz can white $drained $$"  can white $drained $$
*  =               "6-oz can white $in_water $$" can white $in_water $$
   =               "can white $in_water $$"      canned white $in_water $$
*  120     0    80 "$canned_drained"             6-oz can lump $drained $$
* 6_OZ     0    80 "$canned_drained"             6-oz can lump $in_water $$
*  =               "6-oz can lump $drained $$"   can lump $drained $$
*  =               "6-oz can lump $in_water $$"  can lump $in_water $$
   =               "can lump $in_water $$"       canned lump $in_water $$
*  120     0    80 "$canned_drained"             6-oz can pink $drained $$
* 6_OZ     0    80 "$canned_drained"             6-oz can pink $in_water $$
*  =               "6-oz can pink $drained $$"   can pink $drained $$
*  =               "6-oz can pink $in_water $$"  can pink $in_water $$
   =               "can pink $in_water $$"       canned pink $in_water $$
*  =               "6-oz can pink $drained $$"   can $drained $$
*  =               "6-oz can pink $in_water $$"  can $in_water $$
   =               "can $in_water $$"            canned $in_water $$
