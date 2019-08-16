import numpy as np
from scipy.interpolate import interp1d

class Fluid:
    def get_viscosity(self, *args):
        raise Exception("Not implemented error")


class InterpFluid(Fluid):

    def __init__(self, x, y):
        self.interpf = interp1d(x, y, 5, fill_value='extrapolate')

    def get_viscosity(self, T, *args):
        return self.interpf(T)


class ChengFluid(Fluid):

    def __init__(self, A, B, C, D):
        self.A = A
        self.B = B
        self.C = C
        self.D = D
    
    def get_viscosity(self, T, *dummy):
        num = np.multiply(np.subtract(self.B, T), T)
        den = np.add(self.C, np.multiply(self.D, T))
        return np.multiply(self.A, np.exp(np.divide(num, den)))


class ChengSolution(Fluid):

    def __init__(self, glycerol, water):
        self.glycerol = glycerol
        self.water = water

    def get_viscosity(self, T=None, Cm=None, **kwargs):
        a = np.subtract(0.705, np.multiply(0.0017, T))
        b = np.multiply(np.add(4.9, np.multiply(0.036, T)), np.power(a, 2.5))
        num = np.multiply(np.multiply(a, b), np.multiply(Cm, np.subtract(1, Cm)))
        den = np.add(np.multiply(a, Cm), np.multiply(b, np.subtract(1, Cm)))
        alpha = np.add(np.subtract(1, Cm), np.divide(num, den))
        muw = self.water.get_viscosity(T=T)
        mug = self.glycerol.get_viscosity(T=T)
        mum = np.multiply(np.power(muw, alpha), np.power(mug, np.subtract(1, alpha)))
        return mum

s600 = InterpFluid(
        [20,25,37.78,40,50,60,80,98.89,100],
        [1.945,1.309,0.5277,0.4572,0.2511,0.1478,0.06091,0.03122,0.03017]
    )
glycerol = ChengFluid(12.100, -1233, 9900, 70)
water = ChengFluid(0.00179, -1230, 36100, 360)
glycerol_water_solution = ChengSolution(glycerol, water)








