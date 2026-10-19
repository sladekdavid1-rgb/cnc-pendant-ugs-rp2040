import os
import pcbcheck as P, numpy as np, math, sys
from PIL import Image, ImageDraw, ImageFont
from scipy import ndimage
RES=P.RES; OUT=20  # výstup px/mm
F=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',17)
FB=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf',30)
def zone_mask(B,layer):
    m=np.zeros((B.H,B.W),bool)
    for z in B.d['zones']:
        if z['l']!=layer: continue
        img=Image.new('L',(B.W,B.H),0); ImageDraw.Draw(img).polygon([B.px(p) for p in z['pts']],fill=1); m|=np.array(img,bool)
    return m
def render(pcb, out, title, layers=('B.Cu','F.Cu'), mirror=False, refs=False, fade=False):
    B=P.Board(pcb)
    H,W=B.H,B.W
    img=np.zeros((H,W,3),float); img[:]=(250,250,248)
    board=B.inside
    img[board]=(214,201,150)          # FR4
    col={'F.Cu':np.array((200,70,40)),'B.Cu':np.array((40,90,190))}
    for L in layers:
        cu=B.copper(L)>0; z=zone_mask(B,L)
        a=0.85 if len(layers)==1 or L=='F.Cu' else 0.75
        c=col[L]
        img[cu&~z]=img[cu&~z]*(1-a)+c*a
        zz=cu&z; img[zz]=img[zz]*(1-0.3)+c*0.3
    # výřezy (kruh v Edge.Cuts) = díra
    for k,g in B.d['edge']:
        if k=='gr_circle':
            c=g['center']; r=math.dist(c,g['end']); sl,m=B.seg_mask(c,c,r); img[sl][m]=(250,250,248)
    # otvory
    for p in B.d['pads']:
        if p['drill']>0:
            sl,m=B.seg_mask((p['x'],p['y']),(p['x'],p['y']),p['drill']/2); img[sl][m]=(30,30,30)
    for v in B.d['vias']:
        sl,m=B.seg_mask((v['x'],v['y']),(v['x'],v['y']),v['drill']/2); img[sl][m]=(30,30,30)
    # obrys a kruh
    if fade: img=img*0.45+np.array((250,250,248))*0.55
    im=Image.fromarray(img.astype(np.uint8))
    d=ImageDraw.Draw(im)
    for k,g in B.d['edge']:
        q=P.edge_pts([(k,g)],0.05); d.line([B.px(p) for p in q],fill=(60,60,60),width=4)
    im=im.resize((int(W*OUT/RES),int(H*OUT/RES)),Image.LANCZOS)
    if mirror: im=im.transpose(Image.FLIP_LEFT_RIGHT)
    d=ImageDraw.Draw(im)
    if refs:
        import pcbcheck
        t=B.d['t']
        from sx import find, first, unq
        for fp in find(t,'footprint'):
            ref=[unq(p[2]) for p in find(fp,'property') if unq(p[1])=='Reference'][0]
            if ref.startswith('H'): continue
            at=first(fp,'at'); x,y=(float(at[1])-B.x0)*OUT,(float(at[2])-B.y0)*OUT
            if mirror: x=im.width-x
            side=unq(first(fp,'layer')[1])
            tb=d.textbbox((0,0),ref,font=F); tw,th=tb[2]-tb[0],tb[3]-tb[1]
            d.rectangle((x-tw/2-3,y-th/2-3,x+tw/2+3,y+th/2+5),fill=(255,255,255,200))
            d.text((x-tw/2,y-th/2-2),ref,fill=(0,0,0) if side=='F.Cu' else (20,50,150),font=F)
    # titulek
    pad=70; can=Image.new('RGB',(im.width,im.height+pad),(250,250,248)); can.paste(im,(0,pad))
    dd=ImageDraw.Draw(can); dd.text((12,14),title,fill=(20,20,20),font=FB)
    can.save(out); print(out,can.size)
if __name__=='__main__':
    R=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'); o=sys.argv[1] if len(sys.argv)>1 else os.path.join(R,'00_obrazky')
    A=f'{R}/01_DPS_KiCad/pendant_A_horni/pendant_A_horni.kicad_pcb'; Bp=f'{R}/01_DPS_KiCad/pendant_B_spodni/pendant_B_spodni.kicad_pcb'; Cp=f'{R}/01_DPS_KiCad/pendant_C_led/pendant_C_led.kicad_pcb'
    render(A,f'{o}/DPS_A_obe_vrstvy.png','Deska A – obě vrstvy (červená = horní F.Cu, modrá = spodní B.Cu), pohled shora')
    render(A,f'{o}/DPS_A_horni_med.png','Deska A – horní měď (F.Cu), pohled shora',layers=('F.Cu',))
    render(A,f'{o}/DPS_A_spodni_med.png','Deska A – spodní měď (B.Cu), pohled shora (prosvícená)',layers=('B.Cu',))
    render(A,f'{o}/DPS_A_rozmisteni.png','Deska A – rozmístění (černě = horní strana, modře = spodní strana)',refs=True,fade=True)
    render(Bp,f'{o}/DPS_B_med.png','Deska B – měď (B.Cu), pohled shora (prosvícená)',layers=('B.Cu',))
    render(Bp,f'{o}/DPS_B_rozmisteni.png','Deska B – rozmístění (černě = horní strana, modře = spodní strana)',layers=('B.Cu',),refs=True,fade=True)
    render(Cp,f'{o}/DPS_C_med.png','Deska C – měď (F.Cu), pohled shora',layers=('F.Cu',))
    render(Cp,f'{o}/DPS_C_rozmisteni.png','Deska C – rozmístění',layers=('F.Cu',),refs=True,fade=True)
