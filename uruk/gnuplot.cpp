#include "gnuplot.h"

GnuPlot::GnuPlot() {
    gfeed_ = popen("feedgnuplot --lines --nodomain --ymin -0.5 --ymax 4.5 \
                   --legend 0 \"right mu power\" --legend 1 \"right mu power\" \
                   --legend 2 \"left beta power\" --legend 3 \"right beta power\"\
                   --stream -xlen 400 --geometry 940x450-0+0", "w");
}

void GnuPlot::Plot(float value) {
    fprintf(gfeed_, "%f \n", value);
    fprintf(gfeed_, "replot\n");
    fflush(gfeed_);
}
