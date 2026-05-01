#pragma once

#include <algorithm>
#include <vector>
#include <limits>
#include <iostream> // Included for operator<< helper

namespace db_interval_set {

// The 'Interval' struct is just a plain data structure for the API.
// It is NOT the internal storage format.
struct Interval {
    int start;
    int end;
};

// This is the core internal data structure.
// Represents a "half-interval" or a "transition point".
struct Transition {
    int time;
    bool isStart; // true = start of an interval, false = end of an interval
};

struct IntervalSet {
    Transition *buffer;
    int startIndex;
    int endIndex;
};

struct IntervalSetHolder {
    Transition *readBuffer;
    Transition *writeBuffer;
    int writeIndex;
    int bufferSize;
};

// --- THIS IS THE STRUCT YOU PROPOSED ---
// I've renamed it to SegmentIterator as it holds the iterator state.
// I also added the 'domain' field which is necessary to bound the iteration.
struct SegmentIterator {
    // --- Result fields (valid after getNextSegment returns true) ---
    bool leftTruthy;  // The boolean state of leftIntervalSet during the 'interval'
    bool rightTruthy; // The boolean state of rightIntervalSet during the 'interval'
    Interval interval;  // The interval [start, end) for this segment

    // --- Config fields (set at creation) ---
    IntervalSet leftIntervalSet;
    IntervalSet rightIntervalSet;
    Interval domain; // The "world" boundaries for the iteration

    // --- State fields (used internally by the iterator) ---
    int leftIndex;  // The next transition to read from leftIntervalSet
    int rightIndex; // The next transition to read from rightIntervalSet
    
    // --- (FIX) ADDED STATE FIELD ---
    // This holds the start time and state *for the next segment*.
    // The 'result' fields (leftTruthy, rightTruthy, interval) will
    // hold the data for the *completed* segment.
    int currentSegmentStart;
    bool currentLeftTruthy;
    bool currentRightTruthy;
};

struct CheckAndClipResult {
    bool output;      // Was 'time' in the set?
    IntervalSet set;  // The new clipped set (starting >= time + 1)
};


IntervalSetHolder newHolder(int bufferSize);

void swapBuffers(IntervalSetHolder &holder);

IntervalSet empty(IntervalSetHolder &holder);

/**
 * @brief Checks if a single time point is contained within the interval set.
 * Since intervals are [start, end), start is inclusive and end is exclusive.
 *
 * @param set The interval set to check.
 * @param time The time point to query.
 * @return true if the time point is in the set, false otherwise.
 */
bool includes(const IntervalSet& set, int time);

/**
 * @brief Creates a new set from a single [start, end) interval.
 * This is the primary way to get data into the system.
 */
IntervalSet fromInterval(IntervalSetHolder &holder, Interval interval);

/**
 * @brief (NEW) Copies a set from either buffer to the end of the write buffer.
 * This is crucial for carrying over unmodified sets
 * before calling swapBuffers().
 */
IntervalSet copySet(IntervalSetHolder& holder, IntervalSet set);

/**
 * @brief Computes the union (OR) of two sets using a plane-sweep algorithm.
 */
IntervalSet unionSets(IntervalSetHolder &holder, IntervalSet setA, IntervalSet setB);

/**
 * @brief Computes the union of a set with an interval, where the interval 
 * is guaranteed to be towards the right, and have at most one overlap.
 */
IntervalSet unionIntervalFromRight(IntervalSetHolder &holder, IntervalSet set, Interval interval);


/**
 * @brief Computes the intersection (AND) of two sets.
 */
IntervalSet intersectSets(IntervalSetHolder &holder, IntervalSet setA, IntervalSet setB);

/**
 * @brief Computes the negation of a set within a given domain.
 * This is (domain AND (NOT setA)).
 */
IntervalSet negateSet(IntervalSetHolder &holder, IntervalSet setA, Interval domain);


void destroyHolder(IntervalSetHolder &holder);

// --- NEW SEGMENT ITERATOR FUNCTIONS ---

/**
 * @brief Creates and initializes a SegmentIterator.
 * This iterator walks through a 'domain' and reports the boolean state
 * of setA and setB for each sub-segment.
 *
 * @param setA The "left" interval set.
 * @param setB The "right" interval set.
 * @param domain The interval [start, end) to iterate over.
 * @return An initialized SegmentIterator.
 */
SegmentIterator createSegmentIterator(IntervalSet setA, IntervalSet setB, Interval domain);

/**
 * @brief Advances the iterator to the next segment.
 * @param it The iterator (passed by reference) to advance.
 * @return true if a valid segment was found, false if the iteration is finished.
 */
bool getNextSegment(SegmentIterator& it);

CheckAndClipResult checkAndClip(IntervalSetHolder &holder, IntervalSet set, int time);


// --- Helper Functions ---

/**
 * @brief Converts an IntervalSet (of transitions) back to a
 * std::vector<Interval> for inspection.
 */
std::vector<Interval> toVectorIntervals(const IntervalSet& set);

/**
 * @brief Converts an IntervalSet to a std::vector of its raw transitions.
 */
std::vector<Transition> toVectorTransitions(const IntervalSet& set);

// Helper for std::sort
bool compareTransitions(const Transition& a, const Transition& b);

IntervalSet createSetFromIntervals(
    IntervalSetHolder& holder, 
    const std::vector<Interval>& intervals);

/**
 * @brief Overload operator== for Interval so Catch2 can compare vectors of them.
 */
bool operator==(const Interval& a, const Interval& b);

/**
 * @brief Overload operator== for Transition for testing.
 */
bool operator==(const Transition& a, const Transition& b);

/**
 * @brief Overload operator<< for Interval for printing.
 */
std::ostream& operator<<(std::ostream& os, const Interval& iv);

/**
 * @brief Overload operator<< for Transition for printing.
 */
std::ostream& operator<<(std::ostream& os, const Transition& t);

/**
 * @brief Overload operator<< for SegmentIterator for printing.
 */
std::ostream& operator<<(std::ostream& os, const SegmentIterator& it);


} // namespace db_interval_set
