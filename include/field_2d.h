/**
 * \file
 * Bicubic interpolation of axisymmetric field tables.
 */

#ifndef FIELD_2D_H_
#define FIELD_2D_H_

#include <vector>

#include "interpolation.h"

#include "field.h"

/**
 * Class for bicubic field interpolation, create one for every table file you want to use.
 *
 * This class loads a special file format from "Vectorfields Opera" containing a regular, rectangular table of magnetic and electric fields and
 * calculates bicubic interpolation coefficients (4x4 matrix for each grid point) to allow fast evaluation of the fields at arbitrary points.
 * Therefore it assumes that the fields are axisymmetric around the z axis.
 *
 */
class TabField: public TField{
	private:
		int m; ///< radial size of the table file
		int n; ///< axial size of the arrays
		double rdist; ///< distance between grid points in radial direction
		double zdist; ///< distance between grid points in axial direction
		double r_mi; ///< lower radial coordinate of rectangular grid
		double z_mi; ///< lower axial coordinate of rectangular grid
		bool fBrc, fBphic, fBzc, fErc, fEphic, fEzc, fVc; ///< remember which field components were loaded from table file
		alglib::spline2dinterpolant Brc, Bphic, Bzc, Erc, Ephic, Ezc, Vc; ///< spline interpolant for each field component
		double lengthconv; ///< Factor to convert length units in file to PENTrack units
		double Bconv; ///< Factor to convert magnetic field units in file to PENTrack units
		double Econv; ///< Factor to convert electric field units in file to PENTrack units


		/**
		 * Reads an Opera table file.
		 *
		 * File has to contain x and z coordinates, it may contain B_x, B_y ,B_z, E_x, E_y, E_z and V columns. If V is present, E_i are ignored.
		 * Sets TabField::m, TabField::n, TabField::rdist, TabField::zdist, TabField::r_mi, TabField::z_mi according to the values in the table file which are used to determine the needed indeces on interpolation.
		 *
		 * @param tabfile Path to table file
		 * @param Bscale Magnetic field is always scaled by this factor
		 * @param Escale Electric field is always scaled by this factor
		 * @param rind Vector containing r-components of grid
		 * @param zind Vector containing z-components of grid
		 * @param BTabs Three vectors containing magnetic field components at each grid point
		 * @param ETabs Three vectors containing electric field components at each grid point
		 * @param VTab Vector containing electric potential at each grid point
		 */
		void ReadTabFile(const std::string &tabfile, alglib::real_1d_array &rind, alglib::real_1d_array &zind,
						alglib::real_1d_array BTabs[3], alglib::real_1d_array ETabs[3], alglib::real_1d_array &VTab);


		/**
		 * Print some information for each table column
		 *
		 * @param rind Vector containing r-components of grid
		 * @param zind Vector containing z-components of grid
		 * @param BTabs Three vectors containing magnetic field components at each grid point
		 * @param ETabs Three vectors containing electric field components at each grid point
		 * @param VTab Vector containing electric potential at each grid point
		 */
		void CheckTab(const alglib::real_1d_array &rind, const alglib::real_1d_array &zind,
				const alglib::real_1d_array BTabs[3], const alglib::real_1d_array ETabs[3], const alglib::real_1d_array &VTab);


	public:
		/**
		 * Constructor.
		 *
		 * Calls TabField::ReadTabFile, TabField::CheckTab and for each column TabField::PreInterpol
		 *
		 * @param tabfile Path of table file
		 * @param Bscale Time-dependent scaling formula for magnetic field
		 * @param Escale Time-dependent scaling formula for electric field
		 * @param alengthconv Factor to convert length units in file to PENTrack units (default: expect cm (cgs), convert to m)
		 * @param aBconv Factor to convert magnetic field units in file to PENTrack units (default: expect Gauss (cgs), convert to Tesla)
		 * @param aEconv Factor to convert electric field units in file to PENTrack units (default: expect V/cm (cgs), convert to V/m)
		 */
		TabField(const std::string &tabfile, const std::string &Bscale, const std::string &Escale,
				const double alengthconv = 0.01, const double aBconv = 1e-4, const double aEconv = 100.);

		~TabField();

		/**
		 * Get magnetic field at a specific point.
		 *
		 * Evaluates the interpolation polynoms and their derivatives for each field component.
		 * These radial, axial und azimuthal components have to be rotated into cartesian coordinate system.
		 *
		 * @param x X coordinate where the field shall be evaluated
		 * @param y Y coordinate where the field shall be evaluated
		 * @param z Z coordinate where the field shall be evaluated
		 * @param t Time
		 * @param B Return magnetic-field components
		 * @param dBidxj Returns spatial derivatives of magnetic-field components (optional)
		 */
		void BField(const double x, const double y, const double z, const double t, double B[3], double dBidxj[3][3] = NULL) const;


		/**
		 * Get electric field at a specific point.
		 *
		 * Evaluates the interpolation polynoms for each field component or the potential and its derivatives.
		 * These radial, axial und azimuthal components have to be rotated into cartesian coordinate system.
		 *
		 * @param x X coordinate where the field shall be evaluated
		 * @param y Y coordinate where the field shall be evaluated
		 * @param z Z coordinate where the field shall be evaluated
		 * @param t Time
		 * @param V Returns electric potential
		 * @param Ei Return electric field (negative spatial derivatives of V)
		 * @param dEidxj Return spatial derivatives of electric field components (optional) !!!NOT YET IMPLEMENTED!!!
		 */
		void EField(const double x, const double y, const double z, const double t,
				double &V, double Ei[3], double dEidxj[3][3] = NULL) const;
};


#endif // FIELD_2D_H_
