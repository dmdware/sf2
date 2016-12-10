










#ifndef BARYCENTRIC_H
#define BARYCENTRIC_H

double triarea(double a, double b, double c);

double dist(double x0, double y0, double z0, double x1, double y1, double z1);

void barycent(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2,
			  double vx, double vy, double vz,
			  double *u, double *v, double *w);

#endif
