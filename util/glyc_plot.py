from matplotlib import pyplot as plt
from scipy.interpolate import interp1d
import numpy as np

import util

cm = np.linspace(0, 1, 100)
T = 20
visc = util.glycerol_water_solution.get_viscosity(Cm=cm, T=T)

interpf = interp1d(visc, cm)

fig = plt.figure(figsize=(11.69, 9.27))
plotf = plt.semilogy
plotf(cm, visc)
highlighted_viscosities = [0.2, 0.3, 0.4, 0.5, 1.0]
highlighted_cms = [0.85, 1.0]
for hcm in highlighted_cms:
    highlighted_viscosities.append(util.glycerol_water_solution.get_viscosity(Cm=hcm, T=T))
highlighted_viscosities = sorted(highlighted_viscosities)
plt.ylim(bottom=min(highlighted_viscosities)-0.07)
plt.xlim(interpf(min(highlighted_viscosities)) - 0.12, right=1.02)
ymin, ymax = plt.ylim()
xmin, xmax = plt.xlim()

number_highlights = 0
def highlight_mix(v, c):
    global number_highlights
    number_highlights += 1
    #plt.axhline(v, color=f'C{number_highlights}', xmax=(c-xmin)/(xmax-xmin))
    #plt.axvline(c, color=f'C{number_highlights}', ymax=(v-np.log(ymin))/(np.log(ymax)-np.log(ymin)))
    plotf([xmin, c], [v, v], f'C{number_highlights}')
    plotf([c, c], [ymin, v], f'C{number_highlights}')
    plt.text(
            c, v*0.7, f'$C_m = {c:.3f}$', color='white',
            ha='center', va='top', rotation=270,
            bbox={'facecolor':f'C{number_highlights}', 'pad':3})
    plt.text(
            c-0.05, v, f'$\\mu = {v:.3f}$', color='white',
            ha='center', va='center',
            bbox={'facecolor':f'C{number_highlights}', 'pad':3})

for i, v in enumerate(highlighted_viscosities):
    c = interpf(v)
    highlight_mix(v, c)

plt.title(r'Viscosity of glycerol-water mixtures at $\rm20^oC$')
plotf(cm, visc, 'C0')
plt.xlabel(r'Glycerol mass fraction $C_m$')
plt.ylabel(r'Viscosity (20$\rm^o$C) $\mu\rm/Pa\,s$')

# inset = fig.add_axes([0.2, 0.72, 0.15, 0.1])
# inset.plot(cm, visc)
# inset.set_xlabel(r'$C_m$')
# inset.set_ylabel(r'$\mu$')
# inset.get_xaxis().set_ticks([])
# inset.get_yaxis().set_ticks([])
# inset.set_title('Non-truncated plot')

plt.savefig('glycerol_water_mixtures.pdf')
