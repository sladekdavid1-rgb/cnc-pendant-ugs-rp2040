import re
def parse(txt):
    toks=re.findall(r'\(|\)|"(?:[^"\\]|\\.)*"|[^\s()]+',txt)
    st=[[]]
    for t in toks:
        if t=='(': st.append([])
        elif t==')': x=st.pop(); st[-1].append(x)
        else: st[-1].append(t)
    return st[0]
def unq(s): return s[1:-1].replace('\\"','"') if s.startswith('"') else s
def q(s): return '"'+str(s).replace('\\','\\\\').replace('"','\\"')+'"'
def dump(n,ind=0):
    if not isinstance(n,list): return n
    if all(not isinstance(c,list) for c in n):
        return "("+" ".join(n)+")"
    head=[]; i=0
    while i<len(n) and not isinstance(n[i],list): head.append(n[i]); i+=1
    s="("+" ".join(head)
    for c in n[i:]:
        s+="\n"+"\t"*(ind+1)+(dump(c,ind+1) if isinstance(c,list) else c)
    return s+"\n"+"\t"*ind+")"
def find(n,key): return [c for c in n if isinstance(c,list) and c and c[0]==key]
def first(n,key):
    r=find(n,key); return r[0] if r else None
LIBDIR=""  # nepoužívá se
_cache={}
def libsym(lib,name):
    if lib not in _cache: _cache[lib]=parse(open(f"{LIBDIR}/{lib}.kicad_sym").read())[0]
    for s in find(_cache[lib],"symbol"):
        if unq(s[1])==name: return s
def flat(lib,name):
    """vrátí kopii symbolu s rozbaleným extends a názvem lib:name"""
    import copy
    s=copy.deepcopy(libsym(lib,name))
    ext=first(s,"extends")
    if ext:
        parent=copy.deepcopy(libsym(lib,unq(ext[1])))
        pname=unq(parent[1])
        props={unq(p[1]):p for p in find(s,"property")}
        new=[parent[0],q(name)]
        for c in parent[2:]:
            if isinstance(c,list) and c[0]=="property" and unq(c[1]) in props: new.append(props[unq(c[1])])
            elif isinstance(c,list) and c[0]=="symbol":
                c[1]=q(unq(c[1]).replace(pname,name,1)); new.append(c)
            else: new.append(c)
        s=new
    s[1]=q(f"{lib}:{name}")
    return s
def pins(sym):
    out=[]
    for sub in find(sym,"symbol"):
        for p in find(sub,"pin"):
            at=first(p,"at"); 
            out.append(dict(num=unq(first(p,"number")[1]),name=unq(first(p,"name")[1]),x=float(at[1]),y=float(at[2]),ang=float(at[3]) if len(at)>3 else 0.0,
                            len=float(first(p,"length")[1]),type=p[1]))
    return out
