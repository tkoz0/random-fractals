import math
import png

from flame import Flame, XForm, render_basic
from variations import *

xforms = [
    XForm(weight=1.0,variations=[(1.0,VarLinear()),(-0.5,VarSpherical())],pre_affine=(0.5,0.0,0.0,0.0,0.5,0.0)),
    XForm(weight=1.0,variations=[(1.0,VarLinear()),(0.0,VarSinusoidal())],pre_affine=(0.5,0.0,0.5,0.0,0.5,0.0)),
    XForm(weight=1.0,variations=[(1.0,VarLinear()),(0.0,VarHyperbolic())],pre_affine=(0.5,0.0,0.0,0.0,0.5,0.5))
]

flame = Flame(xforms=xforms,xrange=(-1.0,1.0),yrange=(-1.0,1.0),samples=2**24)

buf = render_basic(flame)
print(f'sum histogram = {sum(sum(row) for row in buf)}')
print(f'max histogram value = {max(max(row) for row in buf)}')
buf = [[math.log(col+1) for col in row] for row in buf]
scale = max(max(row) for row in buf)
print(f'log scale to = {scale}')

def to_pix_value(f: float) -> list[int]:
    n = math.floor(255.5*f/scale)
    n = n if n < 256 else 255
    return [n]

buf = [sum([to_pix_value(col) for col in row],[])  for row in buf]

img = png.from_array(buf,'L')
#img = png.from_array(buf,'RGB')
img.save('b.png')
