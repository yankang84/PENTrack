/**
 * \file
 * Proton class definition.
 */

#include "globals.h"
#include "proton.h"

const char* NAME_PROTON = "proton";

std::ofstream TProton::endout; ///< endlog file stream
std::ofstream TProton::snapshotout; ///< snapshot file stream
std::ofstream TProton::trackout; ///< tracklog file stream
std::ofstream TProton::hitout; ///< hitlog file stream
std::ofstream TProton::spinout; ///< spinlog file stream

TProton::TProton(const int number, const double t, const double x, const double y, const double z, const double E, const double phi, const double theta, const double polarisation,
		const TMCGenerator &amc, const TGeometry &geometry, const TFieldManager &afield)
		: TParticle(NAME_PROTON, ele_e, m_p, 0, 0, number, t, x, y, z, E, phi, theta, polarisation, amc, geometry, afield){

}


void TProton::OnHit(const value_type x1, const state_type &y1, value_type &x2, state_type &y2, const double normal[3],
		const solid &leaving, const solid &entering, const TMCGenerator &mc, stopID &ID, std::vector<TParticle*> &secondaries) const{

}


void TProton::OnStep(const value_type x1, const state_type &y1, value_type &x2, state_type &y2, const dense_stepper_type &stepper,
		const solid &currentsolid, const TMCGenerator &mc, stopID &ID, std::vector<TParticle*> &secondaries) const{
	if (currentsolid.ID > 1){
		x2 = x1;
		y2 = y1;
		ID = ID_ABSORBED_IN_MATERIAL;
		printf("Absorption!\n");
	}
}


void TProton::Decay(const TMCGenerator &mc, const TGeometry &geom, const TFieldManager &field, std::vector<TParticle*> &secondaries) const{

};
