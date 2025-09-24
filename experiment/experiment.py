import matplotlib.pyplot as plt
import numpy as np
from numpy import genfromtxt
import math
import pandas as pd
import os

def join_all_results_in_one(dir, name):
    files = []
    for file in os.listdir(dir):
        if ".txt" in file:
            files.append(pd.read_csv(dir + file, header=0 ,names=["Frame", "Entities", "FPS", "Frame Time (s)", "Physics Time (s)",  "Cache References", "Cache Misses", "Cache miss rate (%)"]))
    
    
    df = pd.concat(files, axis=0).groupby(["Frame"]).mean()
    return df
    # concatinated_files[["Entities", "FPS"]].plot(x = "Entities", y = "FPS", logx=True)
    # print(concatinated_files.groupby(["FPS"])["Entities"].mean())
    # print(concatinated_files.head())
    
def save_to_csv_reduced(dir, name):
    
    files = []
    for file in os.listdir(dir):
        if ".txt" in file:
            a = pd.read_csv(dir + file, header=0 ,names=["Frame", "Entities", "FPS", "Frame Time (s)", "Physics Time (s)", "Cache References", "Cache Misses", "Cache miss rate (%)"])
            #print(a)
            #print(name)
            files.append(a)
    

    
    df = pd.concat(files, axis=0).groupby(["Frame"]).mean()
    num = 5 if "baseline-no-physics" == name else 2
    df = df.iloc[::num]
    df.to_csv(dir + name + ".csv", encoding='utf-8', index=False )

dir_paths = [
    "./results/record-list/",
    "./results/collision-entity/",
    "./results/collision-relationship/",
    "./results/spatial-hash-per-cell/",
    "./results/spatial-hash-per-entity/",
]

dir_names = {
    "./results/record-list/": "record-list",
    "./results/collision-entity/": "collision-entity",
    "./results/collision-relationship/": "collision-relationship",
    "./results/spatial-hash-per-cell/" : "spatial-hash-per-cell",
    "./results/spatial-hash-per-entity/" : "spatial-hash-per-entity",
}

print (os.listdir("./results"))

ax_line = None
ax_line2 = None
ax_line3 = None
for dir in dir_paths:
    
    print(dir)
    print(dir_names[dir])
    save_to_csv_reduced(dir, dir_names[dir])
    df = join_all_results_in_one(dir, dir_names[dir])[["Entities", "FPS"]]
    df2 = join_all_results_in_one(dir, dir_names[dir])[["Entities", "Frame Time (s)"]]
    df3 = join_all_results_in_one(dir, dir_names[dir])[["Entities", "Physics Time (s)"]]

    df2["Frame Time (s)"] = df2["Frame Time (s)"] * 1000
    df3["Physics Time (s)"] = df3["Physics Time (s)"] * 1000
    if ax_line is None:
        ax_line = df.plot(figsize=(10, 5), ylim=(30,300), xlim=(100, 8000),  ylabel="FPS", x = "Entities", y = "FPS", logx=True, label=dir_names[dir], stacked=False)
    else:
        df.plot(ax=ax_line, x = "Entities",  ylim=(30,300), xlim=(100, 8000), y = "FPS", label=dir_names[dir], logx=True, stacked=False)
    
    if ax_line2 is None:
        ax_line2 = df2.plot(figsize=(10, 5), xlim=(100, 8000),  ylabel="Frame Time (ms)", x = "Entities", y = "Frame Time (s)", logx=True, label=dir_names[dir], stacked=False)
    else:
        df2.plot(ax=ax_line2, x = "Entities",  xlim=(100, 8000), y = "Frame Time (s)", label=dir_names[dir], logx=True, stacked=False)

    if ax_line3 is None:
        ax_line3 = df3.plot(figsize=(10, 5), xlim=(100, 8000),  ylabel="Physics Time (ms)", x = "Entities", y = "Physics Time (s)", logx=True, label=dir_names[dir], stacked=False)
    else:
        df3.plot(ax=ax_line3, x = "Entities",  xlim=(100, 8000), y = "Physics Time (s)", label=dir_names[dir], logx=True, stacked=False)
        
        
        
fps_values = [60, 100, 120, 240]
frame_time_values = [1.0 / 60.0, 1.0 / 100.0 , 1.0 / 120.0, 1.0 / 240.0]
fps_labels = ["60 FPS", "100 FPS", "120 FPS", "240 FPS"]

fpsmeans = {fps: [] for fps in frame_time_values}

cachemeans = {dir: []}
cache_references_sum = {dir: []}


frame_time_at_budget_16ms = {dir: []}
frame_time_at_budget_8ms = {dir: []}
frame_time_at_budget_4ms = {dir: []}
frame_time_at_budget_2ms = {dir: []}
frame_time_at_budget_1ms = {dir: []}


for dir in dir_paths:
    df = join_all_results_in_one(dir, dir_names[dir])[["Entities", "Frame Time (s)", "Physics Time (s)", "Cache References", "Cache miss rate (%)"]].sort_values(by="Frame Time (s)")
    for i, frame_time in enumerate(frame_time_values):
        fpsmeans[frame_time].append(np.interp(frame_time, df['Frame Time (s)'], df["Entities"]))
    cachemeans[dir_names[dir]] = df["Cache miss rate (%)"].mean()
    cache_references_sum[dir_names[dir]] = df["Cache References"].mean()
    frame_time_at_budget_16ms[dir_names[dir]] = np.interp(1/60, df['Physics Time (s)'], df["Entities"]) # at 100% of 60 fps
    frame_time_at_budget_8ms[dir_names[dir]] = np.interp(1/60/2, df['Physics Time (s)'], df["Entities"]) # at 50% of 60 fps
    frame_time_at_budget_4ms[dir_names[dir]] = np.interp(1/120/2, df['Physics Time (s)'], df["Entities"]) # at 25% of 60 fps
    frame_time_at_budget_2ms[dir_names[dir]] = np.interp(1/240/2, df['Physics Time (s)'], df["Entities"]) # at 25% of 60 fps
    frame_time_at_budget_1ms[dir_names[dir]] = np.interp(1/240/4, df['Physics Time (s)'], df["Entities"]) # at 25% of 60 fps


width = 1
x = np.arange(len(dir_names))


for i, fps in enumerate(frame_time_values):
    fig, ax = plt.subplots(figsize=(8, 6))
    mult = 0
    for j, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[j], fpsmeans[fps][j], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%.0f")
    
    ax.set_title(f'Entity count at {fps_labels[i]}')
    ax.set_ylabel('Entities')
    #ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
    ax.get_xaxis().set_visible(False)
    ax.set_yscale('log')
    ax.grid(axis='y', linestyle='--', alpha=0.7)
    ax.legend(loc='upper left')

fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], cachemeans[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.3f")
ax.set_title(f'Average Cache Miss')
ax.set_ylabel('Miss %')
ax.set_ylim(ymin=0, ymax=1)
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')


fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], cache_references_sum[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.3f")
ax.set_title(f'References')
ax.set_ylabel('References')
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')


fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], frame_time_at_budget_16ms[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.0f")
ax.set_title('Nb of Entities at 16ms physics time')
ax.set_ylabel('Entities')
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')

fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], frame_time_at_budget_8ms[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.0f")
ax.set_title('Nb of Entities at 8ms physics time')
ax.set_ylabel('Entities')
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')

fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], frame_time_at_budget_4ms[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.0f")
ax.set_title('Nb of Entities at 4ms physics time')
ax.set_ylabel('Entities')
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')

fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], frame_time_at_budget_2ms[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.0f")
ax.set_title('Nb of Entities at 2ms physics time')
ax.set_ylabel('Entities')
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')

fig, ax = plt.subplots(figsize=(8, 6))
mult = 0
for i, (att, measurement) in enumerate(dir_names.items()):
        rects = ax.bar(x[i], frame_time_at_budget_1ms[measurement], width, label=measurement)
        ax.bar_label(rects, padding=3, fmt="%0.0f")
ax.set_title('Nb of Entities at 1ms physics time')
ax.set_ylabel('Entities')
#ax.set_xticks(x, dir_names.values(), rotation=45, ha='right')
ax.get_xaxis().set_visible(False)
ax.grid(axis='y', linestyle='--', alpha=0.7)
ax.legend(loc='upper left')

plt.tight_layout()
plt.show()

