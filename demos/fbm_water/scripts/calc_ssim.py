import cv2
import os
import csv
from skimage.metrics import structural_similarity as ssim

ref_path = "../output/A_oct6.png"
if not os.path.exists(ref_path):
    print("Reference image A_oct6.png not found. Please run the experiment first.")
    exit(1)

ref = cv2.imread(ref_path)
ref_gray = cv2.cvtColor(ref, cv2.COLOR_BGR2GRAY)

results = []
out_dir = "../output"
for fname in sorted(os.listdir(out_dir)):
    if not fname.endswith(".png") or fname == "A_oct6.png":
        continue
    path = os.path.join(out_dir, fname)
    img = cv2.imread(path)
    if img is None:
        continue
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    if gray.shape != ref_gray.shape:
        gray = cv2.resize(gray, (ref_gray.shape[1], ref_gray.shape[0]))
    score = ssim(ref_gray, gray, data_range=255)
    results.append((fname, score))
    print(f"{fname}: SSIM = {score:.4f}")

os.makedirs("../data", exist_ok=True)
with open("../data/ssim_results.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["filename", "ssim"])
    writer.writerows(results)

print("SSIM results saved to ../data/ssim_results.csv")