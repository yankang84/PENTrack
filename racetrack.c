#include "main.h"

// define racetrack current bars
// defined by two position vectors (SW1r,SW1phi,SW1z and SW2r,SW2phi,SW2z) lying on the straight wire 
						//		current from outside in		from inside out			from high to low			from low to high
long double Bars_1r[14]	= {0,	0.6, 0.6, 0.6, 0.6, 		0, 0, 0, 0, 			0.6, 0.6, 0.6, 0.6, 		0};
long double Bars_1phi[14]={0,	0, pi/2, pi, 1.5*pi,		0, pi/2, pi, 1.5*pi,	0, pi/2, pi, 1.5*pi,		0};
long double Bars_1z[14] = {0,	-0.15, -0.15, -0.15, -0.15, 1.35, 1.35, 1.35, 1.35, 1.35, 1.35, 1.35, 1.35, 	-0.15};
long double Bars_2r[14] = {0,	0, 0, 0, 0,					0.6, 0.6, 0.6, 0.6,		0.6, 0.6, 0.6, 0.6,			0};
long double Bars_2phi[14]={0,	0, pi/2, pi, 1.5*pi,		0, pi/2, pi, 1.5*pi,	0, pi/2, pi, 1.5*pi,		0};
long double Bars_2z[14] = {0,	-0.15, -0.15, -0.15, -0.15, 1.35, 1.35, 1.35, 1.35, -0.15, -0.15, -0.15, -0.15, 1.35};


void RacetrackField(long double rloc, long double philoc, long double zloc){
	// add fields and field derivations of racetrack coils
	
	if((Ibar!=0)&&(RodFieldMultiplicator>0))
	{
		if(Racetracks==1)
		{
			// old implementation of racetracks as 5 infinitely long vertical current bars, fast
			Racetrack(rloc, philoc, zloc, -0.424264, -0.424264, RodFieldMultiplicator*Ibar);
			Racetrack(rloc, philoc, zloc, 0.424264, -0.424264, RodFieldMultiplicator*Ibar); 
			Racetrack(rloc, philoc, zloc, -0.424264, 0.424264, RodFieldMultiplicator*Ibar);
			Racetrack(rloc, philoc, zloc, 0.424264, 0.424264, RodFieldMultiplicator*Ibar); 
			CenterCurrent(rloc, philoc, zloc, -4.0*RodFieldMultiplicator*Ibar);
		}
		else if(Racetracks==2)
		{			
			// new but slow implementation of racetracks including horizontal parts
			BarRaceTrack(rloc, philoc, zloc, RodFieldMultiplicator*Ibar);   // here B field is only calculated for charged particles and derivatives only for neutrons
		}
		
	}  		
}	


// produce   B-Field of one straight wire in z direction an positions lx, ly and current I_rt
void Racetrack(long double r,long double phi,long double z,  long double lx, long double ly, long double I_rt){
		long double Brv,dBrdrv,dBrdphiv,Bphiv,dBphidrv,dBphidphiv; 
	// cartesian coordinates of neutron
	//long double x = cosl(phi) * r, y = sinl(phi) *r;
	// cartesian coordinates of racetracks	
	long double vorfaktor = mu0 * I_rt / (2 * pi);
	
	long double t2 = cosl(phi);    
  long double t3 = ly*r*t2;
  long double     t5 = sinl(phi);
  long double     t6 = lx*r*t5;
      long double t7 = t3-t6;
      long double t8 = vorfaktor*t7;
      long double t9 = r*r;
      long double t10 = t2*t2;
      long double t11 = t9*t10;
      long double t12 = r*t2;
      long double t13 = t12*lx;
      long double t15 = lx*lx;
      long double t16 = t5*t5;
      long double t17 = t9*t16;
      long double t18 = r*t5;
      long double t19 = t18*ly;
      long double t21 = ly*ly;
      long double t22 = t11-2.0*t13+t15+t17-2.0*t19+t21;
      long double t23 = 1/t22;
      long double t24 = 1/r;
      long double t25 = t23*t24;
      Brv = t8*t25;
      long double t31 = t22*t22;
      long double t33 = 1/t31*t24;
      long double t34 = r*t10;
      long double t35 = t2*lx;
      long double t36 = r*t16;
      long double t37 = t5*ly;
      long double t42 = 1/t9;
      dBrdrv = vorfaktor*(ly*t2-lx*t5)*t25-t8*t33*(2.0*t34-2.0*t35+2.0*t36-2.0*t37)-t8*t23*t42;
      dBrdphiv = vorfaktor*(-t19-t13)*t25+2.0*t8*t33*t7;
      long double t53 = vorfaktor*(-t17-t19-t11+t13);
      long double t54 = t12-lx;
      long double t55 = t54*t54;
      long double t56 = t18-ly;
      long double t57 = t56*t56;
      long double t58 = t55+t57;
      long double t59 = sqrt(t58);
      long double t60 = 1/t59;
      long double t61 = t60*t24;
      Bphiv = t53*t61;
      long double t69 = 1/t59/t58*t24;
      dBphidrv = vorfaktor*(-2.0*t36-t37-2.0*t34+t35)*t61-t53*t69*(2.0*t54*t2+2.0*t56*t5)/2.0-t53*t60*t42;
      dBphidphiv = vorfaktor*(-t3-t6)*t61-t53*t69*(-2.0*t54*r*t5+2.0*t56*r*t2)/2.0;


      // add values to current B-values (superposition principle)
      Br = Br + Brv;
      dBrdr = dBrdr + dBrdrv;
      dBrdphi = dBrdphi + dBrdphiv;
      Bphi = Bphi + Bphiv;
      dBphidr = dBphidr + dBphidrv;
      dBphidphi = dBphidphi + dBphidphiv;
      
      if((dBphidrv> 1) || (dBphidrv < -1))
         printf("Br = %17LG, %17LG, %17LG \n Bphi = %17LG, %17LG, %17LG \n r = %17LG, phi = %17LG, z = %17LG\n", Br,dBrdr,dBrdphi,Bphi ,dBphidr,dBphidphi, r, phi, z);
	
	
	return;
	
}

// produce center current rod field 
void CenterCurrent(long double r,long double phi,long double z, long double I_rt){
	
	long double vorfaktor = mu0 * I_rt / (2 * pi);	
		
	// only Bphi is present
	Bphi += vorfaktor/r;	
	
	// dBphidr
	dBphidr -= vorfaktor/(r*r);		
	
	// dBphidphi =0		
	
	return;	
}

// produce B-field at position r,phi,z of a straight wire with current I_rt 
// defined by two position vectors (SW1r,SW1phi,SW1z and SW2r,SW2phi,SW2z) lying on the ends of the straight wire 
// code generated by Maple ("FiniteWireDiffs.mw") with formula http://de.wikipedia.org/wiki/Biot-Savart#Gerader_Linienleiter
void StraightWireField(const long double r,const long double phi,const long double z, const long double I_rt, 
						const long double SW1r, const long double SW1phi, const long double SW1z, 
						const long double SW2r, const long double SW2phi, const long double SW2z)
{
	long double vorfaktor = mu0 * I_rt / (4 * pi);

	long double t1 = cosl(SW1phi);
	long double t2 = t1 * SW1r;
	long double t3 = cosl(phi);
	long double t4 = t3 * r;
	long double t5 = t2 - t4;
	long double t6 = t5 * t5;
	long double t7 = sinl(SW1phi);
	long double t8 = t7 * SW1r;
	long double t9 = sin(phi);
	long double t10 = t9 * r;
	long double t11 = t8 - t10;
	long double t12 = t11 * t11;
	long double t13 = SW1z - z;
	long double t14 = t13 * t13;
	long double t15 = t6 + t12 + t14;
	long double t16 = sqrtl(t15);
	long double t17 = 0.1e1 / t16;
	long double t18 = t17 * vorfaktor;
	long double t19 = 0.1e1 / t15;
	long double t20 = SW1r * SW1r;
	long double t21 = t1 * t1;
	long double t23 = cosl(SW2phi);
	long double t24 = t23 * SW2r;
	long double t28 = t7 * t7;
	long double t30 = sinl(SW2phi);
	long double t31 = t30 * SW2r;
	long double t37 = SW2z - SW1z;
	long double t39 = t21 * t20 - t1 * (t24 + t4) * SW1r + t28 * t20 - t7 * (t10 + t31) * SW1r + t24 * t4 + t31 * t10 - t37 * t13;
	long double t40 = t39 * t39;
	long double t42 = t24 - t2;
	long double t43 = t42 * t42;
	long double t44 = t31 - t8;
	long double t45 = t44 * t44;
	long double t46 = t37 * t37;
	long double t47 = t43 + t45 + t46;
	long double t48 = 0.1e1 / t47;
	long double t50 = 0.1e1 - t48 * t40 * t19;
	long double t51 = sqrtl(t50);
	long double t52 = 0.1e1 / t51;
	long double t53 = t52 * t18;
	long double t54 = t24 - t4;
	long double t55 = t54 * t54;
	long double t56 = t31 - t10;
	long double t57 = t56 * t56;
	long double t58 = SW2z - z;
	long double t59 = t58 * t58;
	long double t60 = t55 + t57 + t59;
	long double t61 = sqrtl(t60);
	long double t62 = 0.1e1 / t61;
	long double t63 = SW2r * SW2r;
	long double t64 = t23 * t23;
	long double t69 = t30 * t30;
	long double t77 = -t64 * t63 + t23 * (t2 + t4) * SW2r - t69 * t63 + t30 * (t8 + t10) * SW2r - t4 * t2 - t10 * t8 - t37 * t58;
	long double t79 = sqrtl(t47);
	long double t80 = 0.1e1 / t79;
	long double t84 = -t80 * t77 * t62 + t80 * t39 * t17;
	long double t87 = -t37 * t11 + t44 * t13;
	long double t88 = t87 * t87;
	long double t91 = -t42 * t13 + t37 * t5;
	long double t92 = t91 * t91;
	long double t95 = -t44 * t5 + t42 * t11;
	long double t96 = t95 * t95;
	long double t97 = t88 + t92 + t96;
	long double t98 = sqrtl(t97);
	long double t99 = 0.1e1 / t98;
	long double t100 = t99 * t84;
	long double t101 = t3 * t87;
	long double t103 = t101 * t100 * t53;
	long double t104 = t9 * t91;
	long double t106 = t104 * t100 * t53;
	long double t109 = 0.1e1 / t16 / t15;
	long double t110 = t109 * vorfaktor;
	long double t111 = t84 * t52;
	long double t112 = t111 * t110;
	long double t113 = t87 * t99;
	long double t116 = -t3 * t5 - t9 * t11;
	long double t117 = 0.2e1 * t116 * t3;
	long double t122 = 0.1e1 / t51 / t50;
	long double t124 = t84 * t122 * t18;
	long double t125 = t15 * t15;
	long double t127 = t40 / t125;
	long double t130 = t39 * t19;
	long double t139 = -t1 * t3 * SW1r - t7 * t9 * SW1r + t23 * SW2r * t3 + t30 * SW2r * t9;
	long double t143 = 0.2e1 * t116 * t48 * t127 - 0.2e1 * t139 * t48 * t130;
	long double t144 = t143 * t3;
	long double t150 = t77 / t61 / t60;
	long double t159 = t39 * t109;
	long double t166 = t99 * ((-t3 * t54 - t9 * t56) * t80 * t150 - t80 * t139 * t62 - t116 * t80 * t159 + t80 * t139 * t17);
	long double t169 = t111 * t18;
	long double t171 = 0.1e1 / t98 / t97;
	long double t172 = t87 * t171;
	long double t173 = t9 * t87;
	long double t175 = t3 * t91;
	long double t178 = t42 * t9;
	long double t179 = t44 * t3 - t178;
	long double t181 = t37 * t173 - t37 * t175 + t179 * t95;
	long double t182 = 0.2e1 * t181 * t3;
	long double t186 = t91 * t99;
	long double t187 = 0.2e1 * t116 * t9;
	long double t191 = t143 * t9;
	long double t197 = t91 * t171;
	long double t198 = 0.2e1 * t181 * t9;
	long double t207 = t9 * r * t5 - t3 * r * t11;
	long double t208 = 0.2e1 * t207 * t3;
	long double t214 = SW1r * r;
	long double t221 = t1 * t9 * t214 - t7 * t3 * t214 - t24 * t10 + t31 * t4;
	long double t225 = 0.2e1 * t207 * t48 * t127 - 0.2e1 * t221 * t48 * t130;
	long double t226 = t225 * t3;
	long double t246 = t99 * ((t9 * r * t54 - t3 * r * t56) * t80 * t150 - t80 * t221 * t62 - t207 * t80 * t159 + t80 * t221 * t17);
	long double t257 = -t44 * t10 - t42 * t4;
	long double t259 = t37 * t3 * r * t87 + t37 * t9 * r * t91 + t257 * t95;
	long double t260 = 0.2e1 * t259 * t3;
	long double t264 = r * t99;
	long double t265 = t3 * t3;
	long double t266 = t37 * t265;
	long double t270 = t173 * t100 * t53;
	long double t271 = 0.2e1 * t207 * t9;
	long double t275 = t225 * t9;
	long double t281 = 0.2e1 * t259 * t9;
	long double t285 = t9 * t9;
	long double t286 = t37 * t285;
	long double t290 = t175 * t100 * t53;
	long double t291 = -t208 * t113 * t112 / 0.2e1 - t226 * t113 * t124 / 0.2e1 + t101 * t246 * t53 - t260 * t172 * t169 / 0.2e1 + t266 * t264 * t169 - t270 - t271 * t186 * t112 / 0.2e1 - t275 * t186 * t124 / 0.2e1 + t104 * t246 * t53 - t281 * t197 * t169 / 0.2e1 + t286 * t264 * t169 + t290;
	long double t292 = -0.2e1 * t13 * t3;
	long double t301 = -0.2e1 * t13 * t48 * t127 - 0.2e1 * t37 * t48 * t130;
	long double t302 = t301 * t3;
	long double t317 = t99 * (-t58 * t80 * t150 - t80 * t37 * t62 + t13 * t80 * t159 + t80 * t37 * t17);
	long double t322 = -t44 * t87 + t42 * t91;
	long double t323 = 0.2e1 * t322 * t3;
	long double t330 = -0.2e1 * t13 * t9;
	long double t334 = t301 * t9;
	long double t340 = 0.2e1 * t322 * t9;
	long double t429 = t52 * t110;
	long double t434 = t122 * t18;
	long double t441 = t171 * t84;
	Br += 		t103 + t106;
	dBrdr += 	-t117 * t113 * t112 / 0.2e1 - t144 * t113 * t124 / 0.2e1 + t101 * t166 * t53 - t182 * t172 * t169 / 0.2e1 - t187 * t186 * t112 / 0.2e1 - t191 * t186 * t124 / 0.2e1 + t104 * t166 * t53 - t198 * t197 * t169 / 0.2e1;
	dBrdphi += 	t291;
	dBrdz += 	-t292 * t113 * t112 / 0.2e1 - t302 * t113 * t124 / 0.2e1 + t101 * t317 * t53 - t323 * t172 * t169 / 0.2e1 - t44 * t3 * t100 * t53 - t330 * t186 * t112 / 0.2e1 - t334 * t186 * t124 / 0.2e1 + t104 * t317 * t53 - t340 * t197 * t169 / 0.2e1 + t178 * t100 * t53;
	Bphi += 	-t270 + t290;
	dBphidr += 	t187 * t113 * t112 / 0.2e1 + t191 * t113 * t124 / 0.2e1 - t173 * t166 * t53 + t198 * t172 * t169 / 0.2e1 - t286 * t100 * t53 - t117 * t186 * t112 / 0.2e1 - t144 * t186 * t124 / 0.2e1 + t175 * t166 * t53 - t182 * t197 * t169 / 0.2e1 - t266 * t100 * t53;
	dBphidphi += t271 * t113 * t112 / 0.2e1 + t275 * t113 * t124 / 0.2e1 - t173 * t246 * t53 + t281 * t172 * t169 / 0.2e1 - t103 - t208 * t186 * t112 / 0.2e1 - t226 * t186 * t124 / 0.2e1 + t175 * t246 * t53 - t260 * t197 * t169 / 0.2e1 - t106;
	dBphidz += 	t330 * t113 * t112 / 0.2e1 + t334 * t113 * t124 / 0.2e1 - t173 * t317 * t53 + t340 * t172 * t169 / 0.2e1 + t9 * t44 * t100 * t53 - t292 * t186 * t112 / 0.2e1 - t302 * t186 * t124 / 0.2e1 + t175 * t317 * t53 - t323 * t197 * t169 / 0.2e1 + t3 * t42 * t100 * t53;
	Bz += 		t95 * t100 * t53;
	dBzdr += 	-t116 * t95 * t100 * t429 - t143 * t95 * t100 * t434 / 0.2e1 + t95 * t166 * t53 - t181 * t95 * t441 * t53 + t179 * t100 * t53;
	dBzdphi += 	-t207 * t95 * t100 * t429 - t225 * t95 * t100 * t434 / 0.2e1 + t95 * t246 * t53 - t259 * t95 * t441 * t53 + t257 * t100 * t53;
	dBzdz += 	t13 * t95 * t100 * t429 - t301 * t95 * t100 * t434 / 0.2e1 + t95 * t317 * t53 - t322 * t95 * t441 * t53;

	return;	
}

// here the magnetic field for all current bars in the race track coils is generated and added to the current field
void BarRaceTrack(long double r_current, long double phi_current, long double z_current, long double I_bar)
{
	// calculate flux density at point r_current, phi_current, z_current
	// for all single current bars
	for(int cobar=1;cobar<=12;cobar++)
	{
		StraightWireField(r_current,phi_current,z_current, I_bar, Bars_1r[cobar],Bars_1phi[cobar], Bars_1z[cobar],Bars_2r[cobar], Bars_2phi[cobar], Bars_2z[cobar]);					
	}
	// then for the center bar (4x current)   
	StraightWireField(r_current,phi_current,z_current, 4.0*I_bar, Bars_1r[13],Bars_1phi[13], Bars_1z[13],Bars_2r[13], Bars_2phi[13], Bars_2z[13]);
}

