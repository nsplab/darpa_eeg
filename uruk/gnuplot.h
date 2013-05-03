#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdio.h>

class GnuPlot
{
public:
    GnuPlot();
    void Plot(float value);
private:
    FILE* gfeed_;
};

#endif // GNUPLOT_H
