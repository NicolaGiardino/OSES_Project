#ifndef __XENOJETBENCH_H__
#define __XENOJETBENCH_H__

#include <stdio.h>
#include <stdlib.h>

#define PRECISION 0.0001

// pi calculation
static long num_steps = 1000000;

// main
static int engine;
int xeno_main();

// methods
float getGama(float temp);
float getCp(float temp);
float getMach(int sub, float corair, float gama1);
float getAir(float mach, float gama2);
float getRayleighLoss(float mach1, float ttrat, float tlow);
void defaultParam();
void deduceInputs();
void getThermo();
void calcPerf();
void getGeo();
float deg2rad(float deg, float pi);
float sqroot(float numb1er);
float xfabs(float x);
float xlog(float x);
float expo(float x);
float fpow(float x, float y);
float power(float x, int y);
void create_tasks();
void cleanup();

// results
static float g0, g0d, gama, tref, abflag, tt4, tt4d, tt7, tt7d, p3p2d, p3fp2d,
    byprat, throtl, altd, u0d, fhvd, fhv, u0min, u0max, altmin, altmax, thrmin,
    thrmax, etmin, etmax, cprmin, cprmax, bypmin, bypmax, fprmin, fprmax, t4min,
    t4max, t7min, t7max, pmax, tmin, tmax, weight, Rgas, alt, ts0, ps0, a0, u0,
    fsmach, q0, rho0, cpair, tsout, psout, a8, a8d, a4, acore, m2, afan, mfr,
    isp, epr, etr, npr, snpr, fnet, fgros, dram, sfc, fa, eair, uexit, ues,
    fnlb, fglb, drlb, flflo, pexit, pfexit, a8max, a8rat, arsched, a4p, acap,
    a2, dcomp, lcomp, dburner, lburn, dturbin, lturb, dnozl, lnoz, dfan, ncomp,
    nturb, sblade, hblade, tblade, xcomp, ncompd;

// arrays
static float trat[20], tt[20], prat[20], pt[20], eta[20], gam[20], cp[20];

#endif /* __XENOJETBENCH_H__ */
