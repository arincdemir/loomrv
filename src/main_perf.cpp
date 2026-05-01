#include <array>
#include <cstddef>
#include <iostream>
#include <string>

#include <loomrv/json_feeder.hpp>
#include <loomrv/ptl.hpp>
#include <loomrv/MTLEngine.hpp>

using namespace db_interval_set;
using namespace loomrv;

int main(int argc, char **argv)
{
    // Repeat for perf measurement, adjust iterations if necessary
    for (int i = 0; i < 100; i++)
    {
        // Adjust buffer size as needed based on the formula and sequence length
        auto monitor = createDenseMultiPropertyMonitor(200); 
        
        ptl_parser parser;
        // Replaced "or" with "||" to match parser syntax correctly based on the documentation
        parser.parse_dense("historically(({r} && !{q} && once {q}) -> ((once[:10]({p} || {q})) since {q}))", monitor);
        
        // Finalize monitor to compute proposition metadata
        finalize_monitor(monitor);

        auto *feeder = create_dense_json_feeder(monitor, "/home/arincdemir/workspace/loomrv/data/fullsuite/RecurBQR/Dense1/1M/RecurBQR10.jsonl");
        
        while (feed_next(feeder)) {
            // Processing loop, pulling and consuming results
        }
        
        destroy_feeder(feeder);
    }
    return 0;
}
