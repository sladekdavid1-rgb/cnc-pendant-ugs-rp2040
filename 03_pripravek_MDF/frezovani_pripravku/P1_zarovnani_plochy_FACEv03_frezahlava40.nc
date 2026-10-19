(FACEv03)
(T13 D=40 CR=0 - ZMIN=0 - face mill)
G90 G94
G17
G21
(-Attention- Property Safe Retracts is set to Clearance Height.)
(Ensure the clearance height will clear the part and or fixtures.)
(Raise the Z-axis to a safe height before starting the program.)
(When using Fusion for Personal Use, the feedrate of rapid)
(moves is reduced to match the feedrate of cutting moves,)
(which can increase machining time. Unrestricted rapid moves)
(are available with a Fusion Subscription.)

(Face1)
T13
S12000 M3
G17 G90 G94
G54
G0 X178 Y5.168
Z15.2
G1 Z5.2 F500
Z4 F333.3
G18 G3 X174 Z0 I-4 K0 F500
G1 X172.01
X-22.01
G3 X-26.01 Z4 I0 K4
G1 Z5.2
X178 Y32.056
Z4 F333.3
G3 X174 Z0 I-4 K0 F500
G1 X172.01
X-22.01
G3 X-26.01 Z4 I0 K4
G1 Z5.2
X178 Y58.944
Z4 F333.3
G3 X174 Z0 I-4 K0 F500
G1 X172.01
X-22.01
G3 X-26.01 Z4 I0 K4
G1 Z5.2
X178 Y85.832
Z4 F333.3
G3 X174 Z0 I-4 K0 F500
G1 X172.01
X-22.01
G3 X-26.01 Z4 I0 K4
G1 Z5.2
X178 Y112.72
Z4 F333.3
G3 X174 Z0 I-4 K0 F500
G1 X172.01
X-22.01
G3 X-26.01 Z4 I0 K4
G1 Z15.2
G17

M5
M30
