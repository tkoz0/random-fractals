import math
import random

point = tuple[float,float]
affine = tuple[float,float,float,float,float,float]

# helper functions

def _r(x: float, y: float) -> float:
    return math.sqrt(x*x+y*y)

def _theta(x: float, y: float) -> float:
    return math.atan(x/y)

def _phi(x: float, y: float) -> float:
    return math.atan(y/x)

def _omega() -> float:
    return math.pi if random.choice([False,True]) else 0.0

def _lambda() -> float:
    return 1.0 if random.choice([False,True]) else -1.0

def _psi() -> float:
    return random.random()

# variations

def linear(x: float, y: float) -> point:
    return x,y

def sinusoidal(x: float, y: float) -> point:
    return math.sin(x),math.sin(y)

def spherical(x: float, y: float) -> point:
    r = _r(x,y)
    r2 = r*r
    return x/r2,y/r2

def swirl(x: float, y: float) -> point:
    r = _r(x,y)
    r2 = r*r
    return x*math.sin(r2)-y*math.cos(r2),x*math.cos(r2)+y*math.sin(r2)

def horseshoe(x: float, y: float) -> point:
    r = _r(x,y)
    return (x-y)*(x+y)/r,2*x*y/r

def polar(x: float, y: float) -> point:
    theta = _theta(x,y)
    r = _r(x,y)
    return theta/math.pi,r-1

def handkerchief(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _theta(x,y)
    return r*math.sin(theta+r),r*math.cos(theta-r)

def heart(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _theta(x,y)
    return r*math.sin(theta*r),-r*math.cos(theta*r)

def disc(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _theta(x,y)
    return theta*math.sin(math.pi*r)/math.pi,theta*math.cos(math.pi*r)/math.pi

def spiral(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _theta(x,y)
    return (math.cos(theta)+math.sin(r))/r,(math.sin(theta)-math.cos(r))/r

def hyperbolic(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _theta(x,y)
    return math.sin(theta)/r,r*math.cos(theta)

def diamond(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _r(x,y)
    return math.sin(theta)*math.cos(r),math.cos(theta)*math.sin(r)

def ex(x: float, y: float) -> point:
    r = _r(x,y)
    theta = _theta(x,y)
    p0 = math.sin(theta+r)
    p1 = math.cos(theta-r)
    return r*(p0**3+p1**3),r*(p0**3-p1**3)

def julia(x: float, y: float, omega: float = 0.0) -> point:
    theta = _theta(x,y)
    sr = math.sqrt(_r(x,y))
    return sr*math.cos(theta/2+omega),sr*math.sin(theta/2+omega)

def bent(x: float, y: float) -> point:
    xr = x if x >= 0.0 else 2*x
    yr = y if y >= 0.0 else y/2
    return xr,yr

def waves(x: float, y: float, af: affine) -> point:
    _,b,c,_,e,f = af
    return x+b*math.sin(y/(c*c)),y+e*math.sin(x/(f*f))

# x,y are switched
def fisheye(x: float, y: float) -> point:
    r = 2/(_r(x,y)+1)
    return r*y,r*x

def popcorn(x: float, y: float, af: affine) -> point:
    _,_,c,_,_,f = af
    return x+c*math.sin(math.tan(3*y)),y+f*math.sin(math.tan(3*x))

def exponential(x: float, y: float) -> point:
    r = math.exp(x-1)
    return r*math.cos(math.pi*y),r*math.sin(math.pi*y)

def power(x: float, y: float) -> point:
    theta = _theta(x,y)
    r = math.pow(_r(x,y),math.sin(theta))
    return r*math.cos(theta),r*math.sin(theta)

def cosine(x: float, y: float) -> point:
    return math.cos(math.pi*x)*math.cosh(y),-math.sin(math.pi*x)*math.sinh(y)

def rings(x: float, y: float, af: affine) -> point:
    _,_,c,_,_,_ = af
    r = _r(x,y)
    theta = _theta(x,y)
    # ((r + c^2) mod (2c^2) - c^2 + r*(1 - c^2)) * (cos(theta),sin(theta))
    # FIXME how the fuck does mod work with floating point
    assert 0
    return 0.0,0.0

def fan(x: float, y: float, af: affine) -> point:
    _,_,c,_,_,f = af
    t = math.pi*c*c
    r = _r(x,y)
    theta = _theta(x,y)
    # if (theta+f) mod t > t/2 then r*(cos(theta-t/2),sin(theta-t/2))
    # if (theta+f) mod t <= t/2 then r*(cos(theta+t/2),sin(theta+t/2))
    # FIXME how the fuck does mod work with floating point
    assert 0
    return 0.0,0.0

def blob(x: float, y: float, high: float, low: float, waves: float) -> point:
    p1 = high
    p2 = low
    p3 = waves
    r = _r(x,y)
    theta = _theta(x,y)
    r2 = p2 + ((p1-p2)/2)*(math.sin(p3*theta) + 1)
    return r*r2*math.cos(theta),r*r2*math.sin(theta)

def pdj(x: float, y: float, a: float, b: float, c: float, d: float) -> point:
    p1 = a
    p2 = b
    p3 = c
    p4 = d
    return math.sin(p1*y)-math.cos(p2*x),math.sin(p3*x)-math.cos(p4*y)

def fan2(x: float, y: float, px: float, py: float) -> point:
    p1 = math.pi*px*px
    p2 = py
    theta = _theta(x,y)
    t = theta + p2 - p1*math.floor(2*theta*p2/p1)
    r = _r(x,y)
    if t > p1/2:
        return r*math.sin(theta-p1/2),r*math.cos(theta-p1/2)
    else:
        return r*math.sin(theta+p1/2),r*math.cos(theta+p1/2)

def rings2(x: float, y: float, val: float) -> point:
    p = val*val
    r = _r(x,y)
    t = r - 2*p*math.floor((r+p)/(2*p)) + r*(1-p)
    theta = _theta(x,y)
    return t*math.sin(theta),t*math.cos(theta)

# corrects the order of x,y
def eyefish(x: float, y: float) -> point:
    r = 2/(_r(x,y)+1)
    return r*x,r*y

def bubble(x: float, y: float) -> point:
    r = 4/(_r(x,y)**2+4)
    return r*x,r*y

def cylinder(x: float, y: float) -> point:
    return math.sin(x),y

def perspective(x: float, y: float, angle: float, dist: float) -> point:
    p1 = angle
    p2 = dist
    r = p2/(p2-y*math.sin(p1))
    return r*x,r*y*math.cos(p1)

def noise(x: float, y: float) -> point:
    psi1 = _psi()
    psi2 = _psi()
    return psi1*x*math.cos(2*math.pi*psi2),psi1*y*math.sin(2*math.pi*psi2)

def julian(x: float, y: float, power: float, dist: float) -> point:
    p1 = power
    p2 = dist
    p3 = math.floor(abs(p1)*_psi())
    phi = _phi(x,y)
    t = (phi+2*math.pi*p3)/p1
    r = math.pow(_r(x,y),p2/p1)
    return r*math.cos(t),r*math.sin(t)

def juliascope(x: float, y: float, power: float, dist: float) -> point:
    p1 = power
    p2 = dist
    p3 = math.floor(abs(p1)*_psi())
    phi = _phi(x,y)
    t = (_lambda()*phi+2*math.pi*p3)/p1
    r = math.pow(_r(x,y),p2/p1)
    return r*math.cos(t),r*math.sin(t)

def blur(x: float, y: float) -> point:
    psi1 = _psi()
    psi2 = _psi()
    return psi1*math.cos(2*math.pi*psi2),psi1*math.sin(2*math.pi*psi2)

# 4 random numbers minus 2 is an estimate of a gaussian distribution
def gaussian(x: float, y: float) -> point:
    r = _psi() + _psi() + _psi() + _psi() - 2
    psi5 = _psi()
    return r*math.cos(2*math.pi*psi5),r*math.cos(2*math.pi*psi5)

def radialblur(x: float, y: float, angle: float) -> point:
    p1 = angle*math.pi/2
    # t1 = v_36 * (psi1 + psi2 + psi3 + psi4 - 2)
    # t2 = phi + t1*sin(p1)
    # t3 = t1*cos(p1) - 1
    # (1/v_36) * (r*cos(t2)+t3*x,r*sin(t2)+t3*y)
    # FIXME what the fuck is v_36
    assert 0
    return 0.0,0.0

# why does this not depend on x,y
def pie(x: float, y: float, slices: float, rotation: float, thickness: float) -> point:
    p1 = slices
    p2 = rotation
    p3 = thickness
    t1 = math.floor(_psi()*p1 + 0.5)
    t2 = p2 + (2*math.pi/p1)*(t1 + _psi()*p3)
    psi3 = _psi()
    return psi3*math.cos(t2),psi3*math.sin(t2)

def ngon(x: float, y: float, power: float, sides: float, corners: float, circle: float) -> point:
    p1 = power
    p2 = 2*math.pi/sides
    p3 = corners
    p4 = circle
    phi = _phi(x,y)
    t3 = phi - p2*math.floor(phi/p2)
    t4 = t3 if t3 > p2/2 else t3-p2
    r = _r(x,y)
    k = (p3*(1/math.cos(t4) - 1) + p4)/math.pow(r,p1)
    return k*x,k*y

def curl(x: float, y: float, c1: float, c2: float) -> point:
    p1 = c1
    p2 = c2
    t1 = 1 + p1*x + p2*(x*x-y*y)
    t2 = p1*y + 2*p2*x*y
    r = 1/(t1*t1+t2*t2)
    return r*(x*t1+y*t2),r*(y*t1-x*t2)

def rectangles(x: float, y: float, px: float, py: float) -> point:
    p1 = px
    p2 = py
    return (2*math.floor(x/p1)+1)*p1-x,(2*math.floor(y/p2)+1)*p2-y

def arch(x: float, y: float) -> point:
    psi = _psi()
    # sin(psi*pi*v_41),sin^2(psi*pi*v_41)/cos(psi*pi*v_41)
    # FIXME what the fuck is v_41
    assert 0
    return 0.0,0.0

def tangent(x: float, y: float) -> point:
    return math.sin(x)/math.cos(y),math.tan(y)

def square(x: float, y: float) -> point:
    return _psi()-0.5,_psi()-0.5

def rays(x: float, y: float) -> point:
    r2 = _r(x,y)
    # r = v_44 * tan(psi*pi*v_44) / r2
    # r*cos(x),r*sin(y)
    # FIXME what the fuck is v_44
    assert 0
    return 0.0,0.0

def blade(x: float, y: float) -> point:
    psi = _psi()
    r = _r(x,y)
    # x*(cos(psi*r*v_45)+sin(psi*r*v_45)),x*(cos(psi*r*v_45)-sin(psi*r*v_45))
    # FIXME what the fuck is v_45
    assert 0
    return 0.0,0.0

def secant(x: float, y: float) -> point:
    r = _r(x,y)
    # x,1/(v_46*cos(v_46*r))
    # FIXME what the fuck is v_46
    assert 0
    return 0.0,0.0

def twintrian(x: float, y: float) -> point:
    # t = log10(sin(psi*r*v_47)**2) + cos(psi*r*v_47)
    # x*(t,t-pi*sin(psi*r*v_47))
    # FIXME what the fuck is v_47
    assert 0
    return 0.0,0.0

def cross(x: float, y: float) -> point:
    r = math.sqrt(1/(x*x-y*y)**2)
    return r*x,r*y

variations = \
[
    linear,
    sinusoidal,
    spherical,
    swirl,
    horseshoe,
    polar,
    handkerchief,
    heart,
    disc,
    spiral,
    hyperbolic,
    diamond,
    ex,
    julia,
    bent,
    waves,
    fisheye,
    popcorn,
    exponential,
    power,
    cosine,
    rings,
    fan,
    blob,
    pdj,
    fan2,
    rings2,
    eyefish,
    bubble,
    cylinder,
    perspective,
    noise,
    julian,
    juliascope,
    blur,
    gaussian,
    radialblur,
    pie,
    ngon,
    curl,
    rectangles,
    arch,
    tangent,
    square,
    rays,
    blade,
    secant,
    twintrian,
    cross
]
