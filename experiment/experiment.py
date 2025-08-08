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
            files.append(pd.read_csv(dir + file, header=0 ,names=["Frame", "Entities",  "Frame Time (us)", "FPS", "Cache References", "Cache Misses", "Cache miss rate (%)"]))
    
    
    df = pd.concat(files, axis=0).groupby(["Frame"]).mean()
    return df
    # concatinated_files[["Entities", "FPS"]].plot(x = "Entities", y = "FPS", logx=True)
    # print(concatinated_files.groupby(["FPS"])["Entities"].mean())
    # print(concatinated_files.head())
    
def save_to_csv_reduced(dir, name):
    
    files = []
    for file in os.listdir(dir):
        if ".txt" in file:
            a = pd.read_csv(dir + file, header=0 ,names=["Frame", "Entities", "Frame Time (us)","FPS", "Cache References", "Cache Misses", "Cache miss rate (%)"])
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
    "./results/spatial-hash/",
    #"./results/spatial-hash-relationship/",
]

dir_names = {
    "./results/record-list/": "record-list",
    "./results/collision-entity/": "collision-entity",
    "./results/collision-relationship/": "collision-relationship",
    "./results/spatial-hash/" : "spatial-hash",
    #"./results/spatial-hash-relationship/" : "spatial-hash-relationship",
}

print (os.listdir("./results"))

ax_line = None
for dir in dir_paths:
    
    print(dir)
    print(dir_names[dir])
    save_to_csv_reduced(dir, dir_names[dir])
    df = join_all_results_in_one(dir, dir_names[dir])[["Entities", "FPS"]]
    df2 = join_all_results_in_one(dir, dir_names[dir])[["Entities", "Cache miss rate (%)"]]
    if ax_line is None:
        ax_line = df.plot(figsize=(10, 5), ylim=(30,300), xlim=(100, 8000),  ylabel="FPS", x = "Entities", y = "FPS", logx=True, label=dir_names[dir], stacked=False)
    else:
        df.plot(ax=ax_line, x = "Entities",  ylim=(30,300), xlim=(100, 8000), y = "FPS", label=dir_names[dir], logx=True, stacked=False)
        
        
        
fps_values = [60, 100, 120, 240]
frame_time_values = [1.0 / 60.0, 1.0 / 100.0 , 1.0 / 120.0, 1.0 / 240.0]
fps_labels = ["60 FPS", "100 FPS", "120 FPS", "240 FPS"]

fpsmeans = {fps: [] for fps in frame_time_values}

cachemeans = {dir: []}


for dir in dir_paths:
    df = join_all_results_in_one(dir, dir_names[dir])[["Entities", "Frame Time (us)", "Cache miss rate (%)"]].sort_values(by="Frame Time (us)")
    for i, fps in enumerate(frame_time_values):
        fpsmeans[fps].append(np.interp(fps, df['Frame Time (us)'], df["Entities"]))
    cachemeans[dir_names[dir]] = df["Cache miss rate (%)"].mean()



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
    ax.legend(loc='upper right')

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
ax.legend(loc='upper right')

plt.tight_layout()
plt.show()

