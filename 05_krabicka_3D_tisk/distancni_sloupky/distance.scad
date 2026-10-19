// CNC pendant – distanční sloupky (tisk na stojato). Jednotky mm.
// B->A: šestihran 5,5 (tištěno 5,4 na plochy – pasuje do nástrčného klíče 5,5 / M3), délka 11 mm,
//       otvor 2,6 = šroub M3 se do plastu sám zařízne z obou stran
// A->panel: kulatý Ø6 × 3,5 mm, otvor 3,4 = šroub M3 jen prochází
$fn = 96;
AF   = 5.4;   // rozměr na plochy (klíč 5,5); když je v klíči volný, dej 5.5
HOLE = 2.6;   // 2.5 tužší / 2.7 lehčí šroubování
module hexsp(af, id, h, ch=0.3) difference() {
  hull() { cylinder(d=(af-2*ch)/cos(30), h=h, $fn=6);
           translate([0,0,ch]) cylinder(d=af/cos(30), h=h-2*ch, $fn=6); }
  translate([0,0,-1]) cylinder(d=id, h=h+2);
}
module tube(od, id, h, ch=0.3) difference() {
  hull() { cylinder(d=od-2*ch, h=h); translate([0,0,ch]) cylinder(d=od, h=h-2*ch); }
  translate([0,0,-1]) cylinder(d=id, h=h+2);
}
for (i=[0:4]) translate([i*9,0,0])  hexsp(AF, HOLE, 11.0);
for (i=[0:4]) translate([i*9,10,0]) tube(6.0, 3.4, 3.5, 0.25);
