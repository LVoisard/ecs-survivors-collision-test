import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import os

# ---------------------------------------------------------------------------
# Config
# ---------------------------------------------------------------------------

DIR_NAMES = {
    "./results/record-list/":                          "record-list",
    "./results/collision-entity/":                     "collision-entity",
    "./results/collision-relationship/":               "collision-relationship",
    "./results/collision-relationship-dontfragment/":  "collision-relationship-dontfragment",
    "./results/spatial-hash-per-cell/":                "spatial-hash-per-cell",
    "./results/spatial-hash-per-entity/":              "spatial-hash-per-entity",
}

COLUMNS = ["Frame", "Entities", "FPS", "Frame Time (s)", "Physics Time (s)",
           "Cache References", "Cache Misses", "Cache miss rate (%)"]

FPS_TARGETS     = [60, 100, 120, 240]
PHYSICS_BUDGETS = {"16ms": 1/60, "8ms": 1/120, "4ms": 1/240, "2ms": 1/480, "1ms": 1/960}

OUTPUT_DIR = "./figures/"
os.makedirs(OUTPUT_DIR, exist_ok=True)


def savefig(filename: str) -> None:
    plt.gcf().savefig(os.path.join(OUTPUT_DIR, filename), dpi=150, bbox_inches="tight")


# ---------------------------------------------------------------------------
# I/O helpers
# ---------------------------------------------------------------------------

def load_runs(directory: str) -> list[pd.DataFrame]:
    """Return one DataFrame per .txt file in *directory*."""
    runs = []
    for f in os.listdir(directory):
        if f.endswith(".txt"):
            runs.append(pd.read_csv(os.path.join(directory, f), header=0, names=COLUMNS))
    if not runs:
        raise FileNotFoundError(f"No .txt files found in {directory}")
    return runs


def load_dir(directory: str) -> tuple[pd.DataFrame, pd.DataFrame]:
    """Return (mean, std) across all runs, grouped by Frame."""
    combined = pd.concat(load_runs(directory), axis=0)
    return combined.groupby("Frame").mean(), combined.groupby("Frame").std()


def save_reduced_csv(directory: str, name: str, df_mean: pd.DataFrame) -> None:
    step = 5 if name == "baseline-no-physics" else 2
    df_mean.iloc[::step].to_csv(
        os.path.join(directory, f"{name}.csv"), encoding="utf-8", index=False
    )


def interp_per_run(directory: str, x_col: str, budget: float) -> tuple[float, float, float]:
    """
    For each run file, sort by *x_col* and interpolate entity count at *budget*.
    Returns (mean, std, median) across runs — the statistically correct way to get
    these quantities on an interpolated value.
    """
    per_run = []
    for df in load_runs(directory):
        df_sorted = df[["Entities", x_col]].sort_values(x_col)
        per_run.append(float(np.interp(budget, df_sorted[x_col], df_sorted["Entities"])))
    return float(np.mean(per_run)), float(np.std(per_run, ddof=1)), float(np.median(per_run))


def cache_stats_per_run(directory: str) -> dict[str, tuple[float, float, float]]:
    """Return {metric: (mean, std, median)} for cache columns, computed across runs."""
    cache_cols = ["Cache miss rate (%)", "Cache References"]
    run_means = {col: [] for col in cache_cols}
    for df in load_runs(directory):
        for col in cache_cols:
            run_means[col].append(df[col].mean())
    return {
        col: (float(np.mean(vals)), float(np.std(vals, ddof=1)), float(np.median(vals)))
        for col, vals in run_means.items()
    }


# ---------------------------------------------------------------------------
# Data loading  (mean/std DataFrames for line plots)
# ---------------------------------------------------------------------------

data: dict[str, dict] = {}

for directory, name in DIR_NAMES.items():
    mean, std = load_dir(directory)
    save_reduced_csv(directory, name, mean)
    data[name] = {"mean": mean, "std": std, "dir": directory}


# ---------------------------------------------------------------------------
# Line plots: FPS / Frame Time / Physics Time  (with std-dev bands)
# ---------------------------------------------------------------------------

LINE_METRICS = [
    ("FPS",              "FPS",              "fps",         dict(ylim=(30, 300))),
    ("Frame Time (s)",   "Frame Time (ms)",  "frame_time",  {}),
    ("Physics Time (s)", "Physics Time (ms)","physics_time",{}),
]

for col, ylabel, slug, extra_kwargs in LINE_METRICS:
    fig, ax = plt.subplots(figsize=(10, 5))

    for name, dfs in data.items():
        entities  = dfs["mean"]["Entities"]
        mean_vals = dfs["mean"][col].copy()
        std_vals  = dfs["std"][col].copy()

        if col != "FPS":
            mean_vals = mean_vals * 1000
            std_vals  = std_vals  * 1000

        ax.plot(entities, mean_vals, label=name)
        ax.fill_between(entities, mean_vals - std_vals, mean_vals + std_vals, alpha=0.15)

    ax.set_xscale("log")
    ax.set_xlim(100, 8000)
    ax.set_xlabel("Entities")
    ax.set_ylabel(ylabel)
    if "ylim" in extra_kwargs:
        ax.set_ylim(*extra_kwargs["ylim"])
    ax.set_title(ylabel)
    ax.legend()
    ax.grid(True, which="both", linestyle="--", alpha=0.4)
    fig.tight_layout()
    savefig(f"line_{slug}.png")


# ---------------------------------------------------------------------------
# Bar-chart helper
# ---------------------------------------------------------------------------

def bar_chart(title: str, ylabel: str, values: dict[str, float],
              errors: dict[str, float] | None = None,
              fmt: str = "%.0f", yscale: str = "linear",
              ylim: tuple | None = None,
              filename: str | None = None) -> None:
    names = list(values.keys())
    vals  = list(values.values())
    errs  = [max(0.0, abs(errors[n])) for n in names] if errors else None

    fig, ax = plt.subplots(figsize=(8, 6))
    x    = np.arange(len(names))
    bars = ax.bar(x, vals, width=0.7, yerr=errs, capsize=4,
                  error_kw=dict(elinewidth=1.2, alpha=0.8))

    for i, (bar, val) in enumerate(zip(bars, vals)):
        ax.text(bar.get_x() + bar.get_width() / 2,
                bar.get_height() + (errs[i] if errs else 0),
                fmt % val, ha="center", va="bottom", fontsize=8)

    ax.set_title(title)
    ax.set_ylabel(ylabel)
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=35, ha="right", fontsize=8)
    if yscale != "linear":
        ax.set_yscale(yscale)
    if ylim:
        ax.set_ylim(*ylim)
    ax.grid(axis="y", linestyle="--", alpha=0.5)
    #ax.legend(bars, names, fontsize=7, loc="upper left")
    fig.tight_layout()

    if filename:
        savefig(filename)


# ---------------------------------------------------------------------------
# Bar charts + table: entity count at each FPS target
# ---------------------------------------------------------------------------

table_rows: dict[str, dict[str, float]] = {name: {} for name in DIR_NAMES.values()}

for fps in FPS_TARGETS:
    vals, errs = {}, {}
    for name, dfs in data.items():
        mean, std, median = interp_per_run(dfs["dir"], "Frame Time (s)", 1.0 / fps)
        vals[name] = mean
        errs[name] = std
        table_rows[name][f"Entities @ {fps} FPS (mean)"]   = mean
        table_rows[name][f"Entities @ {fps} FPS (std)"]    = std
        table_rows[name][f"Entities @ {fps} FPS (median)"] = median
    bar_chart(f"Entity count at {fps} FPS", "Entities", vals, errs,
              fmt="%.0f", yscale="log", filename=f"bar_fps_{fps}.png")


# ---------------------------------------------------------------------------
# Bar charts + table: entity count at each physics time budget
# ---------------------------------------------------------------------------

for label, budget in PHYSICS_BUDGETS.items():
    vals, errs = {}, {}
    for name, dfs in data.items():
        mean, std, median = interp_per_run(dfs["dir"], "Physics Time (s)", budget)
        vals[name] = mean
        errs[name] = std
        table_rows[name][f"Entities @ {label} physics (mean)"]   = mean
        table_rows[name][f"Entities @ {label} physics (std)"]    = std
        table_rows[name][f"Entities @ {label} physics (median)"] = median
    bar_chart(f"Entities at {label} physics budget", "Entities", vals, errs,
              fmt="%.0f", filename=f"bar_physics_{label}.png")


# ---------------------------------------------------------------------------
# Bar charts + table: cache statistics
# ---------------------------------------------------------------------------

for name, dfs in data.items():
    stats = cache_stats_per_run(dfs["dir"])
    for col, (mean, std, median) in stats.items():
        table_rows[name][f"{col} (mean)"]   = mean
        table_rows[name][f"{col} (std)"]    = std
        table_rows[name][f"{col} (median)"] = median

cache_miss_vals = {n: table_rows[n]["Cache miss rate (%) (mean)"] for n in DIR_NAMES.values()}
cache_miss_errs = {n: table_rows[n]["Cache miss rate (%) (std)"]  for n in DIR_NAMES.values()}
cache_ref_vals  = {n: table_rows[n]["Cache References (mean)"]    for n in DIR_NAMES.values()}
cache_ref_errs  = {n: table_rows[n]["Cache References (std)"]     for n in DIR_NAMES.values()}

bar_chart("Average Cache Miss Rate",  "Miss %",     cache_miss_vals, cache_miss_errs,
          fmt="%.3f", ylim=(0, 1), filename="bar_cache_miss.png")
bar_chart("Average Cache References", "References", cache_ref_vals,  cache_ref_errs,
          fmt="%.0f",                 filename="bar_cache_refs.png")


# ---------------------------------------------------------------------------
# Export summary table
# ---------------------------------------------------------------------------

summary_df = pd.DataFrame.from_dict(table_rows, orient="index")
summary_df.index.name = "Approach"
summary_path = os.path.join(OUTPUT_DIR, "summary_stats.csv")
summary_df.to_csv(summary_path)
print(f"Summary table saved to {summary_path}")
print(summary_df.to_string())

plt.show()