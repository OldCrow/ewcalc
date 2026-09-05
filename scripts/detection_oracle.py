#!/usr/bin/env python3
"""Exact required-SNR oracle for radar detection statistics (square-law
detector, N-pulse noncoherent integration, Swerling 0-4), plus Python
mirrors of Albersheim's and Shnidman's empirical equations for validation.

Conventions (Richards, Fundamentals of Radar Signal Processing, ch. 6):
  - Noise: complex Gaussian, unit power per sample. Square-law sum of N
    noise-only samples ~ Gamma(N, 1), so PFA = Q_gamma(N, T) and the
    threshold is T = gammainccinv(N, PFA).
  - Swerling 0: 2*z ~ noncentral chi-square, df=2N, nc=2*N*chi.
  - Swerling 1: chi fluctuates scan-to-scan, exponential pdf; average
    the Sw0 PD over the pdf (numeric integral).
  - Swerling 2: per-pulse exponential; each sample exponential with
    mean 1+chi => z ~ Gamma(N, 1+chi), PD = Q_gamma(N, T/(1+chi)).
  - Swerling 3: scan-to-scan chi-4 pdf (4x/chi^2) exp(-2x/chi); numeric
    integral of Sw0 PD over it.
  - Swerling 4: per-pulse chi-4; validated by Monte Carlo (1e7 samples).
Required per-pulse SNR is found by bisection on chi in dB.
"""
import numpy as np
from scipy.special import gammainccinv, gammaincc
from scipy.stats import ncx2
from scipy.integrate import quad
from math import log, log10, sqrt, exp, copysign

rng = np.random.default_rng(20260906)

def threshold(n, pfa):
    return gammainccinv(n, pfa)

def pd_sw0(chi, n, t):
    return ncx2.sf(2.0 * t, 2 * n, 2.0 * n * chi)

def pd_sw1(chi, n, t):
    f = lambda x: pd_sw0(x, n, t) * np.exp(-x / chi) / chi
    v, _ = quad(f, 0, np.inf, limit=200)
    return v

def pd_sw2(chi, n, t):
    return gammaincc(n, t / (1.0 + chi))

def pd_sw3(chi, n, t):
    f = lambda x: pd_sw0(x, n, t) * (4.0 * x / chi**2) * np.exp(-2.0 * x / chi)
    v, _ = quad(f, 0, np.inf, limit=200)
    return v

def pd_sw4(chi, n, t, m=10_000_000):
    # per-pulse chi-4 power: Gamma(k=2, scale=chi/2); amplitude = sqrt
    p = rng.gamma(2.0, chi / 2.0, size=(m // n if n > 1 else m, n))
    w = (rng.standard_normal(p.shape) + 1j * rng.standard_normal(p.shape)) / sqrt(2.0)
    z = np.abs(np.sqrt(p) + w) ** 2
    return float(np.mean(z.sum(axis=1) > t))

PD_FUNCS = {0: pd_sw0, 1: pd_sw1, 2: pd_sw2, 3: pd_sw3, 4: pd_sw4}

def required_snr_exact(pd, pfa, n, case):
    t = threshold(n, pfa)
    f = PD_FUNCS[case]
    lo, hi = -20.0, 60.0  # dB bracket
    for _ in range(60 if case != 4 else 24):
        mid = 0.5 * (lo + hi)
        chi = 10.0 ** (mid / 10.0)
        if f(chi, n, t) < pd:
            lo = mid
        else:
            hi = mid
    return 0.5 * (lo + hi)

# ---- empirical equations under test -------------------------------------
def albersheim(pd, pfa, n):
    a = log(0.62 / pfa)
    b = log(pd / (1.0 - pd))
    return (-5.0 * log10(n)
            + (6.2 + 4.545 / sqrt(n + 0.44)) * log10(a + 0.12 * a * b + 1.7 * b))

def shnidman(pd, pfa, n, case):
    k = {0: float("inf"), 1: 1.0, 2: float(n), 3: 2.0, 4: 2.0 * float(n)}[case]
    alpha = 0.25 if n >= 40 else 0.0
    eta = (sqrt(-0.8 * log(4.0 * pfa * (1.0 - pfa)))
           + copysign(1.0, pd - 0.5) * sqrt(-0.8 * log(4.0 * pd * (1.0 - pd))))
    x_inf = eta * (eta + 2.0 * sqrt(n / 2.0 + (alpha - 0.25)))
    if case == 0:
        c_db = 0.0
    else:
        c1 = (((17.7006 * pd - 18.4496) * pd + 14.5339) * pd - 3.525) / k
        c2 = 0.0
        if pd > 0.872:
            c2 = (1.0 / k) * (exp(27.31 * pd - 25.14)
                              + (pd - 0.8) * (0.7 * log(1e-5 / pfa)
                                              + (2.0 * n - 20.0) / 80.0))
        c_db = c1 + c2
    c = 10.0 ** (c_db / 10.0)
    x1 = c * x_inf / n
    return 10.0 * log10(x1)

if __name__ == "__main__":
    print(f"{'Pd':>5} {'Pfa':>7} {'N':>3} {'Sw':>2} {'exact':>8} {'shnid':>8} {'err':>6} {'albr':>8} {'aerr':>6}")
    worst = 0.0
    for pd in (0.5, 0.9):
        for pfa in (1e-6, 1e-4):
            for n in (1, 10, 30):
                for case in (0, 1, 2, 3, 4):
                    ex = required_snr_exact(pd, pfa, n, case)
                    sh = shnidman(pd, pfa, n, case)
                    err = sh - ex
                    al = albersheim(pd, pfa, n) if case == 0 else float("nan")
                    aerr = al - ex if case == 0 else float("nan")
                    tag = "" if abs(err) < 0.6 else "  <-- BAD"
                    if case != 4:
                        worst = max(worst, abs(err))
                    print(f"{pd:5.2f} {pfa:7.0e} {n:3d} {case:2d} {ex:8.3f} {sh:8.3f} {err:6.2f} {al:8.3f} {aerr:6.2f}{tag}")
    print(f"worst |shnidman - exact| over Sw0-3 grid: {worst:.3f} dB")
    # closed-form anchor: Swerling 1, N=1: chi = ln(Pfa)/ln(Pd) - 1
    for (pd, pfa) in ((0.5, 1e-6), (0.9, 1e-4)):
        chi = log(pfa) / log(pd) - 1.0
        print(f"Sw1 N=1 closed-form check pd={pd} pfa={pfa:g}: "
              f"{10*log10(chi):.4f} dB vs exact {required_snr_exact(pd,pfa,1,1):.4f} dB")
