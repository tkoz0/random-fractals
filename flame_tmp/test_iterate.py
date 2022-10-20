import math
import random
import png
from typing import Callable

color_t = tuple[int,int,int]
func_t = Callable[[float,float],tuple[float,float]]
point_t = tuple[float,float]

def biunit_rand() -> point_t:
    x = random.random()*2-1
    y = random.random()*2-1
    return x,y

def color_rand() -> color_t:
    r = random.randint(0,255)
    g = random.randint(0,255)
    b = random.randint(0,255)
    return r,g,b

sierpinski: list[tuple[float,color_t,func_t]] = [
    (1.0/3.0, (255,0,0), lambda x,y: (x/2.0,y/2.0)),
    (1.0/3.0, (0,255,0), lambda x,y: ((x+1.0)/2.0,y/2.0)),
    (1.0/3.0, (0,0,255), lambda x,y: (x/2.0,(y+1.0)/2.0))
]

barnsley_fern: list[tuple[float,color_t,func_t]] = [
    (0.01, (255,0,0),   lambda x,y: (0.0,0.16*y)),
    (0.85, (0,255,0),   lambda x,y: (0.85*x+0.04*y,-0.04*x+0.85*y+1.60)),
    (0.07, (0,0,255),   lambda x,y: (0.20*x-0.26*y,0.23*x+0.22*y+1.60)),
    (0.07, (255,255,0), lambda x,y: (-0.15*x+0.28*y,0.26*x+0.24*y+0.44))
]

test_xforms_1: list[tuple[float,color_t,func_t]] = [
    (0.14, (255,0,0), lambda x,y: ((y+0.6)/3.1+0.4,(x*x-y-1.0)/(math.cos(x)*x+30.0)+0.4)),
    (0.09, (0,255,0), lambda x,y: (0.4*math.cos(x)*math.exp(math.sin(x))/(y*y)-math.cos(0.3*y)/2.0-0.3,(-x-1.2*y)/26.0-0.3)),
    (0.17, (0,0,255), lambda x,y: (math.exp(0.2*(-x*x))/(x*y*y)+0.1,math.cos(math.sin(y)*y)+1.5)),
    (0.07, (255,0,255), lambda x,y: (math.tan(-x+1.0),math.sin(x)))
]

test_xforms_2: list[tuple[float,color_t,func_t]] = [
    (1.0/4.0, (255,0,0), lambda x,y: (x/3.0,y/3.0)),
    (1.0/4.0, (0,255,0), lambda x,y: ((x+1.5)/2.0,y/2.0)),
    (1.0/4.0, (0,0,255), lambda x,y: (x/2.0,(y+1.5)/2.0)),
    (1.0/4.0, (255,0,255), lambda x,y: (-(x+1.5)/2.0,-(y+1.5)/2.0))
]

test_xforms_3: list[tuple[float,color_t,func_t]] = [
    (0.5, (255,0,0), lambda x,y: (1.5*x-2.0*y+0.5,-0.5*x+1.5*y-2.0)),
    (1.0, (0,255,0), lambda x,y: (-x/5.0,y/5.0))
]

test_xforms_4: list[tuple[float,color_t,func_t]] = [
    (1.0, (255,0,0), lambda x,y: (x,y))
]

XLO,XHI = -2.0,2.0
YLO,YHI = -2.0,2.0
WIDTH = 800
HEIGHT = 800
N_ITERS = 5000000
SETTLE_ITERS = 20
xforms = test_xforms_2

xstep,ystep = (XHI-XLO)/WIDTH,(YHI-YLO)/HEIGHT
weights = [z[0] for z in xforms]
print(f'weights {weights}')
sum_weights = sum(weights)
weights = [w/sum_weights for w in weights]
print(f'normalized weights {weights}')
cw = [] # cumulative weights for probability selection
s = 0.0
for w in weights:
    s += w
    cw.append(s)
if cw[-1] != 1.0:
    cw[-1] = 1.0
print(f'cumulative weights: {cw}')

buf = [[0]*(3*WIDTH) for _ in range(HEIGHT)]

x,y = biunit_rand()
color = color_rand()
print(f'initial point {x} {y}')
print(f'initial color {color}')
xmin,ymin = math.inf,math.inf
xmax,ymax = -math.inf,-math.inf

for i in range(N_ITERS+SETTLE_ITERS):
    rand = random.random()
    j = len([z for z in cw if z < rand])
    _,(cr,cg,cb),func = xforms[j]
    ocr,ocg,ocb = color
    x,y = func(x,y)
    # FIXME calculate properly
    r = HEIGHT-1-math.floor((y-YLO)/ystep)
    c = math.floor((x-XLO)/xstep)
    ocr = (ocr+cr)//2
    ocg = (ocg+cg)//2
    ocb = (ocb+cb)//2
    color = (ocr,ocg,ocb)
    if i >= SETTLE_ITERS:
        xmin = min(xmin,x)
        ymin = min(ymin,y)
        xmax = max(xmax,x)
        ymax = max(ymax,y)
        if 0 <= r < HEIGHT and 0 <= c < WIDTH:
            buf[r][3*c+0] = ocr
            buf[r][3*c+1] = ocg
            buf[r][3*c+2] = ocb

print(f'xrange {xmin} {xmax}')
print(f'yrange {ymin} {ymax}')

img = png.from_array(buf,'RGB')
img.save('a.png')
