
#include "locations.h"

#include "fracfast/types.h"


namespace Locations {
    const Location home = {{-2.0, 1.0, -1.125, 1.125}, {8000, 6000}, 1000};
    const Location limit = {{-0.7500362982234245, -0.7500362982234176, -0.004929824108246969, -0.004929824108242316}, {1920, 1080}, 1000};
    const Location sym = {{-2.0, 2.0, -1.5, 1.5}, {1600, 1200}, 256};

    // Average domain locations
    const Resolution averageRes = {1920, 1080};
    const Location a = {{-0.6701, -0.6641, 0.4539, 0.4573}, averageRes, 600},
                   b = {{-1.74858614, -1.74858479, 0.01262719, 0.01262795}, averageRes, 2500},
                   c = {{-0.74766, -0.74728, 0.08290, 0.08312}, averageRes, 2250},
                   d = {{-0.16663, -0.16650, 1.04049, 1.04057}, averageRes, 1500},
                   e = {{-1.26, -1.195, 0.1417, 0.1782}, averageRes, 700},
                   f = {{-0.753, -0.727, 0.1441, 0.1588}, averageRes, 350},
                   g = {{-0.7475087485, -0.7475087322, 0.0830715266, 0.0830715359}, averageRes, 1000},
                   h = {{-0.439165, -0.439089, 0.574562, 0.574604}, averageRes, 450},
                   i = {{-0.439165, -0.43909, 0.574507, 0.574549}, averageRes, 475};
}
