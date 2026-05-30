"""ISS Praktikum 2 – Vorbereitung Aufgabe 4 (Plots für Laborbericht)."""
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

CSV = Path(r"c:\Users\danie\Downloads\issp_zettel2_plot_mich_3dim.csv")
OUT = Path(__file__).resolve().parent
OUT.mkdir(exist_ok=True)

df = pd.read_csv(CSV, sep="\t", header=None, names=["dim1", "dim2", "dim3"])
x = list(range(1, len(df) + 1))

plt.rcParams.update({"font.size": 11, "figure.dpi": 150})

# 1) Einzelplot Dimension 1
fig, ax = plt.subplots(figsize=(7, 3.5))
ax.plot(x, df["dim1"], "o-", color="#1f77b4", linewidth=2, markersize=8)
ax.set_xlabel("Messpunkt")
ax.set_ylabel("Wert")
ax.set_title("Dimension 1 – issp_zettel2_plot_mich_3dim.csv")
ax.set_xticks(x)
ax.grid(True, alpha=0.35)
fig.tight_layout()
for ext in ("png", "pdf"):
    fig.savefig(OUT / f"01_plot_dimension1.{ext}", bbox_inches="tight")
plt.close(fig)

# 2) Gemeinsamer Plot aller drei Dimensionen
fig, ax = plt.subplots(figsize=(7, 4))
ax.plot(x, df["dim1"], "o-", label="Dimension 1", linewidth=2, markersize=7)
ax.plot(x, df["dim2"], "s-", label="Dimension 2", linewidth=2, markersize=7)
ax.plot(x, df["dim3"], "^-", label="Dimension 3", linewidth=2, markersize=7)
ax.set_xlabel("Messpunkt")
ax.set_ylabel("Wert")
ax.set_title("Alle drei Dimensionen – gemeinsames Diagramm")
ax.set_xticks(x)
ax.legend()
ax.grid(True, alpha=0.35)
fig.tight_layout()
for ext in ("png", "pdf"):
    fig.savefig(OUT / f"02_plot_alle_dimensionen.{ext}", bbox_inches="tight")
plt.close(fig)

# 3) Drei Subplots untereinander
fig, axes = plt.subplots(3, 1, figsize=(7, 8), sharex=True)
specs = [
    ("dim1", "Dimension 1", "#1f77b4", "o"),
    ("dim2", "Dimension 2", "#ff7f0e", "s"),
    ("dim3", "Dimension 3", "#2ca02c", "^"),
]
for ax, (col, title, color, marker) in zip(axes, specs):
    ax.plot(x, df[col], f"{marker}-", color=color, linewidth=2, markersize=7)
    ax.set_ylabel("Wert")
    ax.set_title(title)
    ax.set_xticks(x)
    ax.grid(True, alpha=0.35)
axes[-1].set_xlabel("Messpunkt")
fig.suptitle("Sub-Plots: je eine Dimension", fontsize=12, y=1.01)
fig.tight_layout()
for ext in ("png", "pdf"):
    fig.savefig(OUT / f"03_plot_subplots_untereinander.{ext}", bbox_inches="tight")
plt.close(fig)

print("Fertig. Dateien in:", OUT)
