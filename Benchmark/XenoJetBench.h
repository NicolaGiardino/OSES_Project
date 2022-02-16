#include <stdio.h>
#include <stdlib.h>

// Define the number of threads
#define NUM_THREADS 2

// pi calculation
static long num_steps = 1000000;

#define PRECISION 0.0001

#ifndef __XENOJETBENCH_H__
#define __XENOJETBENCH_H__

int main();

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
int xeno_main(int engine);

static int engine;

static float g0, g0d, gama, tref, abflag;
static float tt4, tt4d, tt7, tt7d, p3p2d, p3fp2d, byprat, throtl;
static float altd, u0d, fhvd, fhv;

// arrays
static float trat[20];
static float tt[20];
static float prat[20];
static float pt[20];
static float eta[20];
static float gam[20];
static float cp[20];

static float u0min, u0max, altmin, altmax, thrmin, thrmax;
static float etmin, etmax, cprmin, cprmax, bypmin, bypmax;
static float fprmin, fprmax, t4min, t4max, t7min, t7max, pmax, tmin, tmax;

static float weight;
static float Rgas, alt, ts0, ps0, a0, u0, fsmach, q0, rho0, cpair, tsout, psout;

static float a8, a8d, a4, acore, m2, afan, mfr, isp;
static float epr, etr, npr, snpr;
static float fnet, fgros, dram, sfc, fa, eair, uexit, ues;
static float fnlb, fglb, drlb, flflo;
static float pexit, pfexit;

static float a8max, a8rat, arsched, a4p, acap, a2;

static float dcomp, lcomp, dburner, lburn, dturbin, lturb, dnozl, lnoz, dfan, ncomp, nturb;
static float sblade, hblade, tblade, xcomp, ncompd;

#endif /* __XENOJETBENCH_H__ */
