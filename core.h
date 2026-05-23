#ifndef CORE_H
#define CORE_H
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <cstdlib>
#define time_unit 365 //according to your data
const int state=3;//you can change it.
const double init_cash=1000,FEE=0.001;//you can change it too.
struct kline{
	std::string t;
	double o,h,l,c;
};
struct SA_state{
	double t[state],u[state];
};
struct strategy{
	double p[state+1],s[state+1];
};
struct result{
	std::vector<double> line;
	double score;
};
#endif
