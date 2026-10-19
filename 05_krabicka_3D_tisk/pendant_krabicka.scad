// CNC pendant – krabička a celá sestava (generováno). Jednotky mm.
// SHOW: 0 = celá sestava, 1 = jen krabička (pro tisk / export STL)
SHOW = 0;

module Krabicka() { color("#3a3f47") difference() {
    difference() {
    difference() {
    union() {
    difference() {
    union() {
    translate([0,0,0.000]) translate([7.000,7.000,0]) linear_extrude(25.700) offset(r=7.000, $fn=48) square([87.000,57.000]);
  }
    translate([0,0,1.990]) translate([7.000,7.000,0]) linear_extrude(23.720) offset(r=4.400, $fn=48) square([87.000,57.000]);
  }
    translate([0,0,0.000]) translate([6.500,64.500,0]) cylinder(d=8.000, h=7.000, $fn=64);
    translate([0,0,0.000]) translate([94.500,64.500,0]) cylinder(d=8.000, h=7.000, $fn=64);
    translate([0,0,0.000]) translate([6.500,6.500,0]) cylinder(d=8.000, h=7.000, $fn=64);
    translate([0,0,0.000]) translate([94.500,6.500,0]) cylinder(d=8.000, h=7.000, $fn=64);
  }
    translate([0,0,-0.010]) translate([6.500,64.500,0]) cylinder(d=3.400, h=7.020, $fn=64);
    translate([0,0,-0.010]) translate([94.500,64.500,0]) cylinder(d=3.400, h=7.020, $fn=64);
    translate([0,0,-0.010]) translate([6.500,6.500,0]) cylinder(d=3.400, h=7.020, $fn=64);
    translate([0,0,-0.010]) translate([94.500,6.500,0]) cylinder(d=3.400, h=7.020, $fn=64);
  }
    translate([0,0,-0.010]) translate([6.500,64.500,0]) cylinder(d=6.500, h=2.520, $fn=64);
    translate([0,0,-0.010]) translate([94.500,64.500,0]) cylinder(d=6.500, h=2.520, $fn=64);
    translate([0,0,-0.010]) translate([6.500,6.500,0]) cylinder(d=6.500, h=2.520, $fn=64);
    translate([0,0,-0.010]) translate([94.500,6.500,0]) cylinder(d=6.500, h=2.520, $fn=64);
  }
    translate([0,0,9.690]) translate([-1.000,28.390,0]) cube([4.100,13.000,8.020]);
  } }
module Panel() { color("#1b1b1b") difference() {
    union() {
    translate([0,0,24.700]) translate([7.000,7.000,0]) linear_extrude(1.000) offset(r=4.000, $fn=48) square([87.000,57.000]);
  }
    translate([0,0,24.690]) translate([12.000,58.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([21.000,58.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([30.000,58.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([39.000,58.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([48.000,58.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([57.000,58.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([12.000,47.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([30.000,47.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([48.000,47.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([57.000,47.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([12.000,37.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([21.000,37.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([39.000,37.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([48.000,37.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([12.000,26.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([30.000,26.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([48.000,26.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([57.000,26.500,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([21.000,15.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([30.000,15.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([39.000,15.000,0]) cylinder(d=4.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([74.500,60.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([78.500,60.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([82.500,60.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([68.500,51.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([72.500,51.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([76.500,51.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([80.500,51.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([84.500,51.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([88.500,51.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([72.500,43.000,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([76.500,43.000,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([80.500,43.000,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([84.500,43.000,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([68.500,34.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([72.500,34.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([76.500,34.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([80.500,34.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([84.500,34.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([88.500,34.500,0]) cylinder(d=2.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([78.500,18.500,0]) cylinder(d=7.000, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([6.500,64.500,0]) cylinder(d=3.200, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([94.500,64.500,0]) cylinder(d=3.200, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([6.500,6.500,0]) cylinder(d=3.200, h=1.020, $fn=64);
    translate([0,0,24.690]) translate([94.500,6.500,0]) cylinder(d=3.200, h=1.020, $fn=64);
  } }
module DPS_A() { color("#1f6f3a") difference() {
    union() {
    translate([0,0,19.600]) translate([7.000,7.000,0]) linear_extrude(1.600) offset(r=4.000, $fn=48) square([87.000,57.000]);
  }
    translate([0,0,19.590]) translate([6.500,64.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,19.590]) translate([94.500,64.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,19.590]) translate([6.500,6.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,19.590]) translate([94.500,6.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,19.590]) translate([78.500,18.500,0]) cylinder(d=10.000, h=1.620, $fn=64);
  } }
module DPS_B() { color("#2d7a46") difference() {
    union() {
    translate([0,0,7.000]) translate([7.000,7.000,0]) linear_extrude(1.600) offset(r=4.000, $fn=48) square([87.000,57.000]);
  }
    translate([0,0,6.990]) translate([6.500,64.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,6.990]) translate([94.500,64.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,6.990]) translate([6.500,6.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
    translate([0,0,6.990]) translate([94.500,6.500,0]) cylinder(d=3.200, h=1.620, $fn=64);
  } }
module DPS_C() { color("#2f8f52") union() {
    translate([0,0,22.700]) linear_extrude(0.800) polygon([[62.500,64.000],[90.000,64.000],[90.000,61.000],[93.500,61.000],[93.500,31.000],[62.500,31.000]]);
  } }
module Sloupky_AB_M3x11_FF() { color("#c8a24a") difference() {
    union() {
    translate([0,0,8.600]) translate([6.500,64.500,0]) cylinder(d=5.500, h=11.000, $fn=64);
    translate([0,0,8.600]) translate([94.500,64.500,0]) cylinder(d=5.500, h=11.000, $fn=64);
    translate([0,0,8.600]) translate([6.500,6.500,0]) cylinder(d=5.500, h=11.000, $fn=64);
    translate([0,0,8.600]) translate([94.500,6.500,0]) cylinder(d=5.500, h=11.000, $fn=64);
  }
    translate([0,0,8.590]) translate([6.500,64.500,0]) cylinder(d=2.500, h=11.020, $fn=64);
    translate([0,0,8.590]) translate([94.500,64.500,0]) cylinder(d=2.500, h=11.020, $fn=64);
    translate([0,0,8.590]) translate([6.500,6.500,0]) cylinder(d=2.500, h=11.020, $fn=64);
    translate([0,0,8.590]) translate([94.500,6.500,0]) cylinder(d=2.500, h=11.020, $fn=64);
  } }
module Distance_A_panel_3_5() { color("#e8e8e0") difference() {
    union() {
    translate([0,0,21.200]) translate([6.500,64.500,0]) cylinder(d=6.000, h=3.500, $fn=64);
    translate([0,0,21.200]) translate([94.500,64.500,0]) cylinder(d=6.000, h=3.500, $fn=64);
    translate([0,0,21.200]) translate([6.500,6.500,0]) cylinder(d=6.000, h=3.500, $fn=64);
    translate([0,0,21.200]) translate([94.500,6.500,0]) cylinder(d=6.000, h=3.500, $fn=64);
  }
    translate([0,0,21.190]) translate([6.500,64.500,0]) cylinder(d=3.200, h=3.520, $fn=64);
    translate([0,0,21.190]) translate([94.500,64.500,0]) cylinder(d=3.200, h=3.520, $fn=64);
    translate([0,0,21.190]) translate([6.500,6.500,0]) cylinder(d=3.200, h=3.520, $fn=64);
    translate([0,0,21.190]) translate([94.500,6.500,0]) cylinder(d=3.200, h=3.520, $fn=64);
  } }
module Sroub_horni_M3x10_ISO7380() { color("#9aa0a6") union() {
    union() {
    translate([0,0,25.700]) translate([6.500,64.500,0]) cylinder(d=5.700, h=1.650, $fn=64);
    translate([0,0,25.700]) translate([94.500,64.500,0]) cylinder(d=5.700, h=1.650, $fn=64);
    translate([0,0,25.700]) translate([6.500,6.500,0]) cylinder(d=5.700, h=1.650, $fn=64);
    translate([0,0,25.700]) translate([94.500,6.500,0]) cylinder(d=5.700, h=1.650, $fn=64);
  }
    translate([0,0,15.700]) translate([6.500,64.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
    translate([0,0,15.700]) translate([94.500,64.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
    translate([0,0,15.700]) translate([6.500,6.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
    translate([0,0,15.700]) translate([94.500,6.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
  } }
module Sroub_spodni_M3x10() { color("#9aa0a6") union() {
    union() {
    translate([0,0,0.850]) translate([6.500,64.500,0]) cylinder(d=5.500, h=1.650, $fn=64);
    translate([0,0,0.850]) translate([94.500,64.500,0]) cylinder(d=5.500, h=1.650, $fn=64);
    translate([0,0,0.850]) translate([6.500,6.500,0]) cylinder(d=5.500, h=1.650, $fn=64);
    translate([0,0,0.850]) translate([94.500,6.500,0]) cylinder(d=5.500, h=1.650, $fn=64);
  }
    translate([0,0,2.500]) translate([6.500,64.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
    translate([0,0,2.500]) translate([94.500,64.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
    translate([0,0,2.500]) translate([6.500,6.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
    translate([0,0,2.500]) translate([94.500,6.500,0]) cylinder(d=3.000, h=10.000, $fn=64);
  } }
module Pico() { color("#2b6cb0") union() {
    translate([0,0,11.100]) translate([9.130,24.890,0]) linear_extrude(1.000) offset(r=0.500, $fn=48) square([50.000,20.000]);
  } }
module Pico_USB_C() { color("#c0c4c8") union() {
    translate([0,0,12.100]) translate([7.600,30.690,0]) linear_extrude(3.200) offset(r=0.300, $fn=48) square([6.700,8.400]);
  } }
module Pico_listy() { color("#222222") union() {
    translate([0,0,8.600]) translate([8.730,42.510,0]) cube([50.800,2.540,2.500]);
    translate([0,0,8.600]) translate([8.730,24.730,0]) cube([50.800,2.540,2.500]);
  } }
module EC11E() { color("#8a8f96") union() {
    union() {
    union() {
    translate([0,0,8.600]) translate([72.500,11.800,0]) cube([12.000,13.400,6.500]);
  }
    translate([0,0,15.100]) translate([78.500,18.500,0]) cylinder(d=7.000, h=5.000, $fn=64);
  }
    translate([0,0,20.100]) translate([78.500,18.500,0]) cylinder(d=6.000, h=18.500, $fn=64);
  } }
module Knoflik_D25() { color("#555a60") difference() {
    union() {
    translate([0,0,26.700]) translate([78.500,18.500,0]) cylinder(d=25.000, h=15.000, $fn=64);
  }
    translate([0,0,26.690]) translate([78.500,18.500,0]) cylinder(d=6.000, h=12.020, $fn=64);
  } }
module Tlacitka_6x6x6() { color("#2a2a2a") union() {
    translate([0,0,21.200]) translate([9.000,55.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([18.000,55.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([27.000,55.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([36.000,55.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([45.000,55.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([54.000,55.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([9.000,44.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([27.000,44.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([45.000,44.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([54.000,44.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([9.000,34.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([18.000,34.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([36.000,34.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([45.000,34.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([9.000,23.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([27.000,23.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([45.000,23.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([54.000,23.500,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([18.000,12.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([27.000,12.000,0]) cube([6.000,6.000,3.500]);
    translate([0,0,21.200]) translate([36.000,12.000,0]) cube([6.000,6.000,3.500]);
  } }
module Hmatniky() { color("#444444") union() {
    translate([0,0,24.700]) translate([12.000,58.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([21.000,58.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([30.000,58.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([39.000,58.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([48.000,58.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([57.000,58.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([12.000,47.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([30.000,47.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([48.000,47.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([57.000,47.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([12.000,37.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([21.000,37.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([39.000,37.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([48.000,37.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([12.000,26.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([30.000,26.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([48.000,26.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([57.000,26.500,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([21.000,15.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([30.000,15.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
    translate([0,0,24.700]) translate([39.000,15.000,0]) cylinder(d=3.500, h=2.500, $fn=64);
  } }
module LED_0603() { color("#f2f2f2") union() {
    translate([0,0,23.500]) translate([73.700,60.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([77.700,60.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([81.700,60.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([67.700,51.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([71.700,51.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([75.700,51.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([79.700,51.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([83.700,51.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([87.700,51.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([71.700,42.600,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([75.700,42.600,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([79.700,42.600,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([83.700,42.600,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([67.700,34.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([71.700,34.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([75.700,34.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([79.700,34.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([83.700,34.100,0]) cube([1.600,0.800,0.600]);
    translate([0,0,23.500]) translate([87.700,34.100,0]) cube([1.600,0.800,0.600]);
  } }
module 74HC595_SOIC16() { color("#202020") union() {
    translate([0,0,17.850]) translate([73.550,56.550,0]) cube([9.900,3.900,1.750]);
    translate([0,0,17.850]) translate([73.550,44.550,0]) cube([9.900,3.900,1.750]);
    translate([0,0,17.850]) translate([73.550,32.550,0]) cube([9.900,3.900,1.750]);
  } }
module Konektory_A_B() { color("#111111") union() {
    translate([0,0,8.600]) translate([94.330,23.030,0]) cube([2.540,15.240,11.000]);
    translate([0,0,8.600]) translate([15.310,19.980,0]) cube([25.400,2.540,11.000]);
  } }
module Listy_A_C() { color("#111111") union() {
    translate([0,0,21.200]) translate([63.930,32.790,0]) cube([2.540,30.480,1.500]);
    translate([0,0,21.200]) translate([90.530,32.330,0]) cube([2.540,27.940,1.500]);
  } }

if (SHOW == 1) Krabicka();
else { Krabicka(); Panel(); DPS_A(); DPS_B(); DPS_C(); Sloupky_AB_M3x11_FF(); Distance_A_panel_3_5(); Sroub_horni_M3x10_ISO7380(); Sroub_spodni_M3x10(); Pico(); Pico_USB_C(); Pico_listy(); EC11E(); Knoflik_D25(); Tlacitka_6x6x6(); Hmatniky(); LED_0603(); 74HC595_SOIC16(); Konektory_A_B(); Listy_A_C(); }
