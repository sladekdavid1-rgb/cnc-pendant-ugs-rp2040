import os
import re, math, sys
import numpy as np
from scipy import ndimage
import pcbcheck as P
RES=P.RES
def gcode_cuts(fn):
    x=y=z=0.0; cuts=[]; cur=[]
    tool=None
    for l in open(fn,errors='ignore'):
        m=re.search(r'TOOL DIAMETER:\s*([\d.]+)',l)
        if m: tool=float(m[1])
        l=l.split(';')[0]; l=re.sub(r'\(.*?\)','',l)
        g=re.findall(r'([GXYZIJ])(-?\d*\.?\d+)',l)
        if not g: continue
        d={k:float(v) for k,v in g if k!='G'}; gs=[int(float(v)) for k,v in g if k=='G']
        nx,ny,nz=d.get('X',x),d.get('Y',y),d.get('Z',z)
        if z<0 and nz<0 and (nx,ny)!=(x,y):
            if gs and gs[-1] in (2,3) and 'I' in d:
                cx,cy=x+d.get('I',0),y+d.get('J',0); r=math.hypot(x-cx,y-cy)
                a0=math.atan2(y-cy,x-cx); a1=math.atan2(ny-cy,nx-cx)
                sw=(a1-a0)%(2*math.pi)
                if gs[-1]==2: sw=sw-2*math.pi
                if abs(sw)<1e-9: sw=2*math.pi*(1 if gs[-1]==3 else -1)
                n=max(4,int(abs(sw)*r/0.05))
                pts=[(cx+r*math.cos(a0+sw*i/n),cy+r*math.sin(a0+sw*i/n)) for i in range(n+1)]
                for a,b in zip(pts,pts[1:]): cuts.append((a,b))
            else: cuts.append(((x,y),(nx,ny)))
        x,y,z=nx,ny,nz
    return cuts,tool
def simulate(board, fn, mapf, tool=None, label=''):
    B=P.Board(board)
    cuts,t=gcode_cuts(fn); tool=tool or t
    cut=np.zeros((B.H,B.W),bool)
    for a,b in cuts:
        A=mapf(*a); C=mapf(*b)
        sl,m=B.seg_mask(A,C,tool/2); cut[sl]|=m
    remain=B.inside & ~cut
    lab,n=ndimage.label(remain)
    layer=label
    comp={}; oncut=[]
    for p in B.d['pads']:
        if layer not in p['cu'] or p['type']=='np_thru_hole' or not p['net']: continue
        i,j=int((p['x']-B.x0)*RES),int((p['y']-B.y0)*RES)
        c=lab[j,i]
        if c==0: oncut.append(f"{p['ref']}.{p['num']}"); continue
        comp.setdefault(c,set()).add(p['net'])
    shorts=[sorted(v) for v in comp.values() if len(v)>1]
    nets={}
    for c,v in comp.items():
        for nnet in v: nets.setdefault(nnet,set()).add(c)
    # sítě rozpadlé: síť ve více komponentách (jen pokud v DPS na této vrstvě má spojení – orientačně)
    split=[k for k,v in nets.items() if len(v)>1]
    # zásah do mědi: řez přes měď vrstvy
    Cu=B.copper(layer)>0
    hit=(cut&Cu).sum()/RES/RES
    return dict(cuts=len(cuts),tool=tool,pads_on_cut=oncut,shorts=shorts,split=split,cut_into_copper_mm2=round(hit,3),B=B,cut=cut)
