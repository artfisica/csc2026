#include "TrackReconstructor.hpp"

#include <iostream>

namespace tt_e1 {

TrackReconstructor::TrackReconstructor(double minPt) : m_minPt(minPt) {}

void TrackReconstructor::addHit(const Hit& hit) { m_hits.push_back(hit); }

std::vector<Track> TrackReconstructor::reconstruct() {
    std::vector<Track> tracks;

    Hit* hitBuffer = new Hit[m_hits.size()];

    // FIX 2: Heap Buffer Overflow (i < m_hits.size())
    for (size_t i = 0; i < m_hits.size(); ++i) {
        hitBuffer[i] = m_hits[i];
    }

    // Simulate "reconstruction"
    if (!m_hits.empty()) {
        Track t{};
        t.pt = 10.0;          // fake pT
        t.hits = m_hits;      // copy hits
        tracks.push_back(t);
    }

    // FIX 1: Memory Leak (delete[] hitBuffer)
    delete[] hitBuffer;
    

    return tracks;
}

// BUG 3: Use-After-Free
// Return a pointer to memory that has already been freed.
const Track* TrackReconstructor::getBestTrack() const {
    auto* best = new Track{};
    best->pt = 100.0;
    best->hits = m_hits;

    // delete best;   // freed here
    return best;   // ERROR: returning freed pointer
}

} // namespace tt_e1

