#ifndef GNUPLOT_H
#define GNUPLOT_H

#include <stdio.h>
#include <vector>
#include <string>

class GnuPlot
{
public:
    GnuPlot(std::vector<std::string>& legends);
    void Plot(std::vector<float>& values);
private:
    FILE* gfeed_;
};

#endif // GNUPLOT_H
