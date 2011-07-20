#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "main.h"
#include "particle.h"
#include "boundaries.h"
#include "output.h"

extern double OMEGA;
extern double coefficient_of_restitution;
extern double minimum_collision_velocity;
double particle_radius = 1;

void problem_init(int argc, char* argv[]){
	// Setup constants
	OMEGA = 1.;
	G=1;
	// Setup particle structures
	double r_hstar = 0.5;
	double r_h = r_hstar*2.*particle_radius;
	double particle_mass = r_h*r_h*r_h*3./2.;
	double tau = 0.5;
	double surface_density = tau/(M_PI*particle_radius*particle_radius)*particle_mass;
	double lambda_crit = 4.*M_PI*M_PI*G*surface_density/OMEGA/OMEGA;
	boxsize = 50;
	N = (int)round(tau*boxsize*boxsize/(M_PI*particle_radius*particle_radius));
	printf("lambda_crit = %f\nsurface_density = %f\nboxsize = %f\n",lambda_crit,surface_density,boxsize);
	init_particles(N);
	coefficient_of_restitution = 0.5;
	minimum_collision_velocity = 0.01*OMEGA*particle_radius;
	dt = 1e-2;
	//tmax=10.*2.*M_PI;
	softening = 0.1*particle_radius;
	// Initial conditions
	for (int i =0;i<N;i++){
		particles[i].x = ((double)rand()/(double)RAND_MAX-0.5)*boxsize_x;
		particles[i].y = ((double)rand()/(double)RAND_MAX-0.5)*boxsize_y;
		particles[i].z = 2.*particle_radius*((double)rand()/(double)RAND_MAX-0.5);
		particles[i].vx = 0;
		particles[i].vy = -1.5*particles[i].x*OMEGA;
		particles[i].vz = 0;
		particles[i].ax = 0;
		particles[i].ay = 0;
		particles[i].az = 0;
		particles[i].m = particle_mass;
		particles[i].r = particle_radius;
	}
	// Do use ghost boxes in x and y
	nghostx = 3;
	nghosty = 3;
	nghostz = 0;
}

void problem_inloop(){

}

extern double collisions_plog;
double collisions_plog_last =0;
void calculate_viscosity(double* nutrans, double* nucoll, double* nugrav){
	double _nutrans =0;
	double _nugrav  =0;
	double mtot =0;
	for (int i=0;i<N;i++){
		struct particle p = particles[i];
		mtot += p.m;
		double vyr = p.vy + 1.5 * OMEGA * p.x;
		_nutrans += p.m*p.vx*vyr;
	}
	*nutrans = _nutrans*2./(3.*OMEGA*mtot);
	*nucoll  = _nutrans*2./(3.*OMEGA*mtot)*1./(t-collisions_plog_last)*collisions_plog;
	*nugrav  = _nugrav;
}

double nutrans_tot=0, nucoll_tot=0, nugrav_tot=0; int nuN=0;
void calculate_viscosity_tot(){
	double nutrans,nucoll,nugrav;
	calculate_viscosity(&nutrans,&nucoll,&nugrav);
	nutrans_tot +=nutrans;
	nucoll_tot  +=nucoll;
	nugrav_tot  +=nugrav;
	collisions_plog_last=t;
	collisions_plog = 0;
	nuN++;
}

void output_append_viscosity(char* filename){
	FILE* of = fopen(filename,"a"); 
	fprintf(of,"%e\t%e\t%e\t%e\n",t,nutrans_tot/(particle_radius*particle_radius*OMEGA)/(double)nuN,nucoll_tot/(particle_radius*particle_radius*OMEGA)/(double)nuN,nugrav_tot/(particle_radius*particle_radius*OMEGA)/(double)nuN);
	fclose(of);
}

void problem_output(){
	if (output_check(1e-1*2.*M_PI/OMEGA)&&t>0){
		calculate_viscosity_tot();
		output_append_viscosity("viscosity.txt");
	}
}


void problem_finish(){
	FILE* of = fopen("error.txt","a+"); 
	double error= 0;
	struct timeval tim;
	gettimeofday(&tim, NULL);
	double timing_final = tim.tv_sec+(tim.tv_usec/1000000.0);
	double error_limit = 0.1;
	int pass = (error>error_limit?0:1);
	fprintf(of,"%d\t%e\t%e\t%e\t%d\t%e\t",N,dt,error,error_limit,pass,timing_final-timing_initial);
	fprintf(of,"N_init = %d",0);
	fprintf(of,"\n");
	fclose(of);

}
