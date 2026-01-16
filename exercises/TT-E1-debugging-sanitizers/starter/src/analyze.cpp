#include "TrackReconstructor.hpp"

#include <iostream>

int main() {
    std::cout << "TT-E1: Broken Reconstruction (student starter)" << std::endl;

    tt_e1::TrackReconstructor reco(/*minPt=*/1.0);

    // Create a small hit list
    for (int i = 0; i < 10; ++i) {
        tt_e1::Hit h{};
        h.x = 0.1 * i;
        h.y = 0.2 * i;
        h.z = 0.3 * i;
        h.energy = 1.0;
        reco.addHit(h);
    }

    // Trigger: heap-buffer-overflow + leak
    auto tracks = reco.reconstruct();
    std::cout << "Tracks reconstructed: " << tracks.size() << std::endl;

    // Trigger: use-after-free (only visible after you fix the overflow)
    const tt_e1::Track* best = reco.getBestTrack();
    std::cout << "Best track pT: " << best->pt << std::endl;

    // Clean up the allocated track
    delete best;

    return 0;
}

