/**
 * \file
 * Electron class definition.
 */

#include "globals.h"
#include "electron.h"

const char* NAME_ELECTRON = "electron";

std::ofstream TElectron::endout; ///< endlog file stream
std::ofstream TElectron::snapshotout; ///< snapshot file stream
std::ofstream TElectron::trackout; ///< tracklog file stream
std::ofstream TElectron::hitout; ///< hitlog file stream
std::ofstream TElectron::spinout; ///< spinlog file stream

TElectron::TElectron(int number, double t, double x, double y, double z, double E, double phi, double theta, double polarisation,
		TMCGenerator &amc, TGeometry &geometry, TFieldManager *afield)
			: TParticle(NAME_ELECTRON, -ele_e, m_e, 0, 0, number, t, x, y, z, E, phi, theta, polarisation, amc, geometry, afield){

}


void TElectron::OnHit(const value_type x1, const state_type &y1, value_type &x2, state_type &y2,
		const double normal[3], const solid &leaving, const solid &entering, stopID &ID, std::vector<TParticle*> &secondaries) const{

}


void TElectron::OnStep(const value_type x1, const state_type &y1, value_type &x2, state_type &y2,
		const dense_stepper_type &stepper, const solid &currentsolid, stopID &ID, std::vector<TParticle*> &secondaries) const{
	if (currentsolid.ID != GetGeometry()->defaultsolid.ID){
		x2 = x1;
		y2 = y1;
		ID = ID_ABSORBED_IN_MATERIAL;
		printf("Absorption!\n");
	}
/*		else{
		long double v = sqrt(y1[3]*y1[3] + y1[4]*y1[4] + y1[5]*y1[5]);
		long double s = sqrt(pow(y2[0]- y1[0], 2) + pow(y2[1] - y1[1], 2) + pow(y2[2] - y1[2], 2));
		long double Ekin = m*c_0*c_0*(1/sqrt(1 - v*v/c_0/c_0) - 1);
		double Eloss, theta;
		int index;
		long double n = (1e-3*100)/(1.3806488e-23)/(77); // number density of rest gas (P [Pa])/(k [J/K])/(T [K])
		eH2(Ekin, s, n, &Eloss, &theta, &index); // did scattering happen?
		if (index != 0){
			printf("\nScattering %d, Eloss = %f\n",index,Eloss);
			double v_scat[3] = {y2[3], y2[4], y2[5]};
			eH2velocity(Eloss, theta, v_scat); // change velocity accordingly
			if (index == 3){ // on ionization create secondary electron
				TElectron *e = new TElectron(particlenumber, x2, y2, &y2[3], polarisation, *mc, *field);
				secondaries.push_back(e);
			}
		}
	}*/
}


void TElectron::Decay(std::vector<TParticle*> &secondaries) const{

}
