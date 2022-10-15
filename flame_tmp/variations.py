# variation functions

import math
import random

AffineParams = tuple[float,float,float,float,float,float] # a,b,c,d,e,f
Point = tuple[float,float] # x,y
_eps = 1e-10 # value defined in the flam3 source for avoiding division by zero

# in flam3 source, affine parameters
# xform->c[0][0] = a
# xform->c[0][1] = d
# xform->c[1][0] = b
# xform->c[1][1] = e
# xform->c[2][0] = c
# xform->c[2][1] = f

# helper functions
#
# r = radius
# theta = angle of (x,y)
# phi = angle of (y,x)
# psi = random in [0,1)
# omega = pi or 0
# lambda = 1 or -1

def _r2(x: float, y: float) -> float:
    ''' squared 2-norm '''
    return x*x+y*y

def _r(x: float, y: float) -> float:
    ''' 2-norm '''
    return math.sqrt(_r2(x,y))

def _theta(x: float, y: float) -> float:
    ''' atan2(x,y) '''
    return math.atan2(x,y)

def _phi(x: float, y: float) -> float:
    ''' atan2(y,x) '''
    return math.atan2(y,x)

def _psi():
    ''' random in [0,1) '''
    return random.random()

def _omega() -> float:
    ''' 0 or pi '''
    return random.choice([math.pi,0.0])

def _lambda() -> float:
    ''' 1 or -1 '''
    return random.choice([1.0,-1.0])

# base variation class
class Variation:
    def __init__(self):
        raise Exception('do not instantiate Variation, override base class')
    def calc(self, x: float, y: float) -> Point:
        raise NotImplementedError()

class VarLinear(Variation): # 0
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        return x,y

class VarSinusoidal(Variation): # 1
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        return math.sin(x),math.sin(y)

class VarSpherical(Variation): # 2
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r2 = _r2(x,y) + _eps
        return x/r2,y/r2

class VarSwirl(Variation): # 3
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r2 = _r2(x,y)
        s,c = math.sin(r2),math.cos(r2)
        return x*s-y*c,x*c+y*s

class VarHorseshoe(Variation): # 4
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y) + _eps
        return (x-y)*(x+y)/r,2.0*x*y/r

class VarPolar(Variation): # 5
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        t = _theta(x,y)
        r = _r(x,y)
        return t/math.pi,r-1.0

class VarHandkerchief(Variation): # 6
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y)
        t = _theta(x,y)
        return r*math.sin(t+r),r*math.cos(t-r)

class VarHeart(Variation): # 7
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y)
        t = _theta(x,y)
        return r*math.sin(t*r),-r*math.cos(t*r)

class VarDisc(Variation): # 8
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y)
        t = _theta(x,y)
        return t*math.sin(math.pi*r)/math.pi,t*math.cos(math.pi*r)/math.pi

class VarSpiral(Variation): # 9
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y) + _eps
        t = _theta(x,y)
        return (math.cos(t)+math.sin(r))/r,(math.sin(t)-math.cos(r))/r

class VarHyperbolic(Variation): # 10
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y) + _eps
        t = _theta(x,y)
        return math.sin(t)/r,r*math.cos(t)

class VarDiamond(Variation): # 11
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y)
        t = _theta(x,y)
        return math.sin(t)*math.cos(r),math.cos(t)*math.sin(r)

class VarEx(Variation): # 12
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = _r(x,y)
        t = _theta(x,y)
        p0 = math.sin(t+r)
        p1 = math.sin(t-r)
        return r*(p0**3+p1**3),r*(p0**3-p1**3)

class VarJulia(Variation): # 13
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        t = _theta(x,y)
        sr = math.sqrt(_r(x,y))
        o = _omega()
        return sr*math.cos(t/2.0+o),sr*math.sin(t/2.0+o)

class VarBent(Variation): # 14
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        xr = x if x >= 0.0 else 2.0*x
        yr = y if y >= 0.0 else y/2.0
        return xr,yr

class VarWaves(Variation): # 15
    def __init__(self, af: AffineParams):
        self.af = af
    def calc(self, x: float, y: float) -> Point:
        _,b,c,_,e,f = self.af
        return x+b*math.sin(y/(c*c+_eps)),y+e*math.sin(x/(f*f+_eps))

class VarFisheye(Variation): # 16
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = 2.0/(_r(x,y)+1.0)
        return r*y,r*x

class VarPopcorn(Variation): # 17
    def __init__(self, af: AffineParams):
        self.af = af
    def calc(self, x: float, y: float) -> Point:
        _,_,c,_,_,f = self.af
        return x+c*math.sin(math.tan(3*y)),y+f*math.sin(math.tan(3*x))

class VarExponential(Variation): # 18
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        r = math.exp(x-1.0)
        a = math.pi*y
        return r*math.cos(a),r*math.sin(a)

class VarPower(Variation): # 19
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        t = _theta(x,y)
        r = math.pow(_r(x,y),math.sin(t))
        return r*math.cos(t),r*math.sin(t)

class VarCosine(Variation): # 20
    def __init__(self):
        pass
    def calc(self, x: float, y: float) -> Point:
        a = math.pi*x
        return math.cos(a)*math.cosh(y),-math.sin(a)*math.sinh(y)

class VarRings(Variation): # 21
    def __init__(self, af: AffineParams):
        self.af = af
    def calc(self, x: float, y: float) -> Point:
        a,b,c,d,e,f = self.af
        dx = c*c+_eps
        r = _r(x,y)
        z = math.fmod(r+dx,2*dx)-dx+r*(1.0-dx)
        t = _theta(x,y)
        return z*math.cos(t),z*math.sin(t)
