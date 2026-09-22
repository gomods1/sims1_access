"""Dump BHAV chunks of an IFF with SimAntics expressions decoded (FreeSO's VMExpression layout).
  python tools/bhav_dump.py <file.iff> [label substring ...]
"""
import struct, sys
sys.path.insert(0, __import__('os').path.dirname(__file__))
import simsfmt
SCOPES = {0:'MyAttr',1:'StackAttr',2:'TargetAttr',3:'MyObj',4:'StackObj',5:'TargetObj',6:'Global',7:'Lit',8:'Temp',9:'Param',10:'StackObjID',11:'TempByTemp',12:'TreeAdRange',13:'StackObjTemp',14:'MyMotive',15:'StackMotive',16:'StackSlot',17:'StackMotiveByTemp',18:'MyPersonData',19:'StackPersonData',20:'MySlot',21:'StackObjDef',22:'StackAttrByParam',23:'RoomByTemp0',24:'NeighborInStackObj',25:'Local',26:'Tuning',27:'DynSpriteFlag',28:'TreeAdPersonality',29:'TreeAdMin',30:'MyPersonDataByTemp',31:'StackPersonDataByTemp',32:'NeighborPersonData',33:'JobData',34:'NeighborhoodData',35:'StackObjFunction',36:'MyTypeAttr',37:'StackTypeAttr',38:'NeighborsPersonData'}
OPS = {0:':=',1:'+=',2:'-=',3:'*=',4:'/=',5:'==',6:'!=',7:'<',8:'>',9:'<=',10:'>=',11:'setflag',12:'clearflag',13:'++<',14:'--<',15:'push',16:'pop',17:'%=',18:'&=',19:'|=',20:'^=',21:'>>=',22:'<<=',23:':=abs',24:'sqrt',25:'ctan',26:'atan',27:'rand'}
def scope(o, d): return f"{SCOPES.get(o, o)}[{d}]"
def main(argv):
    data = open(argv[0], 'rb').read()
    want = argv[1:]
    for c in simsfmt.read_iff(data):
        if c.type != 'BHAV' or (want and not any(w.lower() in c.label.lower() for w in want)): continue
        d = c.data; hdr = struct.unpack_from('<HHHHHHH', d, 0); off = 12 if hdr[0] < 0x8003 else 14
        print(f"BHAV {c.id} '{c.label}' count {hdr[1]} type {hdr[2]} args {hdr[3]} locals {hdr[4]}")
        for i in range(hdr[1]):
            op, t, f = struct.unpack_from('<HBB', d, off + i * 12); ops = d[off + i * 12 + 4:off + i * 12 + 12]
            tgt = lambda x: {253: 'false', 254: 'true', 255: 'error'}.get(x, str(x))
            if op == 2:
                ld, rd, sg, o, lo, ro = struct.unpack_from('<HHBBBB', ops)
                text = f"{scope(lo, ld)} {OPS.get(o, o)} {scope(ro, rd)}"
            else:
                text = f"op {op} {ops.hex()}"
            print(f"  {i:2d}: {text:48s} T {tgt(t):5s} F {tgt(f)}")
main(sys.argv[1:])
