"""
ISS Praktikum 2 – Vorbereitung Aufgabe 4: Plots aus issp_zettel2_plot_mich_3dim.csv
"""
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

CSV_3DIM = Path(r"c:\Users\danie\Downloads\issp_zettel2_plot_mich_3dim.csv")
OUT_DIR = Path(__file__).resolve().parent / "output"
OUT_DIR.mkdir(exist_ok=True)

df = pd.read_csv(CSV_3DIM, sep="\t", header=None, names=["dim1", "dim2", "dim3"])
x = range(len(df))

# 1) Einzelplot erste Dimension
fig, ax = plt.subplots(figsize=(8, 4))
ax.plot(x, df["dim1"], marker="o", color="tab:blue")
ax.set_title("Dimension 1")
ax.set_xlabel("Sample")
ax.set_ylabel("Wert")
ax.grid(True, alpha=0.3)
fig.tight_layout()
fig.savefig(OUT_DIR / "plot_dim1.png", dpi=150)
fig.savefig(OUT_DIR / "plot_dim1.pdf")
plt.close(fig)

# 2) Gemeinsamer Plot aller drei Dimensionen
fig, ax = plt.subplots(figsize=(8, 4))
ax.plot(x, df["dim1"], marker="o", label="Dimension 1")
ax.plot(x, df["dim2"], marker="s", label="Dimension 2")
ax.plot(x, df["dim3"], marker="^", label="Dimension 3")
ax.set_title("Alle drei Dimensionen")
ax.set_xlabel("Sample")
ax.set_ylabel("Wert")
ax.legend()
ax.grid(True, alpha=0.3)
fig.tight_layout()
fig.savefig(OUT_DIR / "plot_all_dims.png", dpi=150)
fig.savefig(OUT_DIR / "plot_all_dims.pdf")
plt.close(fig)

# 3) Drei Subplots untereinander
fig, axes = plt.subplots(3, 1, figsize=(8, 9), sharex=True)
labels = ["Dimension 1", "Dimension 2", "Dimension 3"]
cols = ["dim1", "dim2", "dim3"]
colors = ["tab:blue", "tab:orange", "tab:green"]
for ax, col, label, color in zip(axes, cols, labels, colors):
    ax.plot(x, df[col], marker="o", color=color)
    ax.set_ylabel("Wert")
    ax.set_title(label)
    ax.grid(True, alpha=0.3)
axes[-1].set_xlabel("Sample")
fig.suptitle("Sub-Plots: je eine Dimension", y=1.01)
fig.tight_layout()
fig.savefig(OUT_DIR / "plot_subplots_3dim.png", dpi=150)
fig.savefig(OUT_DIR / "plot_subplots_3dim.pdf")
plt.close(fig)

print(f"Plots gespeichert in: {OUT_DIR}")
