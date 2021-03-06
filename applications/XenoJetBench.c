#include "XenoJetBench.h"
#include "procedure.h"
#include <rtdevice.h>
#include <rtthread.h>

#define XENO_PRINT 1

#define HIGHEST RT_THREAD_PRIORITY_MAX - 20 /* highest priority */
#define HIGH RT_THREAD_PRIORITY_MAX - 19	/* high priority */
#define MID RT_THREAD_PRIORITY_MAX - 18		/* medium priority */
#define LOW RT_THREAD_PRIORITY_MAX - 17		/* low priority */

#define period 10 /* default T for periodic tasks */

static struct rt_thread thd_deduceinputs, thd_getthermo, thd_getgeo,
	thd_calcperf;
static rt_uint8_t rt_thd_stack[4][1024];

void create_tasks()
{
	rt_thread_init(&thd_deduceinputs, "DeduceInputs", &deduceInputs, RT_NULL,
				   rt_thd_stack[0], sizeof(rt_thd_stack[0]), HIGHEST, period);
	rt_thread_init(&thd_getthermo, "GetThermo", &getThermo, RT_NULL,
				   rt_thd_stack[1], sizeof(rt_thd_stack[1]), HIGH, period);
	rt_thread_init(&thd_getgeo, "GetGeo", &getGeo, RT_NULL, rt_thd_stack[2],
				   sizeof(rt_thd_stack[2]), MID, period);
	rt_thread_init(&thd_calcperf, "CalcPerf", &calcPerf, RT_NULL, rt_thd_stack[3],
				   sizeof(rt_thd_stack[3]), LOW, period);
}

void start_tasks()
{
	rt_thread_startup(&thd_deduceinputs);
	rt_thread_startup(&thd_getthermo);
	rt_thread_startup(&thd_getgeo);
	rt_thread_startup(&thd_calcperf);
}

int init_xeno()
{
	rt_uint32_t BM_Start = rt_tick_get_millisecond();
	rt_uint32_t BM_End, StartTime, EndTime;
	rt_int32_t ExecTime;
	size_t i;

#if XENO_PRINT
	rt_kprintf("XenoJetBench: An Open Source Hard-Real-Time Multiprocessor "
			   "Benchmark\n\n");
#endif

	// automatic engine selection
	engine = 1 + rand() % 3; /* random */

	// define paramaters
	defaultParam();

#if XENO_PRINT
	switch (engine)
	{
	case 1:
		rt_kprintf("Engine %d : Turbojet is selected\n\n", engine);
		break;
	case 2:
		rt_kprintf("Engine %d : Afterburner is selected\n\n", engine);
		break;
	case 3:
		rt_kprintf("Engine %d : Turbofan is selected\n\n", engine);
		break;
	default:
		rt_kprintf("Wrong engine choice [Select from 1, 2 or 3]\n");
		return (EXIT_FAILURE);
		break;
	}

	rt_kprintf(" ==> Starting XenoJetBench Execution \n\n");

	// header for results
	rt_kprintf("ExecTime, "
			   "Spd|Alt|Thr|Mach|Press|Temp|Fnet|Fgros|RamDr|FlFlo|TSFC|Airfl|"
			   "Weight|Fn/W\n");
#endif

	for (i = 0; i < NUM_READINGS; i++)
	{
		//********* PARSE INPUTS **********
		/*
		 * The mutex is taken only when reading/writing from/to the shared variables
		 * allowing other tasks to keep on using them
		 */
		rt_mutex_take(raw_mutex, RT_WAITING_FOREVER);
		u0d = acc_v[i];
		altd = baro_v[i];
		throtl = gyro_v[i];

		rt_mutex_release(raw_mutex);
		create_tasks();

		//********* START CALCULATIONS **********
		StartTime = rt_tick_get_millisecond();
		start_tasks();
		EndTime = rt_tick_get_millisecond();
		ExecTime = (rt_int32_t)(EndTime - StartTime);

		rt_mutex_take(bench_mutex, RT_WAITING_FOREVER);

		results[0] = u0d;
		results[1] = altd;
		results[2] = throtl;
		results[3] = fsmach;
		results[4] = psout;
		results[5] = tsout;
		results[6] = fnlb;
		results[7] = fglb;
		results[8] = drlb;
		results[9] = flflo;
		results[10] = sfc;
		results[11] = eair;
		results[12] = weight;
		results[13] = (fnlb / weight);

#if XENO_PRINT
		//*********** PRINT RESULTS ************
		rt_kprintf("%d, "
				   "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d\n     @ point %d\n",
				   ExecTime, (int)u0d, (int)altd, (int)throtl, (int)fsmach,
				   (int)psout, (int)tsout, (int)fnlb, (int)fglb, (int)drlb,
				   (int)flflo, (int)sfc, (int)eair, (int)weight,
				   (int)(fnlb / weight), i + 1);
#endif

		rt_mutex_release(bench_mutex);
	}

#if XENO_PRINT
	rt_kprintf("\n==> Ending XenoJetBench Execution\n\n");
	rt_kprintf("\n========================================================\n");
	rt_kprintf("    XenoJetBench Successfully Terminated\n\n");
#endif

#if XENO_PRINT
	rt_kprintf("    XenoJetBench Start time : %d secs\n ",
			   (int)(BM_Start / 1000));
#endif

	BM_End = rt_tick_get_millisecond();

#if XENO_PRINT
	rt_kprintf("   XenoJetBench End time : %d secs\n", (int)(BM_End / 1000));
	rt_kprintf("   Total Benchmark time : %d secs\n",
			   (int)((BM_End - BM_Start) / 1000));
	rt_kprintf("\n========================================================\n");
#endif

	return (EXIT_SUCCESS);
}

/* Utility to get gamma as a function of temperature */
float getGama(float temp)
{
	float number, a, b, c, d;
	a = -7.6942651e-13;
	b = 1.3764661e-08;
	c = -7.8185709e-05;
	d = 1.436914;
	number = a * temp * temp * temp + b * temp * temp + c * temp + d;
	return (number);
}

/* Utility to get cp as a function of temperature */
float getCp(float temp)
{
	float number, a, b, c, d;
	// BTU/R
	a = -4.4702130e-13;
	b = -5.1286514e-10;
	c = 2.8323331e-05;
	d = 0.2245283;
	number = a * temp * temp * temp + b * temp * temp + c * temp + d;
	return (number);
}

/* Utility to get the Mach number given the corrected airflow per area */
float getMach(int sub, float corair, float gama1)
{
	float deriv, machn, macho, airo, airn, number,
		chokair; // iterate for mach number
	int iter;
	chokair = getAir(1.0, gama1);
	if (corair > chokair)
	{
		number = 1.0;
		return (number);
	}
	else
	{
		airo = .25618; // initial guess
		if (sub == 1)
		{
			macho = 1.0; // sonic
		}
		else
		{
			if (sub == 2)
				macho = 1.703; // supersonic
			else
				macho = .5; // subsonic
			iter = 1;
			machn = macho - .2;
			while (abs(corair - airo) > .0001 && iter < 20)
			{
				airn = getAir(machn, gama1);
				deriv = (airn - airo) / (machn - macho);
				airo = airn;
				macho = machn;
				machn = macho + (corair - airo) / deriv;
				++iter;
			}
		}
		number = macho;
	}
	return (number);
}

/* Utility to get the corrected airflow per area given the Mach number */
float getAir(float mach, float gama2)
{
	float number, fac1, fac2;
	fac2 = (gama2 + 1.0) / (2.0 * (gama2 - 1.0));
	fac1 = fpow((1.0 + .5 * (gama2 - 1.0) * mach * mach), fac2);
	number = .50161 * sqroot(gama2) * mach / fac1;
	return (number);
}

/* Analysis for Rayleigh flow */
float getRayleighLoss(float mach1, float ttrat, float tlow)
{
	float number, wc1, wc2, mgueso, mach2, g1, gm1, g2, gm2, fac1, fac2, fac3,
		fac4;
	g1 = getGama(tlow);
	gm1 = g1 - 1.0;
	wc1 = getAir(mach1, g1);
	g2 = getGama(tlow * ttrat);
	gm2 = g2 - 1.0;
	number = .95;
	// iterate for mach downstream
	mgueso = .4; // initial guess
	mach2 = .5;
	while (abs(mach2 - mgueso) > .0001)
	{
		mgueso = mach2;
		fac1 = 1.0 + g1 * mach1 * mach1;
		fac2 = 1.0 + g2 * mach2 * mach2;
		fac3 = fpow((1.0 + .5 * gm1 * mach1 * mach1), (g1 / gm1));
		fac4 = fpow((1.0 + .5 * gm2 * mach2 * mach2), (g2 / gm2));
		number = fac1 * fac4 / fac2 / fac3;
		wc2 = wc1 * sqroot(ttrat) / number;
		mach2 = getMach(0, wc2, g2);
	}
	return (number);
}

/* Default parameters */
void defaultParam()
{
	int i;
	tref = 459.6;
	g0 = g0d = 32.2;
	gama = 1.4;
	for (i = 0; i <= 19; ++i)
	{
		trat[i] = 1.0;
		tt[i] = 518.6;
		prat[i] = 1.0;
		pt[i] = 14.7;
		eta[i] = 1.0;
	}
	tt[4] = tt4 = tt4d = 2500.;
	tt[7] = tt7 = tt7d = 2500.;
	prat[3] = p3p2d = 8.0;
	prat[13] = p3fp2d = 2.0;
	byprat = 1.0;
	abflag = 0;
	fhvd = fhv = 18600.;
	a2 = acore = 2.0;
	acap = .9 * a2;
	a8rat = .35;
	a8 = .7;
	a8d = .40;
	arsched = 0;
	afan = 2.0;
	a4 = .418;
	u0min = 0.0;
	u0max = 1500.;
	altmin = 0.0;
	altmax = 60000.;
	thrmin = 30;
	thrmax = 100;
	etmin = .5;
	etmax = 1.0;
	cprmin = 1.0;
	cprmax = 50.0;
	bypmin = 0.0;
	bypmax = 10.0;
	fprmin = 1.0;
	fprmax = 2.0;
	t4min = 1000.0;
	t4max = 3200.0;
	t7min = 1000.0;
	t7max = 4000.0;
	a8max = 0.4;
	pmax = 20.0;
	tmin = -100.0 + tref;
	tmax = 100.0 + tref;
	weight = 1000.;
	dfan = 293.02;
	dcomp = 293.02;
	dburner = 515.2;
	dturbin = 515.2;
	dnozl = 515.2;
}

/* Utility to have mach speed, atmospheric pressure and temperature */
void deduceInputs()
{
	Rgas = 1718.; /* ft2/sec2 R */
	alt = altd;
	if (alt < 36152.)
	{
		ts0 = 518.6 - 3.56 * alt / 1000.;
		ps0 = 2116. * fpow(ts0 / 518.6, 5.256);
	}
	if (alt >= 36152. && alt <= 82345.)
	{ // Stratosphere
		ts0 = 389.98;
		ps0 = 2116. * .2236 * expo((36000. - alt) / (53.35 * 389.98));
	}
	if (alt >= 82345.)
	{
		ts0 = 389.98 + 1.645 * (alt - 82345) / 1000.;
		ps0 = 2116. * .02456 * fpow(ts0 / 389.98, -11.388);
	}
	a0 = sqroot(gama * Rgas * ts0); // speed of sound ft/sec
	u0 = u0d * 5280. / 3600.;		// airspeed ft/sec
	fsmach = u0 / a0;
	q0 = gama / 2.0 * fsmach * fsmach * ps0;
	if (u0 > .0001)
	{
		rho0 = q0 / (u0 * u0);
	}
	else
	{
		rho0 = 1.0;
	}
	tt[0] = ts0 * (1.0 + .5 * (gama - 1.0) * fsmach * fsmach);
	pt[0] = ps0 * fpow(tt[0] / ts0, gama / (gama - 1.0));
	ps0 = ps0 / 144.;
	pt[0] = pt[0] / 144.;
	cpair = getCp(tt[0]); // BTU/lbm R
	tsout = ts0 - 459.6;
	psout = ps0;
}

/* Utility to have Thermodynamic parameters */
void getThermo()
{
	float m5, delhc, delhht, delhf, delhlt, deltc, deltht, deltf, deltlt;
	//  inlet recovery
	if (fsmach > 1.0) // supersonic
	{
		prat[2] = 1.0 - .075 * fpow(fsmach - 1.0, 1.35);
	}
	else
	{
		prat[2] = 1.0;
	}
	eta[2] = prat[2];
	// protection for overwriting input
	if (eta[3] < .5)
	{
		eta[3] = .5;
	}
	if (eta[5] < .5)
	{
		eta[5] = .5;
	}
	trat[7] = 1.0;
	prat[7] = 1.0;
	tt[2] = tt[1] = tt[0];
	pt[1] = pt[0];
	gam[2] = getGama(tt[2]);
	cp[2] = getCp(tt[2]);
	pt[2] = pt[1] * prat[2];
	// design - p3p2 specified - tt4 specified
	if (engine <= 2) // turbojet
	{
		prat[3] = p3p2d; // core compressor
		if (prat[3] < .5)
		{
			prat[3] = .5;
		}
		delhc = (cp[2] * tt[2] / eta[3]) *
				(fpow(prat[3], (gam[2] - 1.0) / gam[2]) - 1.0); // 0.25
		deltc = delhc / cp[2];
		pt[3] = pt[2] * prat[3];
		tt[3] = tt[2] + deltc;
		trat[3] = tt[3] / tt[2];
		gam[3] = getGama(tt[3]);
		cp[3] = getCp(tt[3]);
		tt[4] = tt4 * throtl / 100.0;
		gam[4] = getGama(tt[4]);
		cp[4] = getCp(tt[4]);
		trat[4] = tt[4] / tt[3];
		pt[4] = pt[3] * prat[4];
		delhht = delhc;
		deltht = delhht / cp[4];
		tt[5] = tt[4] - deltht;
		gam[5] = getGama(tt[5]);
		cp[5] = getCp(tt[5]);
		trat[5] = tt[5] / tt[4];
		prat[5] =
			fpow((1 - delhht / cp[4] / tt[4] / eta[5]), (gam[4] / (gam[4] - 1.0)));
		pt[5] = pt[4] * prat[5];
		// fan conditions
		prat[13] = 1.0;
		trat[13] = 1.0;
		tt[13] = tt[2];
		pt[13] = pt[2];
		gam[13] = gam[2];
		cp[13] = cp[2];
		prat[15] = 1.0;
		pt[15] = pt[5];
		trat[15] = 1.0;
		tt[15] = tt[5];
		gam[15] = gam[5];
		cp[15] = cp[5];
	}
	if (engine == 3) // turbofan
	{
		prat[13] = p3fp2d;
		if (prat[13] < .5)
			prat[13] = .5;
		delhf = (cp[2] * tt[2] / eta[13]) *
				(fpow(prat[13], (gam[2] - 1.0) / gam[2]) - 1.0);
		deltf = delhf / cp[2];
		tt[13] = tt[2] + deltf;
		pt[13] = pt[2] * prat[13];
		trat[13] = tt[13] / tt[2];
		gam[13] = getGama(tt[13]);
		cp[13] = getCp(tt[13]);
		prat[3] = p3p2d; // core compressor
		if (prat[3] < .5)
			prat[3] = .5;
		delhc = (cp[13] * tt[13] / eta[3]) *
				(fpow(prat[3], (gam[13] - 1.0) / gam[13]) - 1.0);
		deltc = delhc / cp[13];
		tt[3] = tt[13] + deltc;
		pt[3] = pt[13] * prat[3];
		trat[3] = tt[3] / tt[13];
		gam[3] = getGama(tt[3]);
		cp[3] = getCp(tt[3]);
		tt[4] = tt4 * throtl / 100.0;
		pt[4] = pt[3] * prat[4];
		gam[4] = getGama(tt[4]);
		cp[4] = getCp(tt[4]);
		trat[4] = tt[4] / tt[3];
		delhht = delhc;
		deltht = delhht / cp[4];
		tt[5] = tt[4] - deltht;
		gam[5] = getGama(tt[5]);
		cp[5] = getCp(tt[5]);
		trat[5] = tt[5] / tt[4];
		prat[5] = fpow((1.0 - delhht / cp[4] / tt[4] / eta[5]),
					   (gam[4] / (gam[4] - 1.0)));
		pt[5] = pt[4] * prat[5];
		delhlt = (1.0 + byprat) * delhf;
		deltlt = delhlt / cp[5];
		tt[15] = tt[5] - deltlt;
		gam[15] = getGama(tt[15]);
		cp[15] = getCp(tt[15]);
		trat[15] = tt[15] / tt[5];
		prat[15] = fpow((1.0 - delhlt / cp[5] / tt[5] / eta[5]),
						(gam[5] / (gam[5] - 1.0)));
		pt[15] = pt[5] * prat[15];
	}
	tt[7] = tt7;
	prat[6] = 1.0;
	pt[6] = pt[15];
	trat[6] = 1.0;
	tt[6] = tt[15];
	gam[6] = getGama(tt[6]);
	cp[6] = getCp(tt[6]);
	if (abflag > 0) // afterburner
	{
		trat[7] = tt[7] / tt[6];
		m5 = getMach(0, getAir(1.0, gam[5]) * a4 / acore, gam[5]);
		prat[7] = getRayleighLoss(m5, trat[7], tt[6]);
	}
	tt[7] = tt[6] * trat[7];
	pt[7] = pt[6] * prat[7];
	gam[7] = getGama(tt[7]);
	cp[7] = getCp(tt[7]);
	// engine press ratio EPReair
	epr = prat[7] * prat[15] * prat[5] * prat[4] * prat[3] * prat[13];
	// engine temp ratio ETR
	etr = trat[7] * trat[15] * trat[5] * trat[4] * trat[3] * trat[13];
}

/* Utility to determine engine performance */
void calcPerf()
{
	float fac1, game, cpe, cp3;
	cp3 = getCp(tt[3]); // BTU/lbm R
	g0 = 32.2;
	ues = 0.0;
	game = getGama(tt[5]);
	fac1 = (game - 1.0) / game;
	cpe = getCp(tt[5]);
	if (eta[7] < .8)
		eta[7] = .8; // protection during overwriting
	if (eta[4] < .8)
		eta[4] = .8;
	// specific net thrust - thrust / (g0*airflow) - lbf/lbm/sec
	// turbine engine core
	// airflow determined at choked nozzle exit
	pt[8] = epr * prat[2] * pt[0];
	eair =
		getAir(1.0, game) * 144. * a8 * pt[8] / 14.7 / sqroot(etr * tt[0] / 518.);
	m2 = getMach(
		0, eair * sqroot(tt[0] / 518.) / (prat[2] * pt[0] / 14.7 * acore * 144.),
		gama);
	npr = pt[8] / ps0;
	uexit = sqroot(2.0 * Rgas / fac1 * etr * tt[0] * eta[7] *
				   (1.0 - fpow(1.0 / npr, fac1)));
	if (npr <= 1.893)
		pexit = ps0;
	else
		pexit = .52828 * pt[8];
	fgros = (uexit + (pexit - ps0) * 144. * a8 / eair) / g0;
	// turbo fan -- added terms for fan flow
	if (engine == 3)
	{
		fac1 = (gama - 1.0) / gama;
		snpr = pt[13] / ps0;
		ues = sqroot(2.0 * Rgas / fac1 * tt[13] * eta[7] *
					 (1.0 - fpow(1.0 / snpr, fac1)));
		m2 = getMach(0,
					 eair * (1.0 + byprat) * sqroot(tt[0] / 518.) /
						 (prat[2] * pt[0] / 14.7 * afan * 144.),
					 gama);
		if (snpr <= 1.893)
		{
			pfexit = ps0;
		}
		else
		{
			pfexit = .52828 * pt[13];
		}
		fgros = fgros +
				(byprat * ues + (pfexit - ps0) * 144. * byprat * acore / eair) / g0;
	}
	// ram drag
	dram = u0 / g0;
	if (engine == 3)
	{
		dram = dram + u0 * byprat / g0;
	}
	// mass flow ratio
	if (fsmach > .01)
	{
		mfr = getAir(m2, gama) * prat[2] / getAir(fsmach, gama);
	}
	else
	{
		mfr = 5.;
	}
	// net thrust
	fnet = fgros - dram;
	// thrusts in pounds
	fnlb = fnet * eair;
	fglb = fgros * eair;
	drlb = dram * eair;
	// fuel-air ratio and sfc
	fa = (trat[4] - 1.0) / (eta[4] * fhv / (cp3 * tt[3]) - trat[4]) +
		 (trat[7] - 1.0) / (fhv / (cpe * tt[15]) - trat[7]);
	if (fnet > 0.0)
	{
		sfc = 3600. * fa / fnet;
		if (sfc < 0)
		{
			sfc = 0.0;
		}
		flflo = sfc * fnlb;
		isp = (fnlb / flflo) * 3600.;
	}
	else
	{
		fnlb = 0.0;
		flflo = 0.0;
		sfc = 0.0;
		isp = 0.0;
	}
	// weight calculation
	if (engine == 1)
	{
		weight = .12754 * sqroot(acore * acore * acore) *
				 (dcomp * lcomp + dburner * lburn + dturbin * lturb + dnozl * lnoz);
	}
	if (engine == 2)
	{
		weight = .08533 * sqroot(acore * acore * acore) *
				 (dcomp * lcomp + dburner * lburn + dturbin * lturb + dnozl * lnoz);
	}
	if (engine == 3)
	{
		weight = .08955 * acore *
				 ((1.0 + byprat) * dfan * 4.0 + dcomp * (ncomp - 3) + dburner +
				  dturbin * nturb + dburner * 2.0) *
				 sqroot(acore / 6.965);
	}
}

/* Utility to determine geometric variables */
void getGeo()
{
	if (afan < acore)
	{
		afan = acore;
	}
	// limits compressor face
	a8max = .75 * sqroot(etr) / epr;
	// mach number  to < .5
	if (a8max > 1.0)
	{
		a8max = 1.0;
	}
	if (a8rat > a8max)
	{
		a8rat = a8max;
	}
	// dumb down limit - a8 schedule
	if (arsched == 0)
	{
		a8rat = a8max;
	}
	a8 = a8rat * acore;
	a8d = a8 * prat[7] / sqroot(trat[7]);
	// assumes choked a8 and a4
	a4 = a8 * prat[5] * prat[15] * prat[7] / sqroot(trat[7] * trat[5] * trat[15]);
	a4p = a8 * prat[15] * prat[7] / sqroot(trat[7] * trat[15]);
	acap = .9 * a2;
	// size parameters for weight
	ncomp = (int)(1.0 + p3p2d / 1.5);
	if (ncomp > 15)
	{
		ncomp = 15;
	}
	sblade = .02;
	hblade = sqroot(2.0 / 3.1415926);
	tblade = .2 * hblade;
	xcomp = ncomp * (tblade + sblade);
	ncompd = ncomp;
	if (engine == 3) // fan geometry
	{
		ncompd = ncomp + 3;
		xcomp = ncompd * (tblade + sblade);
	}
	lcomp = xcomp;
	lburn = hblade;
	nturb = 1 + ncomp / 4;
	if (engine == 3)
	{
		nturb = nturb + 1;
	}
	lturb = nturb * (tblade + sblade);
	lnoz = lburn;
	if (engine == 2)
	{
		lnoz = 3.0 * lburn;
	}
}

// *********** Math utilities ***********
float sqroot(float number)
{
	float x0, x, prec = 1;
	if (number < 0)
	{
		number = -number;
		return (0);
	}
	x = (1 + number) / 2;
	while ((prec > 0.0001) || (prec < -0.0001))
	{
		x0 = x;
		x = 0.5 * (x0 + number / x0);
		prec = (x - x0) / x0;
	}
	return (x);
}

float xfabs(float x) { return (x < 0 ? -x : x); }

float xlog(float x)
{
	float number = 0, coeff = -1;
	int i = 1;
	if (x <= 0)
	{
		x = -x;
		return 0;
	}
	if (x == 1)
		return 0;
	if (x > 1)
		return -xlog(1 / x);
	// 0<x<1
	// log : x - x^2/2 + x^3/3 - x^4/4...
	while (xfabs(coeff) > PRECISION)
	{
		coeff *= 1 - x;
		number += coeff / i;
		i++;
	}
	return number;
}

float expo(float x)
{
	float number = 1, coeff = 1;
	int i = 1;
	// if x > log(DBL_MAX)
	if (x > 709.782712893384)
	{
		return expo(709.78); // Infinite value
	}
	// exp : x^0/0! + x^1/1! + x^2/2! + x^3/3!
	while (xfabs(coeff) > PRECISION)
	{
		coeff *= x / i;
		number += coeff;
		++i;
	}
	return number;
}

float fpow(float x, float y)
{
	int partieEntiere = y;
	if (x < 0)
	{
		return power(x, partieEntiere);
	}
	// now x>0
	// factorize y into integer and decimal parts
	// For example : 12.345^67.890123 = (12.345^67) * (12.345^0.890123)
	// integer part : pow(float, int) and the decimal par : x^y = exp(y*ln(x))
	return power(x, partieEntiere) * expo((y - partieEntiere) * xlog(x));
}

float power(float x, int y)
{
	float number = 1;
	int i;
	// x^(-y) = 1/(x^y)
	if (y < 0)
	{
		return 1 / (power(x, -y));
	}
	for (i = 0; i < y; ++i)
	{
		number *= x;
	}
	return number;
}
