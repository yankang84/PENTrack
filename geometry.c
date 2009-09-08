#include "main.h"
#include "kdtree.h"

int reflektlog = 0;
FILE *REFLECTLOG = NULL;

KDTree geometry, sourcevolume;

struct material{
	string name;
	long double FermiReal, FermiImag, DiffProb;
};

struct solid{
	string name;
	material mat;
	unsigned kennz;
	set<int> ignoretimes;
};

vector<solid> solids;	// dynamic array to associate solids with material/kennzahl/name

vector<int> kennz_counter[3];

long double r_min = INFINITY, r_max = -INFINITY, 
			phi_min = INFINITY, phi_max = -INFINITY, 
			z_min = INFINITY, z_max = -INFINITY;


// transmission through a wall (loss of UCN) with Fermi potential  Mf (real) and PF (im) for a UCN with energy Er perpendicular to the wall (all in neV)
long double Transmission(long double Er, long double Mf, long double Pf){
return 0.2e1 * sqrtl(Er) * sqrtl(0.2e1) * sqrtl(sqrtl(Er * Er - 0.2e1 * Er * Mf + Mf * Mf + Pf * Pf) + Er - Mf) / (Er + sqrtl(Er) * sqrtl(0.2e1) * sqrtl(sqrtl(Er * Er - 0.2e1 * Er * Mf + Mf * Mf + Pf * Pf) + Er - Mf) + sqrtl(Er * Er - 0.2e1 * Er * Mf + Mf * Mf + Pf * Pf));
}


#define REFLECT_TOLERANCE 1e-6 // if the integration point is farther from the reflection point, the integration will be repeated

// load STL-files and create kd-trees
void LoadGeometry(){
	cout << endl;
	ifstream infile((inpath+"/geometry.in").c_str());
	string line;
	vector<material> materials;	// dynamic array to store material properties
	char c;
	while (infile.good()){
		infile >> ws; // ignore whitespaces
		c = infile.peek();
		if ((infile.peek() == '[') && getline(infile,line).good()){	// parse geometry.in for section header
			if (line.compare(0,11,"[MATERIALS]") == 0){
				do{	// parse material list
					infile >> ws; // ignore whitespaces
					c = infile.peek();
					if (c == '#') continue; // skip comments
					else if (!infile.good() || c == '[') break;	// next section found
					material mat;
					infile >> mat.name >> mat.FermiReal >> mat.FermiImag >> mat.DiffProb;
					materials.push_back(mat);
				}while(infile.good() && getline(infile,line).good());
			}
			else if (line.compare(0,10,"[GEOMETRY]") == 0){
				string STLfile;
				string matname;
				char name[80];
				int ignoretime;
				do{	// parse STLfile list
					infile >> ws; // ignore whitespaces
					c = infile.peek();
					if (c == '#') continue;	// skip comments
					else if (!infile.good() || c == '[') break;	// next section found
					solid model;
					infile >> model.kennz;
					if (kennz_counter[0].size() < model.kennz){
						kennz_counter[0].resize(model.kennz+1,0);
						kennz_counter[1].resize(model.kennz+1,0);
						kennz_counter[2].resize(model.kennz+1,0);
					}
					infile >> STLfile;
					infile >> matname;
					STLfile = inpath + '/' + STLfile;
					for (unsigned i = 0; i < materials.size(); i++){
						if (matname == materials[i].name){
							model.mat = materials[i];
							geometry.ReadFile(STLfile.c_str(),solids.size(),name);
							model.name = name;
							break;
						}
						else if (i+1 == materials.size()){
							fprintf(stderr,"Material %s used for %s but not defined in geometry.in!",matname.c_str(),name);
							exit(-1);
						}
					}
					while ((c = infile.peek()) == '\t' || c == ' ')
						infile.ignore();
					while (infile && c != '#' && c != '\n'){
						infile >> ignoretime;
						model.ignoretimes.insert(ignoretime);
						while ((c = infile.peek()) == '\t' || c == ' ')
							infile.ignore();
					}
					solids.push_back(model);
				}while(infile.good() && getline(infile,line).good());
				geometry.Init();
			}
			else if (line.compare(0,8,"[SOURCE]") == 0){
				do{	// parse source line
					infile >> ws; // ignore whitespaces
					c = infile.peek();
					if (c == '#') continue; // skip comments
					else if (!infile.good() || c == '[') break;	// next section found
					string name;
					infile >> name;
					if (name == "custom")
						infile >> r_min >> r_max >> phi_min >> phi_max >> z_min >> z_max;
					else{
						sourcevolume.ReadFile((inpath + '/' + name).c_str(),0);
/*						long double r,phi,z;
						infile >> r;
						infile >> phi;
						infile >> z;
						long double p[3] = {r*cos(phi),r*sin(phi),z};
*/
						sourcevolume.Init(/*p*/);
					}
				}while(infile.good() && getline(infile,line).good());
			}
			else if (line.compare(0,8,"[FIELDS]") == 0){
				do{	// parse STLfile list
					infile >> ws; // ignore whitespaces
					c = infile.peek();
					if (c == '#') continue;	// skip comments
					else if (!infile.good() || c == '[') break;	// next section found
					string ft;
					infile >> ft;
					fieldvaltab.push_back(ft);
				}while(infile.good() && getline(infile,line).good());
			}
			else getline(infile,line);
		}
		else getline(infile,line);
	}
	
	
	if(reflektlog == 1){
		ostringstream reflectlogfile;
		reflectlogfile << outpath << "/" << setw(8) << setfill('0') << jobnumber << setw(0) << "reflect.out";
		REFLECTLOG = fopen(reflectlogfile.str().c_str(),mode_w);
		fprintf(REFLECTLOG,"t r z phi x y diffuse vabs Eges Erefl winkeben winksenkr vr vz vphi phidot dvabs\n"); // Header for Reflection File
	}	
}

// return a random point in sourcevolume
void RandomPointInSourceVolume(long double &r, long double &phi, long double &z){
	long double p1[3],p2[3];
	bool valid;
	do{	
		valid = false;
		list<TCollision> c;
		if (r_min == INFINITY){
			p1[0] = p2[0] = mt_get_double(v_mt_state)*(sourcevolume.hi[0] - sourcevolume.lo[0]) + sourcevolume.lo[0]; // random point 
			p1[1] = p2[1] = mt_get_double(v_mt_state)*(sourcevolume.hi[1] - sourcevolume.lo[1]) + sourcevolume.lo[1];
			p1[2] = mt_get_double(v_mt_state)*(sourcevolume.hi[2] - sourcevolume.lo[2]) + sourcevolume.lo[2];
			p2[2] = sourcevolume.lo[2];
			valid = (sourcevolume.Collision(p1,p2,c) && c.front().normalz < 0); // random point inside source volume (surface normals pointing away from it)?
			c.clear();
			r = sqrt(p1[0]*p1[0] + p1[1]*p1[1]);
			phi = atan2(p1[1],p1[0]);
			z = p1[2];
		}
		else{
			r = sqrt(mt_get_double(v_mt_state) * (r_max*r_max - r_min*r_min) + r_min*r_min); // weighting because of the volume element and a r^2 probability outwards
			phi = mt_get_double(v_mt_state)*(phi_max - phi_min) + phi_min;
			z = mt_get_double(v_mt_state)*(z_max - z_min) + z_min;
			p1[0] = p2[0] = r*cos(phi);
			p1[1] = p2[1] = r*sin(phi);
			p1[2] = z;
			valid = true;
		}
		p2[2] = geometry.lo[2];
		if (valid && geometry.Collision(p1,p2,c)){	// test if random point is inside solid
			for (list<TCollision>::iterator i = c.begin(); i != c.end(); i++){
				list<TCollision>::iterator j = i;
				for (j++; j != c.end(); j++){
					if (i->tri == j->tri) 
						j = c.erase(j);	// delete identical entries in collision-list
				}
			}
			int count = 0;
			for (list<TCollision>::iterator i = c.begin(); i != c.end(); i++){
				if (i->normalz > 0) count++; // count surfaces whose normals point to random point
				else count--; // count surfaces whose normals point away from random point
			}
			valid &= (count == 0); // count is zero, if all surfaces are closed -> point does not lie in a solid
		}
	}while(!valid);
}

// check if a point is inside the source volume
bool InSourceVolume(long double r, long double phi, long double z){
	if (r_min == INFINITY){
		long double p1[3] = {r*cos(phi), r*sin(phi), z};
		long double p2[3] = {p1[0], p1[1], sourcevolume.lo[2]};
		list<TCollision> c;
		return (sourcevolume.Collision(p1,p2,c) && c.front().normalz < 0);
	}
	else
		return (r >= r_min && r <= r_max && phi >= phi_min && phi <= phi_max && z >= z_min && z <= z_max);
}

// check step y1->y2 for reflection, return -1 for failed reflection (step has to be repeated with smaller stepsize)
//											0 for no reflection
//											1 for reflection successful
short ReflectCheck(long double x1, long double *y1, long double &x2, long double *y2){
	long double p1[3] = {y1[1]*cos(y1[5]), y1[1]*sin(y1[5]), y1[3]}; // cart. coords
	
	if (!geometry.PointInBox(p1)){
		kennz=99;  
		stopall=1;
		printf("\nParticle has hit outer boundaries: Stopping it! t=%LG r=%LG z=%LG\n",x1,y1[1],y1[3]);
		fprintf(LOGSCR,"Particle has hit outer boundaries: Stopping it! t=%LG r=%LG z=%LG\n",x1,y1[1],y1[3]);
		return 1;
	}
	
	long double p2[3] = {y2[1]*cos(y2[5]), y2[1]*sin(y2[5]), y2[3]};
	list<TCollision> colls;
	if (geometry.Collision(p1,p2,colls)){ // search in the kdtree for reflection
		list<TCollision>::iterator it;
		for (it = colls.begin(); it != colls.end(); it++){
			if (!solids[(*it).ID].ignoretimes.empty()){
				long double x = x1 + (*it).s*(x2-x1);
				set<int> itimes = solids[(*it).ID].ignoretimes;
				if ((x < FillingTime && itimes.count(1) > 0) ||
					(x >= FillingTime && x < FillingTime + CleaningTime && itimes.count(2) > 0) ||
					(x >= FillingTime + CleaningTime && x < FillingTime + CleaningTime + RampUpTime && itimes.count(3) > 0) ||
					(x >= FillingTime + CleaningTime + RampUpTime && x < FillingTime + CleaningTime + RampUpTime + FullFieldTime && itimes.count(4) > 0) ||
					(x >= FillingTime + CleaningTime + RampUpTime + FullFieldTime && x < FillingTime + CleaningTime + RampUpTime + FullFieldTime + RampDownTime && itimes.count(5) > 0) ||
					(x >= FillingTime + CleaningTime + RampUpTime + FullFieldTime + RampDownTime && itimes.count(6) > 0))
					continue;
			}
			break;
		}
		if (it == colls.end())
			return 0;
			
		long double normal_cart[3] = {(*it).normalx,(*it).normaly,(*it).normalz};
		long double s = (*it).s;
		unsigned i = (*it).ID;
		
		long double u[3] = {p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]};
		long double dist = sqrt(u[0]*u[0] + u[1]*u[1] + u[2]*u[2]); // distance p1-p2
		long double distnormal = abs(u[0]*normal_cart[0] + u[1]*normal_cart[1] + u[2]*normal_cart[2]); // distance p1-p2 normal to surface
		if (s*distnormal > REFLECT_TOLERANCE){ // if p1 too far from surface
			s -= dist/distnormal*1e-10; // decrease s by a small amount to avoid double reflection
			x2 = x1 + s*(x2-x1); // write smaller integration time into x2
			return -1; // return fail to repeat integration step
		}
		
		long double v[3] = {y1[2], y1[1]*y1[6], y1[4]}; // reflection velocity in local cylindrical coord.
		long double vabs = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
		long double normal[3] = {normal_cart[0]*cos(y1[5]) + normal_cart[1]*sin(y1[5]), // transform normal into local cyl. coord.
								-normal_cart[0]*sin(y1[5]) + normal_cart[1]*cos(y1[5]), 
								normal_cart[2]};
		long double vnormal = v[0]*normal[0] + v[1]*normal[1] + v[2]*normal[2]; // velocity normal to reflection plane
		long double Enormal = 0.5*m_n*vnormal*vnormal; // energy normal to reflection plane
		material mat = solids[i].mat; // get material-index
		
		
		//************ handle different absorption characteristics of materials ****************
		long double prob = mt_get_double(v_mt_state);	
		if(!reflekt){
			stopall = 1;
			kennz = solids[i].kennz;
			printf("\nParticle hit %s (no reflection) at r=%LG z=%LG\n",solids[i].name.c_str(),y1[1],y1[3]);
			fprintf(LOGSCR,"Particle hit %s (no reflection) at r=%LG z=%LG\n",solids[i].name.c_str(),y1[1],y1[3]);
			return 1;
		}
		else if (prob < Transmission(Enormal*1e9,mat.FermiReal,mat.FermiImag)) // statistical absorption
		{
			stopall = 1;
			kennz = solids[i].kennz;
			printf("\nStatistical absorption at %s (%s)!\n",solids[i].name.c_str(),mat.name.c_str());
			fprintf(LOGSCR,"Statistical absorption at %s (%s)!\n",solids[i].name.c_str(),mat.name.c_str());
			return 1;
		}		
		
		//*************** specular reflexion ************
		prob = mt_get_double(v_mt_state);
		if ((diffuse == 1) || ((diffuse==3)&&(prob >= mat.DiffProb)))
		{
	    	printf("\npol %d t=%LG Erefl=%LG neV r=%LG z=%LG tol=%LG m",polarisation,x1,Enormal*1e9,y1[1],y1[3],s*distnormal);
			fprintf(LOGSCR,"pol %d t=%LG Erefl=%LG neV r=%LG z=%LG tol=%LG m\n",polarisation,x1,Enormal*1e9,y1[1],y1[3],s*distnormal);
			nrefl++;
			v[0] -= 2*vnormal*normal[0]; // reflect velocity
			v[1] -= 2*vnormal*normal[1];
			v[2] -= 2*vnormal*normal[2];
			if(reflektlog == 1)
				fprintf(REFLECTLOG,"%LG %LG %LG %LG %LG %LG 1 %LG %LG %LG %LG %LG %LG %LG %LG %LG %LG\n",
									x1,y1[1],y1[3],y1[5],p1[0],p1[1],vabs,H,Enormal*1e9,(long double)0.0,(long double)0.0,y1[2],y1[4],y1[1]*y1[6],y1[6],sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])-vabs);
		}
		
		//************** diffuse reflection ************
		else if ((diffuse == 2) || ((diffuse == 3)&&(prob < mat.DiffProb)))
		{
			long double winkeben = 0.0 + (mt_get_double(v_mt_state)) * (2 * pi); // generate random reflection angles
			long double winksenkr = acos( cos(0.0) - mt_get_double(v_mt_state) * (cos(0.0) - cos(0.5 * pi)));
			if (vnormal > 0) winksenkr += pi; // if normal points out of volume rotate by 180 degrees
			v[0] = vabs*cos(winkeben)*cos(winksenkr);	// new velocity with respect to z-axis
			v[1] = vabs*sin(winkeben)*cos(winksenkr);
			v[2] = vabs*sin(winksenkr);

			long double cosalpha = normal[2], sinalpha = sqrt(1 - cosalpha*cosalpha);	// rotation angle (angle between z and n)
			if (sinalpha > 1e-30){ // when normal not parallel to z-axis rotate new velocity into the coordinate system where the normal is the z-axis
				long double a[2] = {-normal[1]/sinalpha, normal[0]/sinalpha};	// rotation axis (z cross n), a[2] = 0
				long double vtemp[3] = {v[0],v[1],v[2]};
				// rotate velocity vector
				v[0] = (cosalpha + a[0]*a[0]*(1 - cosalpha))*	vtemp[0] +  a[0]*a[1]*(1 - cosalpha)*				vtemp[1] + a[1]*sinalpha*	vtemp[2];
				v[1] =  a[1]*a[0]*(1 - cosalpha)*				vtemp[0] + (cosalpha + a[1]*a[1]*(1 - cosalpha))*	vtemp[1] - a[0]*sinalpha*	vtemp[2];
				v[2] = -a[1]*sinalpha*							vtemp[0] +  a[0]*sinalpha*							vtemp[1] + cosalpha*		vtemp[2];
			}
	       	printf("\npol %d t=%LG Erefl=%LG neV r=%LG z=%LG w_e=%LG w_s=%LG tol=%LG m",polarisation,x1,Enormal*1e9,y1[1],y1[3],winkeben/conv,winksenkr/conv,s*distnormal);
	       	fprintf(LOGSCR,"pol %d t=%LG Erefl=%LG neV r=%LG z=%LG w_e=%LG w_s=%LG tol=%LG m\n",polarisation,x1,Enormal*1e9,y1[1],y1[3],winkeben/conv,winksenkr/conv,s*distnormal);
	       	nrefl++;
	       	if(reflektlog == 1)
				fprintf(REFLECTLOG,"%LG %LG %LG %LG %LG %LG 2 %LG %LG %LG %LG %LG %LG %LG %LG %LG %LG\n",
									x1,y1[1],y1[3],y1[5],p1[0],p1[1],vabs,H,Enormal*1e9,winkeben,winksenkr,y1[2],y1[4],y1[1]*y1[6],y1[6],sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])-vabs);
		}
		y1[2] = v[0]; // write new velocity into y1-vector
		y1[4] = v[2];
		y1[6] = v[1]/y1[1];
		
		return 1;
	}
	return 0;
}

//======== The endcodes of trajectories are added up here regarding the particle typ =======================================
void IncrementCodes(int kennz)
{	// protneut % 3 = {0, 1, 2} <<<	ELECTRONS (=6) % 3 = 0
	// 								NEUTRON   (=1) % 3 = 1	
	// 								PROTON    (=2) % 3 = 2
	kennz_counter[protneut % 3][kennz]++;
}
//======== end of IncrementCodes ===========================================================================================

//======== Output of the endcodes ==========================================================================================
void OutputCodes(int iMC){
	int ncount = accumulate(kennz_counter[1].begin(),kennz_counter[1].end(),0);
	int pcount = accumulate(kennz_counter[2].begin(),kennz_counter[2].end(),0);
	int ecount = accumulate(kennz_counter[0].begin(),kennz_counter[0].end(),0);
	printf("\nThe calculations of %li particle(s) yielded:\n"
	       "endcode:  of %4i neutron(s) ; of %4i proton(s) ; of %4i electron(s)\n"
	       "   0 %12i %20i %19i 		(were not categorized)\n"
	       "   1 %12i %20i %19i 		(did not finish)\n"
	       "   2 %12i %20i %19i 		(hit outer boundaries)\n"
	       "   3 %12i %20i %19i 		(left field boundaries)\n"
	       "   4 %12i %20i %19i 		(decayed)\n"
	       "   5 %12i %20i %19i 		(found no initial position)\n",
	       (iMC - 1 + 2 * decay.counter),
	       ncount, pcount, ecount,
	       kennz_counter[1][0], kennz_counter[2][0], kennz_counter[0][0],
	       kennz_counter[1][1], kennz_counter[2][1], kennz_counter[0][1],
	       kennz_counter[1][2], kennz_counter[2][2], kennz_counter[0][2],
	       kennz_counter[1][3], kennz_counter[2][3], kennz_counter[0][3],
	       kennz_counter[1][4], kennz_counter[2][4], kennz_counter[0][4],
	       kennz_counter[1][5], kennz_counter[2][5], kennz_counter[0][5]);
	for (unsigned i = 6; i < kennz_counter[0].size(); i++){
		string solidnames;
		for (vector<solid>::iterator it = solids.begin(); it != solids.end(); it++)
			if (it->kennz == i)
				solidnames += '/' + it->name;
		printf("  %2i %12i %20i %19i		(were statistically absorbed by %s)\n",
				i,kennz_counter[1][i],kennz_counter[2][i],kennz_counter[0][i],solidnames.c_str()+1);
	}
				
	fprintf(LOGSCR,"\nThe calculations of %li particle(s) yielded:\n"
	       "endcode:  of %4i neutron(s) ; of %4i proton(s) ; of %4i electron(s)\n"
	       "   0 %12i %20i %19i 		(were not categorized)\n"
	       "   1 %12i %20i %19i 		(did not finish)\n"
	       "   2 %12i %20i %19i 		(hit outer boundaries)\n"
	       "   3 %12i %20i %19i 		(left field boundaries)\n"
	       "   4 %12i %20i %19i 		(decayed)\n"
	       "   5 %12i %20i %19i 		(found no initial position)\n",
	       (iMC - 1 + 2 * decay.counter),
	       ncount, pcount, ecount,
	       kennz_counter[1][0], kennz_counter[2][0], kennz_counter[0][0],
	       kennz_counter[1][1], kennz_counter[2][1], kennz_counter[0][1],
	       kennz_counter[1][2], kennz_counter[2][2], kennz_counter[0][2],
	       kennz_counter[1][3], kennz_counter[2][3], kennz_counter[0][3],
	       kennz_counter[1][4], kennz_counter[2][4], kennz_counter[0][4],
	       kennz_counter[1][5], kennz_counter[2][5], kennz_counter[0][5]);
	for (unsigned i = 6; i < kennz_counter[0].size(); i++){
		string solidnames;
		for (vector<solid>::iterator it = solids.begin(); it != solids.end(); it++)
			if (it->kennz == i)
				solidnames += '/' + it->name;
		fprintf(LOGSCR,"  %2i %12i %20i %19i		(were statistically absorbed by %s)\n",
				i,kennz_counter[1][i],kennz_counter[2][i],kennz_counter[0][i],solidnames.c_str()+1);
	}
}
//======== end of OutputCodes ==============================================================================================
