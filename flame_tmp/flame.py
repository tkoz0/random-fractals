import math
import random

from variations import Variation

# these are ordered a,d,b,e,c,f in the .xml flam3 files
AffineParams = tuple[float,float,float,float,float,float] # a,b,c,d,e,f

Point = tuple[float,float] # x,y
RGBColorFloat = tuple[float,float,float]
RGBColorInt = tuple[int,int,int] # r,g,b
HistogramBasic = list[list[int]]

# maps (x,y) -> (x,y)
null_affine: AffineParams = (1.0,0.0,0.0,0.0,1.0,0.0)

_emach32 = 2**-23
_emach64 = 2**-52

_eps = 1e-10 # value defined in the flam3 source for avoiding division by zero
_emach = _emach64 # python float is 64 bit

# TODO add color blending mode in addition to color index in palette

class XForm:
    # weight = probability of choosing (must normalize)
    # color_index = index in palette (scaled from 0.0 to 1.0)
    # color_speed = measure of how much a point is pulled toward the color
    # - must be in [0,1] to guarantee colors stay in range [0,1]
    # - TODO negative may be allowed, pulling away from the color
    # opacity = how much it contributes to final image
    # variations = list of weights and variations applied
    # pre_affine = affine transformation applied before variations
    # post_affine = affine transformation applied after variations
    # TODO xaos = advanced control for random xform selection
    # note: final xform has all params except weight and xaos
    def __init__(self,
            weight: float = 1.0,
            color: RGBColorFloat = (1.0,1.0,1.0),
            color_index: float = 0.5,
            color_speed: float = 0.5,
            opacity: float = 1.0,
            variations: list[tuple[float,Variation]] = [],
            pre_affine: AffineParams = null_affine,
            post_affine: AffineParams = null_affine):
        assert 0.0 < weight
        assert 0.0 <= color_index <= 1.0
        assert 0.0 <= color_speed <= 1.0
        assert 0.0 <= opacity <= 1.0
        self.weight = weight
        self.color = color
        self.color_index = color_index
        self.color_speed = color_speed
        self.pre_affine = pre_affine
        self.post_affine = post_affine
        self.variations = variations

# color palette
class Palette:
    # colors = list of RGB data
    def __init__(self, colors: list[RGBColorInt]):
        self.colors = colors
    def to_color(self, c: float) -> RGBColorInt:
        assert 0.0 <= c <= 1.0
        return self.colors[math.floor(c*len(self.colors)-1)]

# TODO
# filters for final image
class Filters:
    def __init__(self):
        pass

class Flame:
    # name = string identifier
    # size_x = pixel width
    # size_y = pixel height
    # xrange = coordinate range on x axis
    # yrange = coordinate range on y axis
    # xforms = list of xforms to apply
    def __init__(self,
            name: str = '',
            size_x: int = 512,
            size_y: int = 512,
            xrange: Point = (-2.0,2.0),
            yrange: Point = (-2.0,2.0),
            xforms: list[XForm] = [],
            samples: int = 1000000,
            palette: Palette = Palette([(0,0,0)]*256)):
        assert 0 < size_x < 10000
        assert 0 < size_y < 10000
        assert -1024.0 < xrange[0] < xrange[1] < 1024.0
        assert -1024.0 < yrange[0] < yrange[1] < 1024.0
        assert 0 < samples
        self.name = name
        self.size_x = size_x
        self.size_y = size_y
        self.xmin, self.xmax = xrange
        self.ymin, self.ymax = yrange
        self.xforms = xforms
        self.samples = samples
        self.palette = palette

def biunit_rand(s = 1.0) -> Point:
    x = s*(random.random()*2 - 1)
    y = s*(random.random()*2 - 1)
    return x,y

# notes
# - color update:
#   - newindex = speed*(index of current xform) + (1-speed)*oldindex
#   - blends index in the palette

def apply_xform_basic(x: float, y: float, xf: XForm) -> Point:
    # pre affine transform
    a,b,c,d,e,f = xf.pre_affine
    tx = a*x + b*y + c
    ty = d*x + e*y + f
    # sum variations
    vx,vy = 0.0,0.0
    for w,var in xf.variations:
        rx,ry = var.calc(tx,ty)
        vx += w*rx
        vy += w*ry
    # post affine transform
    a,b,c,d,e,f = xf.post_affine
    ox = a*vx + b*vy + c
    oy = d*vx + e*vy + f
    #speed = xf.color_speed
    return ox,oy#,xf.color_index*speed+(1.0-speed)*c

SETTLE_ITERS = 20

def render_basic(flame: Flame) -> HistogramBasic:
    R,C = flame.size_x,flame.size_y
    xlo,xhi = flame.xmin,flame.xmax
    ylo,yhi = flame.ymin,flame.ymax
    xstep,ystep = (xhi-xlo)/C,(yhi-ylo)/R
    buf: HistogramBasic = [[0]*C for _ in range(R)]
    x,y = biunit_rand()
    W = [xform.weight for xform in flame.xforms]
    SW = sum(W)
    W = [z/SW for z in W] # normalize weights
    CW = [] # cumulative
    s = 0.0
    for w in W:
        s += w
        CW.append(s)
    xmin,ymin = math.inf,math.inf
    xmax,ymax = -math.inf,-math.inf
    CW[-1] = 1.0 # ensure ends with 1.0 for proper random selection
    for i in range(SETTLE_ITERS+flame.samples):
        xf = flame.xforms[len([z for z in CW if z < random.random()])]
        x,y = apply_xform_basic(x,y,xf)
        if i >= SETTLE_ITERS:
            xmin = min(xmin,x)
            ymin = min(ymin,y)
            xmax = max(xmax,x)
            ymax = max(ymax,y)
            r = R-1-math.floor((y-ylo)/ystep)
            c = math.floor((x-xlo)/xstep)
            if 0 <= r < R and 0 <= c < C:
                buf[r][c] += 1
    return buf

