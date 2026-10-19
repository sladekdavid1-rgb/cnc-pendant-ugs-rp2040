(2,5mm drill)
(T1 D=2.5 CR=0 TAPER=118deg - ZMIN=-4 - drill)
G90 G94
G17
G21
(-Attention- Property Safe Retracts is set to Clearance Height.)
(Ensure the clearance height will clear the part and or fixtures.)
(Raise the Z-axis to a safe height before starting the program.)

(Drill1)
T1
S12000 M3
G17 G90 G94
G54
G0 X31 Y36
Z16
G0 Z6
Z5
G1 Z-4 F500
G0 Z6
Y94
Z5
G1 Z-4 F500
G0 Z6
X119
Z5
G1 Z-4 F500
G0 Z6
Y36
Z5
G1 Z-4 F500
G0 Z6
Z16

M5
M30
