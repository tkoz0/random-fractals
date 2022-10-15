
# these are ordered a,d,b,e,c,f in the .xml flam3 files
AffineParams = tuple[float,float,float,float,float,float] # a,b,c,d,e,f

Point = tuple[float,float] # x,y
RGBColor = tuple[int,int,int] # r,g,b

# maps (x,y) -> (x,y)
null_affine: AffineParams = (1.0,0.0,0.0,0.0,1.0,0.0)

_emach32 = 2**-23
_emach64 = 2**-52

_eps = 1e-10 # value defined in the flam3 source for avoiding division by zero
_emach = _emach64 # python float is 64 bit
