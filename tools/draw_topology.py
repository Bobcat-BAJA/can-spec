import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch
W,H=10.8,11.2
fig,ax=plt.subplots(figsize=(W*0.72,(H+0.4)*0.72)); ax.set_xlim(0,W); ax.set_ylim(-0.4,H); ax.axis("off")
TRUNK="#B7791F"; INK="#222"; SUB="#555"
C={"corner":("#E0F2F1","#00796B"),"core":("#E8EAF6","#3949AB"),"other":("#F5F5F5","#616161")}
tx=W/2; top,bot=9.75,2.75
ax.plot([tx,tx],[bot,top],color=TRUNK,lw=7,solid_capstyle="butt",zorder=1)
def endnode(y,name,l2,l3):
    w,h=5.4,1.05
    ax.add_patch(FancyBboxPatch((tx-w/2,y-h/2),w,h,boxstyle="round,pad=0.02,rounding_size=0.1",fc=C["core"][0],ec=C["core"][1],lw=2,zorder=3))
    ax.text(tx,y+0.27,name,ha="center",va="center",fontsize=11,fontweight="bold",color=INK,zorder=4)
    ax.text(tx,y,l2,ha="center",va="center",fontsize=8.5,color=INK,zorder=4)
    ax.text(tx,y-0.27,l3,ha="center",va="center",fontsize=8.5,fontweight="bold",color="#B71C1C",zorder=4)
endnode(top+0.45,"Hub (DAQ) · bus end","SYNC 0x080 · HB 0x701 · SD, XBee, IMU","120 Ω on board")
endnode(bot-0.45,"Rear-steer · bus end","0x010, 0x100–0x101 · HB 0x702","120 Ω on board (R11 fitted)")
bw,bh=3.85,0.92
def node(side,y,name,l2,l3,kind,dashed=False):
    fc,ec=C[kind]; x=0.35 if side<0 else W-0.35-bw
    ax.add_patch(FancyBboxPatch((x,y-bh/2),bw,bh,boxstyle="round,pad=0.02,rounding_size=0.1",fc=fc,ec=ec,lw=1.6,ls="--" if dashed else "-",zorder=3))
    cx=x+bw/2
    ax.text(cx,y+0.24,name,ha="center",va="center",fontsize=10.5,fontweight="bold",color=INK,zorder=4)
    ax.text(cx,y-0.02,l2,ha="center",va="center",fontsize=8.5,color=INK,zorder=4)
    ax.text(cx,y-0.25,l3,ha="center",va="center",fontsize=8,color=SUB,zorder=4)
    xe=x+bw if side<0 else x
    ax.plot([xe,tx],[y,y],color=TRUNK,lw=2.2,zorder=2)
    ax.plot([tx],[y],"o",ms=6,color=TRUNK,zorder=2)
rows=[ # y, left, right
 (8.85,("Front-left","0x320 · HB 0x709","wheel spd · shock · tie-rod","corner"),("Front-right","0x321 · HB 0x70A","wheel spd · shock · tie-rod","corner")),
 (7.12,("Cockpit","0x120–0x121 · HB 0x70D","steering · throttle · brakes","core"),("GPS (if built)","0x090, 0x410–0x412 · HB 0x707","","other",True)),
 (5.38,("Powertrain","0x200–0x201 · HB 0x706","engine/CVT speed · temps","core"),None),
 (3.65,("Rear-left","0x322 · HB 0x70B","wheel spd · shock · tie-rod","corner"),("Rear-right","0x323 · HB 0x70C","wheel spd · shock · tie-rod","corner")),
]
for y,l,r in rows:
    node(-1,y,*l)
    if r: node(1,y,*r)
# legend
ly=1.4
ax.plot([0.4,1.1],[ly,ly],color=TRUNK,lw=7,solid_capstyle="butt"); ax.text(1.25,ly,"Trunk: 12 V, GND, CAN-H, CAN-L (CAN pair twisted)",va="center",fontsize=8.5,color=INK)
ax.plot([0.4,1.1],[ly-0.4,ly-0.4],color=TRUNK,lw=2.2); ax.text(1.25,ly-0.4,"Drop: DT06-4S plug to the node's DT13-4P, under 0.3 m. Dashed = optional",va="center",fontsize=8.5,color=INK)
ax.text(0.4,ly-0.8,"Only the two end boards terminate (bus reads 60 Ω). Order is logical; the harness team sets the route.",va="center",fontsize=8.5,color=SUB)
ax.text(0.4,ly-1.15,"IDs are message IDs as seen on the bus. HB = heartbeat, 1 Hz, on 0x700 + node ID.",va="center",fontsize=8.5,color=SUB)
plt.savefig("topology_v0.4.png",dpi=200,bbox_inches="tight",facecolor="white")
