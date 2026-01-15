import os
import sys
import time
import ROOT
from ROOT import TMath

# Usage:
#   python3 gamma-gamma-analysis-v1.py <ROOT_URL> [max_events]
#
# Output:
#   plots/histogram_<basename>.png
#   plots/histogram_<basename>.root

if len(sys.argv) < 2:
    raise ValueError("No ROOT file provided. Pass the ROOT file URL as argv[1].")

root_file_url = sys.argv[1]
max_events = 0
if len(sys.argv) >= 3:
    try:
        max_events = int(sys.argv[2])
    except ValueError:
        max_events = 0

start = time.time()

f = ROOT.TFile.Open(root_file_url)
if not f or f.IsZombie():
    raise RuntimeError(f"Failed to open ROOT file: {root_file_url}")

tree = f.Get("mini")
if not tree:
    raise RuntimeError("Tree 'mini' not found in file.")

# Histogram: diphoton invariant mass
hist = ROOT.TH1F(
    "h_M_Hyy",
    "Diphoton invariant-mass ; Invariant Mass m_{yy} [GeV] ; events",
    30, 105, 160
)

Photon_1 = ROOT.TLorentzVector()
Photon_2 = ROOT.TLorentzVector()

n = 0
for event in tree:
    n += 1
    if max_events > 0 and n > max_events:
        break

    if n % 10000 == 0:
        print(f"Processed {n} events...")

    # Trigger condition
    if tree.trigP:
        goodphoton_index = [0] * 5
        goodphoton_n = 0
        photon_index = 0

        # Loop over photons
        for j in range(tree.photon_n):
            if (tree.photon_isTightID[j] and tree.photon_pt[j] > 20000 and
                (TMath.Abs(tree.photon_eta[j]) < 2.37) and
                (TMath.Abs(tree.photon_eta[j]) < 1.37 or TMath.Abs(tree.photon_eta[j]) > 1.52)):

                goodphoton_n += 1
                if photon_index < len(goodphoton_index):
                    goodphoton_index[photon_index] = j
                photon_index += 1

        # Exactly two good photons
        if goodphoton_n == 2:
            i1 = goodphoton_index[0]
            i2 = goodphoton_index[1]

            # Isolation
            if ((tree.photon_ptcone30[i1] / tree.photon_pt[i1] < 0.065) and
                (tree.photon_etcone20[i1] / tree.photon_pt[i1] < 0.065) and
                (tree.photon_ptcone30[i2] / tree.photon_pt[i2] < 0.065) and
                (tree.photon_etcone20[i2] / tree.photon_pt[i2] < 0.065)):

                Photon_1.SetPtEtaPhiE(tree.photon_pt[i1] / 1000., tree.photon_eta[i1],
                                      tree.photon_phi[i1], tree.photon_E[i1] / 1000.)
                Photon_2.SetPtEtaPhiE(tree.photon_pt[i2] / 1000., tree.photon_eta[i2],
                                      tree.photon_phi[i2], tree.photon_E[i2] / 1000.)

                Photon_12 = Photon_1 + Photon_2
                hist.Fill(Photon_12.M())

# Save outputs
os.makedirs("plots", exist_ok=True)

base = os.path.basename(root_file_url)
if base.endswith(".root"):
    base = base[:-5]

output_png = f"plots/histogram_{base}.png"
output_root = f"plots/histogram_{base}.root"

canvas = ROOT.TCanvas("Canvas", "cz", 800, 600)
hist.Draw("E")
canvas.SetLogy()
canvas.SaveAs(output_png)

root_output_file = ROOT.TFile(output_root, "RECREATE")
hist.Write()
root_output_file.Close()

end = time.time()
duration = end - start
print(f"Finished {base} in {int(duration // 60)} min {int(duration % 60)} s")
