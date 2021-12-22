#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include <sched.h>

#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>

#include  <rtdk.h>
#include <rtdm/rtdm.h>
#include <rtdm/rtserial.h>
#include <sys/io.h>

//Define the number of threads
#define NUM_THREADS 2

//pi calculation
static long num_steps = 1000000; 

#define PRECISION 0.0001

#ifndef _TEST_
#define _TEST_


int main();
int rt_printf(const char *format, ...);

double getGama(double temp);
double getCp(double temp);
double getMach();
double getAir();
double getRayleighLoss();
void defaultParam();
void deduceInputs();
void getThermo();
void calcPerf();
void getGeo();
double deg2rad(double deg,double pi);
double sqroot(double numb1er);
double fabs(double x);
double log(double x);
double expo(double x);
double fpow(double x, double y);
double power(double x, int y);

static int engine;

static double g0,g0d,gama,tref,abflag;
static double tt4,tt4d,tt7,tt7d,p3p2d,p3fp2d,byprat,throtl;
static double altd,u0d,fhvd,fhv;

//arrays
static double *trat;
static double *tt;
static double *prat;
static double *pt;
static double *eta;
static double *gam;
static double *cp;

static double u0min,u0max,altmin,altmax,thrmin,thrmax;
static double etmin,etmax,cprmin,cprmax,bypmin,bypmax;
static double fprmin,fprmax,t4min,t4max,t7min,t7max,pmax,tmin,tmax;

static double weight;
static double Rgas,alt,ts0,ps0,a0,u0,fsmach,q0,rho0,cpair,tsout,psout;

static double a8,a8d,a4,acore,m2,afan,mfr,isp;
static double epr,etr,npr,snpr;
static double fnet,fgros,dram,sfc,fa,eair,uexit,ues;
static double fnlb,fglb,drlb,flflo;
static double pexit,pfexit;

static double a8max,a8rat,arsched,a4p,acap,a2;

static double dcomp,lcomp,dburner,lburn,dturbin,lturb,dnozl,lnoz,dfan,ncomp,nturb;
static double sblade,hblade,tblade,xcomp,ncompd;


#endif
