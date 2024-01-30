# Meat
[meat]
: ground beef 30%
   100     0   332  14.35  30.00   0.00   0.00 $$		// usda 168652
   100     0   270  25.56  17.86   0.00   0.00 pan-browned $$	// usda 169473
   139     0     0 this                        replace
  8_OZ     0     = this                        $$, pan-browned
   100     0   241  23.87  15.37   0.00   0.00 baked $$		// usda 169474
*  284     0     0 this                        loaf $this
*   LB     0     = "loaf $this"                loaf $$, baked
     = "loaf $$, baked"                        $$, baked
   100     0   238  22.86  15.54   0.00   0.00 pan-broiled $$	// usda 168650
*   77     0     0 this                        patty $this
* 4_OZ     0     = "patty $this"               patty $$, pan-broiled
     = "patty $$, pan-broiled"                 $$, pan-broiled
   100     0   277  25.38  18.66   0.00   0.00 broiled $$	// usda 168651
*   70     0     0 this                        patty $this
* 4_OZ     0     = "patty $this"               patty $$, broiled
     = "patty $$, broiled"                     $$, broiled

: ground beef 25%
   100     0   293  15.76  25.00   0.00   0.00 $$ // usda 174037
*   85     0     0 this                        serving $this // 3 oz
   100     0   277  26.28  18.21   0.00   0.00 pan-browned $$	// usda 174040
   100     0   254  24.56  16.50   0.00   0.00 baked $$		// usda 171801
   100     0   279  25.56  18.87   0.00   0.00 broiled $$	// usda 174038
   100     0   248  23.45  16.44   0.00   0.00 pan-broiled $$	// usda 174039

: ground beef 20%
   100     0   254  17.17  20.00   0.00   0.00 $$		// usda 174036
   100     0   272  27.00  17.36   0.00   0.00 pan-browned $$	// usda 171799
   100     0   254  25.25  16.17   0.00   0.00 baked $$		// usda 171800
   100     0   270  25.75  17.78   0.00   0.00 broiled $$	// usda 171797
   100     0   246  24.04  15.94   0.00   0.00 pan-broiled $$	// usda 171798

: ground beef 15%
   100     0   215  18.59  15.00   0.00   0.00 $$		// usda 171796
   100     0   256  27.73  15.30   0.00   0.00 pan-browned $$	// usda 174034
   100     0   240  25.93  14.36   0.00   0.00 baked $$		// usda 174035
   100     0   250  25.93  15.41   0.00   0.00 broiled $$	// usda 174032
*   77     0     0 this                        patty $this // yield from 1/4 lb raw meat
* 4_OZ     0     = "patty $this"               patty $$, broiled
     = "patty $$, broiled"                     $$, broiled
   100     0   232  24.62  14.02   0.00   0.00 pan-broiled $$	// usda 174033
*   83     0     0 this                        patty $this //  yield from 1/4 lb raw meat
* 4_OZ     0     = "patty $this"               patty $$, pan-broiled
     = "patty $$, pan-broiled"                 $$, pan-broiled

/ground beef 15%/ground beef/

: ground beef 10%
   100     0   176  20.00  10.00   0.00   0.00 $$		// usda 174030
   100     0   230  28.45  12.04   0.00   0.00 pan-browned $$	// usda 171794
*  154     0     0 this                        portion $this //  yield from 1/2 lb raw meat
* 8_OZ     0     = "portion $this"             portion $$, pan-browned
     = "portion $$, pan-browned"  $$, pan-browned
   100     0   214  26.62  11.10   0.00   0.00 baked $$		// usda 171795
   100     0   217  26.11  11.75   0.00   0.00 broiled $$	// usda 174031
   100     0   204  25.21  10.68   0.00   0.00 pan-broiled $$	// usda 171793

: ground beef 7%
   100     0   152  20.85   7.00   0.00   0.00 $$		// usda 173110
   100     0   209  28.88   9.51   0.00   0.00 pan-browned $$	// usda 174755
   100     0   192  27.03   8.43   0.00   0.00 baked $$		// usda 174754
   100     0   193  26.22   8.94   0.00   0.00 broiled $$	// usda 174752
   100     0   182  25.56   8.01   0.06   0.00 pan-broiled $$	// usda 174753

: ground beef 5%
   100     0   137  21.41   5.00   0.00   0.00 $$		// usda 171790
   100     0   193  29.17   7.58   0.00   0.00 pan-browned $$	// usda 174028
   100     0   174  27.31   6.37   0.00   0.00 baked $$		// usda 174029
   100     0   174  26.29   6.80   0.00   0.00 broiled $$	// usda 171791
   100     0   164  25.80   5.94   0.00   0.00 pan-broiled $$	// usda 171792

: ground beef 3%
   100     0   121  21.98   3.00   0.00   0.00 $$		// usda 173111
   100     0   175  29.46   5.46   0.00   0.00 pan-browned $$	// usda 174756
   100     0   154  27.58   4.06   0.00   0.00 baked $$		// usda 173114
   100     0   153  26.36   4.46   0.00   0.00 broiled $$	// usda 173112
*   70     0     0 this                        patty $this
   100     0   144  26.03   3.65   0.00   0.00 pan-broiled $$	// usda 173113
*  139     0     0 this                        patty $this // yield from 1/2 lb raw meat
* 8_OZ     0     = "patty $this"               patty $$, pan-broiled
     = "patty $$, pan-broiled"                 $$, pan-broiled

: frozen ground beef
   100     0   295  23.05  21.83   0.00   0.00 broiled $$ 	// usda 169447
*  313     0     0 this                        patty $this // yield from 1 lb raw meat
*   LB     0     = "patty $this"               patty $$, broiled
     = "patty $$, broiled"                     $$, broiled

   100     0   240  25.07  14.53   0.62   0.00 cooked ground beef // usda 172161
:
