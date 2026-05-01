#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_all.hpp>



#include "loomrv/interval_set.hpp"
#include <ostream>

// Use the namespace for cleaner tests
using namespace db_interval_set;


TEST_CASE("Basic Set Creation and Lifecycle", "[interval_set]") {
    
    SECTION("newHolder and destroyHolder") {
        // This test mainly checks that the code compiles and doesn't crash.
        // True memory leak detection would require a tool like Valgrind.
        IntervalSetHolder holder = newHolder(128);
        REQUIRE(holder.writeIndex == 0);
        REQUIRE(holder.readBuffer != nullptr);
        REQUIRE(holder.writeBuffer != nullptr);
        destroyHolder(holder);
    }

    SECTION("swapBuffers") {
        IntervalSetHolder holder = newHolder(128);
        Transition* readPtr = holder.readBuffer;
        Transition* writePtr = holder.writeBuffer;
        holder.writeIndex = 5; // Set a dummy index

        swapBuffers(holder);

        // Pointers should be swapped
        REQUIRE(holder.readBuffer == writePtr);
        REQUIRE(holder.writeBuffer == readPtr);
        
        // Write index should be reset
        REQUIRE(holder.writeIndex == 0);

        destroyHolder(holder);
    }

    SECTION("empty set") {
        IntervalSetHolder holder = newHolder(128);
        IntervalSet s = empty(holder);
        
        // Use the helper to convert back to intervals
        std::vector<Interval> v = toVectorIntervals(s);
        
        REQUIRE(v.empty());
        destroyHolder(holder);
    }

    SECTION("fromInterval") {
        IntervalSetHolder holder = newHolder(128);
        
        // Test a valid interval
        IntervalSet s1 = fromInterval(holder, {10, 20});
        std::vector<Interval> v1 = toVectorIntervals(s1);
        REQUIRE(v1 == std::vector<Interval>{{10, 20}});

        // Test an empty interval
        IntervalSet s2 = fromInterval(holder, {30, 30});
        std::vector<Interval> v2 = toVectorIntervals(s2);
        REQUIRE(v2.empty());

        // Test an invalid interval
        IntervalSet s3 = fromInterval(holder, {40, 30});
        std::vector<Interval> v3 = toVectorIntervals(s3);
        REQUIRE(v3.empty());
        
        destroyHolder(holder);
    }
}


TEST_CASE("Union Operations (unionSets)", "[interval_set]") {
    IntervalSetHolder holder = newHolder(1024);

    // This is the core "double buffer" pattern for testing operations:
    // 1. Create all your *input* sets in the writeBuffer.
    // 2. swapBuffers() to move them to readBuffer.
    // 3. Define IntervalSet structs pointing to the correct slices in readBuffer.
    // 4. Run the operation (e.g., unionSets), which writes to writeBuffer.
    // 5. Convert the *result* (which is in writeBuffer) to a vector for checking.

    SECTION("Simple Overlap") {
        // A = {10, 20}, B = {15, 25} -> Union = {10, 25}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = fromInterval(holder, {15, 25});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = unionSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{10, 25}});
    }

    SECTION("Adjacent") {
        // A = {10, 20}, B = {20, 30} -> Union = {10, 30}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = fromInterval(holder, {20, 30});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = unionSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{10, 30}});
    }

    SECTION("Disjoint") {
        // A = {10, 20}, B = {30, 40} -> Union = {10, 20}, {30, 40}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = fromInterval(holder, {30, 40});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = unionSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{10, 20}, {30, 40}});
    }

    SECTION("Contained") {
        // A = {10, 40}, B = {20, 30} -> Union = {10, 40}
        auto s1_t = fromInterval(holder, {10, 40});
        auto s2_t = fromInterval(holder, {20, 30});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = unionSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{10, 40}});
    }

    SECTION("With Empty Set") {
        // A = {10, 20}, B = {} -> Union = {10, 20}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = empty(holder);
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = unionSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{10, 20}});
    }

    SECTION("Complex Multi-Interval Union") {
        // A = {10, 20} U {30, 40}
        // B = {15, 35}
        // Result = {10, 40}

        // 1. Create {10, 20} and {30, 40}
        auto i1_t = fromInterval(holder, {10, 20});
        auto i2_t = fromInterval(holder, {30, 40});
        swapBuffers(holder);
        
        // 2. Union them to create Set A
        IntervalSet i1 = {holder.readBuffer, i1_t.startIndex, i1_t.endIndex};
        IntervalSet i2 = {holder.readBuffer, i2_t.startIndex, i2_t.endIndex};
        auto setA_t = unionSets(holder, i1, i2); // setA_t is in writeBuffer
        
        // 3. Create Set B in writeBuffer
        auto setB_t = fromInterval(holder, {15, 35});

        // 4. Swap. Now setA and setB are both in readBuffer
        swapBuffers(holder);
        
        // 5. Define the final input sets
        IntervalSet setA = {holder.readBuffer, setA_t.startIndex, setA_t.endIndex};
        IntervalSet setB = {holder.readBuffer, setB_t.startIndex, setB_t.endIndex};
        
        // 6. Run the final operation
        IntervalSet result = unionSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{10, 40}});
    }

    destroyHolder(holder);
}

TEST_CASE("Intersection Operations (intersectSets)", "[interval_set]") {
    IntervalSetHolder holder = newHolder(1024);

    SECTION("Simple Overlap") {
        // A = {10, 20}, B = {15, 25} -> Intersect = {15, 20}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = fromInterval(holder, {15, 25});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = intersectSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{15, 20}});
    }

    SECTION("Adjacent") {
        // A = {10, 20}, B = {20, 30} -> Intersect = {}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = fromInterval(holder, {20, 30});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = intersectSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v.empty());
    }

    SECTION("Disjoint") {
        // A = {10, 20}, B = {30, 40} -> Intersect = {}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = fromInterval(holder, {30, 40});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = intersectSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v.empty());
    }

    SECTION("Contained") {
        // A = {10, 40}, B = {20, 30} -> Intersect = {20, 30}
        auto s1_t = fromInterval(holder, {10, 40});
        auto s2_t = fromInterval(holder, {20, 30});
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = intersectSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{20, 30}});
    }

    SECTION("With Empty Set") {
        // A = {10, 20}, B = {} -> Intersect = {}
        auto s1_t = fromInterval(holder, {10, 20});
        auto s2_t = empty(holder);
        swapBuffers(holder);

        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet setB = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        IntervalSet result = intersectSets(holder, setA, setB);
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v.empty());
    }

    destroyHolder(holder);
}


TEST_CASE("Negation Operations (negateSet)", "[interval_set]") {
    IntervalSetHolder holder = newHolder(1024);
    Interval domain = {0, 100};

    SECTION("Simple Negation") {
        // A = {10, 20}, Domain = {0, 100} -> Negate = {0, 10}, {20, 100}
        auto s1_t = fromInterval(holder, {10, 20});
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{0, 10}, {20, 100}});
    }

    SECTION("Set touches domain start") {
        // A = {0, 10}, Domain = {0, 100} -> Negate = {10, 100}
        auto s1_t = fromInterval(holder, {0, 10});
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{10, 100}});
    }

    SECTION("Set touches domain end") {
        // A = {90, 100}, Domain = {0, 100} -> Negate = {0, 90}
        auto s1_t = fromInterval(holder, {90, 100});
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{0, 90}});
    }

    SECTION("Set covers domain") {
        // A = {0, 100}, Domain = {0, 100} -> Negate = {}
        auto s1_t = fromInterval(holder, {0, 100});
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v.empty());
    }

    SECTION("Empty set") {
        // A = {}, Domain = {0, 100} -> Negate = {0, 100}
        auto s1_t = empty(holder);
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{0, 100}});
    }

    SECTION("Set partially outside domain") {
        // A = {-10, 10}, Domain = {0, 100} -> Negate = {10, 100}
        auto s1_t = fromInterval(holder, {-10, 10});
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{10, 100}});
    }

    SECTION("Set fully outside domain") {
        // A = {-20, -10}, Domain = {0, 100} -> Negate = {0, 100}
        auto s1_t = fromInterval(holder, {-20, -10});
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{0, 100}});
    }

    SECTION("Multi-interval set") {
        // A = {10, 20} U {50, 60}
        // Domain = {0, 100}
        // Negate = {0, 10}, {20, 50}, {60, 100}
        
        // 1. Create set A = {10, 20} U {50, 60}
        auto i1_t = fromInterval(holder, {10, 20});
        auto i2_t = fromInterval(holder, {50, 60});
        swapBuffers(holder);
        IntervalSet i1 = {holder.readBuffer, i1_t.startIndex, i1_t.endIndex};
        IntervalSet i2 = {holder.readBuffer, i2_t.startIndex, i2_t.endIndex};
        auto setA_t = unionSets(holder, i1, i2);
        
        // 2. Swap so setA is in readBuffer
        swapBuffers(holder);
        IntervalSet setA = {holder.readBuffer, setA_t.startIndex, setA_t.endIndex};

        // 3. Run negation
        IntervalSet result = negateSet(holder, setA, domain);
        std::vector<Interval> v = toVectorIntervals(result);
        
        REQUIRE(v == std::vector<Interval>{{0, 10}, {20, 50}, {60, 100}});
    }

    destroyHolder(holder);
}



// Helper struct for testing segments
struct ExpectedSegment {
    db_interval_set::Interval interval;
    bool a;
    bool b;

    // Overload operator== for Catch2 vector comparison
    bool operator==(const ExpectedSegment& other) const {
        return interval == other.interval && a == other.a && b == other.b;
    }
    
    // Friend function for printing
    friend std::ostream& operator<<(std::ostream& os, const ExpectedSegment& seg) {
        os << "Seg{ " << seg.interval << ", A:" << (seg.a ? "T" : "F")
           << ", B:" << (seg.b ? "T" : "F") << " }";
        return os;
    }
};

// Helper function to run the iteration and collect results
std::vector<ExpectedSegment> runIteration(
    db_interval_set::IntervalSet setA,
    db_interval_set::IntervalSet setB,
    db_interval_set::Interval domain)
{
    std::vector<ExpectedSegment> results;
    
    db_interval_set::SegmentIterator it = 
        db_interval_set::createSegmentIterator(setA, setB, domain);
    
    while (db_interval_set::getNextSegment(it)) {
        // Only add non-empty segments
        if (it.interval.start != it.interval.end) {
             results.push_back(ExpectedSegment{ {it.interval.start, it.interval.end}, it.leftTruthy, it.rightTruthy });
        }
    }
    
    return results;
}


TEST_CASE("SegmentIterator tests", "[interval_set]") {
    using namespace db_interval_set;
    
    IntervalSetHolder holder = newHolder(1024);

    // setA = [10, 20) U [30, 40)
    IntervalSet setA = createSetFromIntervals(holder, {
        {10, 20}, {30, 40}
    });

    // setB = [15, 35)
    IntervalSet setB = fromInterval(holder, {15, 35});
    
    // An empty set for testing
    IntervalSet setEmpty = empty(holder);

    SECTION("Full Domain Iteration [0, 50)") {
        Interval domain = {0, 50};
        std::vector<ExpectedSegment> results = runIteration(setA, setB, domain);

        std::vector<ExpectedSegment> expected = {
            { {0, 10},  false, false },
            { {10, 15}, true,  false },
            { {15, 20}, true,  true  },
            { {20, 30}, false, true  },
            { {30, 35}, true,  true  },
            { {35, 40}, true,  false },
            { {40, 50}, false, false }
        };
        
        REQUIRE(results == expected);
    }

    SECTION("Partial Domain [12, 32)") {
        Interval domain = {12, 32};
        std::vector<ExpectedSegment> results = runIteration(setA, setB, domain);

        std::vector<ExpectedSegment> expected = {
            { {12, 15}, true,  false },
            { {15, 20}, true,  true  },
            { {20, 30}, false, true  },
            { {30, 32}, true,  true  }
        };
        
        REQUIRE(results == expected);
    }

    SECTION("Domain before all intervals [0, 5)") {
        Interval domain = {0, 5};
        std::vector<ExpectedSegment> results = runIteration(setA, setB, domain);

        std::vector<ExpectedSegment> expected = {
            { {0, 5}, false, false }
        };
        
        REQUIRE(results == expected);
    }

    SECTION("Domain after all intervals [45, 55)") {
        Interval domain = {45, 55};
        std::vector<ExpectedSegment> results = runIteration(setA, setB, domain);
   
        std::vector<ExpectedSegment> expected= {
            { {45, 55}, false, false }
        };

        REQUIRE(results == expected);
    }
    
    SECTION("Empty Domain [5, 5)") {
        Interval domain = {5, 5};
        std::vector<ExpectedSegment> results = runIteration(setA, setB, domain);
        std::vector<ExpectedSegment> expected = {}; // No non-empty segments
        
        REQUIRE(results == expected);
    }

    SECTION("Iteration with an empty set") {
        Interval domain = {0, 20};
        // setA = empty, setB = [15, 35) (but we only care up to 20)
        std::vector<ExpectedSegment> results = runIteration(setEmpty, setB, domain);

        std::vector<ExpectedSegment> expected = {
            { {0, 15},  false, false },
            { {15, 20}, false, true  }
        };
        
        REQUIRE(results == expected);
    }
    
    SECTION("Iteration with two empty sets") {
        Interval domain = {0, 20};
        std::vector<ExpectedSegment> results = runIteration(setEmpty, setEmpty, domain);

        std::vector<ExpectedSegment> expected = {
            { {0, 20}, false, false }
        };
        
        REQUIRE(results == expected);
    }
    
    SECTION("Domain exactly matching a transition [10, 15)") {
        Interval domain = {10, 15};
        std::vector<ExpectedSegment> results = runIteration(setA, setB, domain);
        
        std::vector<ExpectedSegment> expected = {
            { {10, 15}, true, false }
        };
        
        REQUIRE(results == expected);
    }

    destroyHolder(holder);
}

TEST_CASE("Optimized Union (unionIntervalFromRight)", "[interval_set]") {
    IntervalSetHolder holder = newHolder(1024);

    SECTION("Disjoint (Append to End)") {
        // Set: {[0, 10)}, Interval: [20, 30)
        // Expected: {[0, 10), [20, 30)}
        
        auto s1_t = fromInterval(holder, {0, 10});
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        IntervalSet result = unionIntervalFromRight(holder, set, {20, 30});
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{0, 10}, {20, 30}});
    }

    SECTION("Touching (Merge)") {
        // Set: {[0, 10)}, Interval: [10, 20)
        // Expected: {[0, 20)}
        // This validates that it doesn't leave a gap or duplicate the "10" transition.
        
        auto s1_t = fromInterval(holder, {0, 10});
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        IntervalSet result = unionIntervalFromRight(holder, set, {10, 20});
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{0, 20}});
    }

    SECTION("Overlapping (Merge/Extension)") {
        // Set: {[0, 20)}, Interval: [15, 30)
        // Expected: {[0, 30)}
        // This validates that the old end (20) is removed and replaced by new end (30).
        
        auto s1_t = fromInterval(holder, {0, 20});
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        IntervalSet result = unionIntervalFromRight(holder, set, {15, 30});
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{0, 30}});
    }

    SECTION("Complex Multi-Interval Extension") {
        // Set: {[0, 10), [20, 30)}, Interval: [25, 40)
        // Expected: {[0, 10), [20, 40)}
        // Validates that previous intervals (0-10) are copied correctly unmodified.
        
        // 1. Create the complex set manually
        auto s1_t = fromInterval(holder, {0, 10});
        auto s2_t = fromInterval(holder, {20, 30});
        swapBuffers(holder);
        
        IntervalSet i1 = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};
        IntervalSet i2 = {holder.readBuffer, s2_t.startIndex, s2_t.endIndex};
        
        // Union them to create the input set in writeBuffer
        auto inputSet_t = unionSets(holder, i1, i2);
        
        // Swap so inputSet is now in readBuffer
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, inputSet_t.startIndex, inputSet_t.endIndex};

        // Act
        IntervalSet result = unionIntervalFromRight(holder, set, {25, 40});
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{0, 10}, {20, 40}});
    }

    SECTION("Empty Set Handling") {
        // Set: {}, Interval: [5, 10)
        // Expected: {[5, 10)}
        
        auto s1_t = empty(holder);
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        IntervalSet result = unionIntervalFromRight(holder, set, {5, 10});
        std::vector<Interval> v = toVectorIntervals(result);

        REQUIRE(v == std::vector<Interval>{{5, 10}});
    }

    destroyHolder(holder);
}


TEST_CASE("Optimized Check and Clip (checkAndClip)", "[interval_set]") {
    IntervalSetHolder holder = newHolder(1024);
    int time = 10;

    SECTION("Case A: Set starts in the future (> time)") {
        // Set: {[15, 20)}, Time: 10
        // Expected: output=false, Set unchanged {[15, 20)}
        
        auto s1_t = fromInterval(holder, {15, 20});
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        CheckAndClipResult result = checkAndClip(holder, set, time);
        std::vector<Interval> v = toVectorIntervals(result.set);

        // Assert
        REQUIRE(result.output == false);
        REQUIRE(v == std::vector<Interval>{{15, 20}});
    }

    SECTION("Case B: Set starts exactly at time") {
        // Set: {[10, 20)}, Time: 10
        // Expected: output=true, Set clipped to {[11, 20)}
        
        auto s1_t = fromInterval(holder, {10, 20});
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        CheckAndClipResult result = checkAndClip(holder, set, time);
        std::vector<Interval> v = toVectorIntervals(result.set);

        // Assert
        REQUIRE(result.output == true);
        REQUIRE(v == std::vector<Interval>{{11, 20}});
    }

    SECTION("Case C: Micro-Optimization (Result becomes empty)") {
        // Set: {[10, 11)}, Time: 10
        // Expected: output=true
        // Logic: Clipping [10, 11) to [11, INF) leaves [11, 11), which is empty.
        
        auto s1_t = fromInterval(holder, {10, 11});
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        CheckAndClipResult result = checkAndClip(holder, set, time);
        std::vector<Interval> v = toVectorIntervals(result.set);

        // Assert
        REQUIRE(result.output == true);
        REQUIRE(v.empty());
    }

    SECTION("Case D: Multi-Interval Preservation") {
        // Set: {[10, 15), [20, 25)}, Time: 10
        // Expected: output=true, Set becomes {[11, 15), [20, 25)}
        
        // Setup complex set
        auto i1 = fromInterval(holder, {10, 15});
        auto i2 = fromInterval(holder, {20, 25});
        swapBuffers(holder);
        // Union to make one set
        auto set_t = unionSets(holder, 
            {holder.readBuffer, i1.startIndex, i1.endIndex}, 
            {holder.readBuffer, i2.startIndex, i2.endIndex}
        );
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, set_t.startIndex, set_t.endIndex};

        // Act
        CheckAndClipResult result = checkAndClip(holder, set, time);
        std::vector<Interval> v = toVectorIntervals(result.set);

        // Assert
        REQUIRE(result.output == true);
        REQUIRE(v == std::vector<Interval>{{11, 15}, {20, 25}});
    }

    SECTION("Case E: Empty Input Set") {
        // Set: {}, Time: 10
        // Expected: output=false, Set empty
        
        auto s1_t = empty(holder);
        swapBuffers(holder);
        IntervalSet set = {holder.readBuffer, s1_t.startIndex, s1_t.endIndex};

        // Act
        CheckAndClipResult result = checkAndClip(holder, set, time);
        std::vector<Interval> v = toVectorIntervals(result.set);

        // Assert
        REQUIRE(result.output == false);
        REQUIRE(v.empty());
    }
    

    destroyHolder(holder);
}