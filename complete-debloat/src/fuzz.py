#!/usr/bin/env python
# coding: utf-8


import numpy as np
from matplotlib import pyplot as plt
import sys
import subprocess
import os
from queue import PriorityQueue
import random
import math
from collections import deque
import time
import queue
from scipy.spatial import ConvexHull, convex_hull_plot_2d
import configparser
import argparse
import matplotlib
matplotlib.rcParams.update({'font.size': 16})


# Initialsing global data structures
good_inp_x = []
good_inps = []
bad_inps = []
good_inp_y = []
bad_inp_x = []
bad_inp_y = []
offset_x = []
offset_y = []
clusters = {'good':[], 'bad':[]}
cluster_data = {'good':{}, 'bad':{}}

good_inputs = set()
bad_inputs = set()
offsets = set()
q = queue.Queue()
random.seed(time.time())


# ---------------------Reading config------------------------------------

config = configparser.ConfigParser()
config.read('config.ini')

inp_dim = config.getint('Fuzz', 'inp_dim')

ranges = config.get('Fuzz', 'min_ranges')
min_ranges = [int(num) for num in ranges.split(',')]

ranges = config.get('Fuzz', 'max_ranges')
max_ranges = [int(num) for num in ranges.split(',')]

ranges = config.get('Fuzz', 'min_ranges_out')
min_ranges_out = [int(num) for num in ranges.split(',')]

ranges = config.get('Fuzz', 'max_ranges_out')
max_ranges_out = [int(num) for num in ranges.split(',')]

offset_dim = config.getint('Fuzz', 'offset_dim')
s_reps = config.getint('Fuzz', 's_reps')
f_reps = config.getint('Fuzz', 'f_reps')

ranges = config.get('Fuzz', 's_dist')
s_dist = [int(num) for num in ranges.split(',')]

ranges = config.get('Fuzz', 'f_dist')
f_dist = [int(num) for num in ranges.split(',')]

stop_iter = config.getint('Fuzz', 'stop_iter')
max_iter = config.getint('Fuzz', 'max_iter')
last_new_offset = config.getint('Fuzz', 'last_new_offset')
bfs_explore_dist = config.getint('Fuzz', 'bfs_explore_dist')
cluster_dist = config.getint('Fuzz', 'cluster_dist')
epsilon = config.getint('Fuzz', 'epsilon')
decay = config.getfloat('Fuzz', 'decay')
decay_iter = config.getint('Fuzz', 'decay_iter')
restart_iter = config.getint('Fuzz', 'restart_iter')
dot_size = config.getint('Fuzz', 'dot_size')
program_name = config.get('Fuzz', 'program')
directory = config.get('Fuzz', 'directory')

parser = argparse.ArgumentParser(description='Command line configuration parser')
parser.add_argument('--inp-dim', type = int , help ='input dimension')
parser.add_argument('--min-ranges', type = int, nargs='+', help ='minimum input param ranges')
parser.add_argument('--max-ranges', type = int, nargs='+', help ='maximum input param ranges')
parser.add_argument('--min-ranges-out', type = int, nargs='+', help ='minimum output param ranges')
parser.add_argument('--max-ranges-out', type = int, nargs='+', help ='maximum output param ranges')
parser.add_argument('--offset-dim', type = int, help ='offset dimension')
parser.add_argument('--s-reps', type = int, help ='mutation reps for success')
parser.add_argument('--f-reps', type = int, help ='mutation reps for failure')
parser.add_argument('--max-iter', type = int, help ='maximum iterations for fuzzing')
parser.add_argument('--cluster-dist', type = int, help ='cluster distance for fuzzing')
parser.add_argument('--decay', type = float, help ='decay magnitude')
parser.add_argument('--decay-iter', type = int, help ='decay rate')
parser.add_argument('--restart-iter', type = int, help ='restart rate')
parser.add_argument('--s-dist', type = int, nargs='+', help ='samplind distance for success')
parser.add_argument('--f-dist', type = int, nargs='+', help ='sampling distance for failure')
parser.add_argument('--program', type = str, help ='program to run')
parser.add_argument('--directory', type = str, help ='directory of the program')
parser.add_argument('--plot', type = int, help ='whether to plot the graphs')
parser.add_argument('--normal', type = int, help ='set to 1 for only explore and exploit schedule')


args = parser.parse_args()

if args.inp_dim:
    inp_dim = args.inp_dim
if args.min_ranges:
    min_ranges = args.min_ranges
if args.max_ranges:
    max_ranges = args.max_ranges
if args.min_ranges_out:
    min_ranges_out = args.min_ranges_out
if args.max_ranges_out:
    max_ranges_out = args.max_ranges_out
if args.offset_dim:
    offset_dim = args.offset_dim
if args.s_reps:
    s_reps = args.s_reps
if args.f_reps:
    f_reps = args.f_reps
if args.max_iter:
    max_iter = args.max_iter
if args.cluster_dist:
    cluster_dist = args.cluster_dist
if args.decay:
    decay = args.decay
if args.decay_iter:
    decay_iter = args.decay_iter
if args.restart_iter:
    restart_iter = args.restart_iter
if args.s_dist:
    s_dist = args.s_dist
if args.f_dist:
    f_dist = args.f_dist
if args.program:
    program_name = args.program
if args.directory:
    directory = args.directory
plot = args.plot
normal = args.normal


if normal:
    decay = 1
    

# initialsing filenames to write statistics

data_fuzz = f'./data/{program_name}_fuzz.npy'
data_truth = f'./data/{program_name}_truth.npy'
data_hull = f'./data/{program_name}_hull.npy'
logger = f'./logs/{program_name}.log'
if normal:
    input_map = f'./input_map/{program_name}_normal.png'
else:
    input_map = f'./input_map/{program_name}.png'
output_map = f'./output_map/{program_name}_fuzz.png'
program = f'./{directory}/{program_name}'
execution_time_file = f'./output/{program_name}/execution_time'
space_evaluated_file = f'./output/{program_name}/parameter_space_evaluated'

execution_time = 0


# ----------------------------Done reading config------------------------------

# put max and min numbers in the queue
if inp_dim == 2:
    q.put( (0, (max_ranges[0], max_ranges[1])) )
    q.put( (0, (min_ranges[0], min_ranges[1])) )
    q.put( (0, (min_ranges[0], max_ranges[1])) )
    q.put( (0, (max_ranges[0], min_ranges[1])) )

if inp_dim == 3:
    q.put( (0, (max_ranges[0], max_ranges[1], max_ranges[2])) )
    q.put( (0, (min_ranges[0], min_ranges[1], min_ranges[2])) )
    q.put( (0, (min_ranges[0], min_ranges[1], max_ranges[2])) )
    q.put( (0, (min_ranges[0], max_ranges[1], min_ranges[2])) )
    q.put( (0, (min_ranges[0], max_ranges[1], max_ranges[2])) )
    q.put( (0, (max_ranges[0], min_ranges[1], min_ranges[2])) )
    q.put( (0, (max_ranges[0], min_ranges[1], max_ranges[2])) )
    q.put( (0, (max_ranges[0], max_ranges[1], min_ranges[2])) )


# start with 10 additional random seeds
for i in range(10):
    q.put((0, tuple([random.randint(min_ranges[i], max_ranges[i]) for i in range(inp_dim)])))

# setup plots
if inp_dim == 2:
    inp_fig, inp_ax = plt.subplots(tight_layout=True)

    # inp_ax.set_title("Input coverage")
    inp_ax.set_xlim(min_ranges[0], max_ranges[0])
    inp_ax.set_ylim(min_ranges[1], max_ranges[1])
    inp_ax.set_xlabel("X", fontweight='bold')
    inp_ax.set_ylabel("Y", fontweight='bold')

if inp_dim == 3:
    inp_fig = plt.figure(tight_layout=True)
    inp_ax = inp_fig.add_subplot(projection='3d')

    # inp_ax.set_title("Input coverage")
    inp_ax.set_xlim(min_ranges[0], max_ranges[0])
    inp_ax.set_ylim(min_ranges[1], max_ranges[1])
    inp_ax.set_zlim(min_ranges[2], max_ranges[2])
    inp_ax.set_xlabel("X", fontweight='bold')
    inp_ax.set_ylabel("Y", fontweight='bold')
    inp_ax.set_zlabel("Z", fontweight='bold')


if offset_dim == 2:
    out_fig, out_ax = plt.subplots(tight_layout=True)
    # out_ax.set_title("Output coverage")
    out_ax.set_xlim(min_ranges_out[0], max_ranges_out[0])
    out_ax.set_ylim(min_ranges_out[1], max_ranges_out[1])
    out_ax.set_xlabel("offsetX", fontweight='bold')
    out_ax.set_ylabel("offsetY", fontweight='bold') 
if offset_dim == 3:
    out_fig = plt.figure(tight_layout=True)
    out_ax = out_fig.add_subplot(projection='3d')
    # out_ax.set_title("Output coverage")
    out_ax.set_xlim(min_ranges_out[0], max_ranges_out[0])
    out_ax.set_ylim(min_ranges_out[1], max_ranges_out[1])
    out_ax.set_zlim(min_ranges_out[2], max_ranges_out[2])
    out_ax.set_xlabel("offsetX", fontweight='bold')
    out_ax.set_ylabel("offsetY", fontweight='bold')
    out_ax.set_zlabel("offsetZ", fontweight='bold')



itr = 0
def stopping_criteria():
    if last_new_offset >= stop_iter or itr >= max_iter:
        print(f'last new offset: {last_new_offset}; itr: {itr}')
        return True
    return False



def evaluate_seed(seed):
    '''
    evaluate X with parameter valuation: seed. Study the output produced.
    If non empty output, that means finite data was read and seed is valid.
    else no data was read which implies invalid seed.
    '''
    global last_new_offset
    global execution_time
    
    cmd = [f'{int(i)}' for i in seed] # TODO: remove the int cast
    cmd.insert(0, program)

    start = time.time()
    output = subprocess.run(cmd, stdout=subprocess.PIPE)
    execution_time += time.time() - start

    output = output.stdout.decode('utf-8')

    if output == '':
        # print("empty output")
        return False
    
    outputs = output.split('\n')[:-1]

    for i in outputs:
        offset = i.split(',')
        if offset=='':
            continue
        offset = tuple([int(off) for off in offset if off != '' ])
        
        if offset not in offsets:
            last_new_offset = 0
            offsets.add(offset)
            # if plot:
            #     out_ax.scatter(*np.array(offset), c='y', s = dot_size)
    # out_fig.savefig(output_map)
            

    return True


# In[ ]:


def get_neighbours_func(node):
    '''
    get the 4 adjacent neighbours of a point in the eucledian space.
    Unused as of now.
    '''
    neighbors = []
    for dim in range(len(node)):
        for sign in [-1, 1]:
            neighbor = list(node)
            neighbor[dim] += sign
            if neighbor[dim] < 0:
                continue
            neighbors.append(tuple(neighbor))
    return neighbors

def calculate_relevance(seed):
    '''
    assgin a relevance score to the seed based on some criteria.
    Could be used to assign priority to some seeds while mutation.
    Unusued as of now.
    '''
    visited = {seed}
    count = 0
    return 0

    # bfs 
    pts = deque([(seed, 0)])
    while pts:
        node, distance = pts.popleft()
        
        if node in good_inputs:
            count+=1

        if distance >= bfs_explore_dist: 
            break
        
        neighbors = get_neighbours_func(node)
        for neighbor in neighbors:
            if neighbor not in visited:
                visited.add(neighbor)
                pts.append((neighbor, distance+1))
    return count



def manhattan_distance(a, b):
    dists = [abs(a[i]-b[i]) for i in range(len(a))]
    return np.sum(np.array(dists))


def check_range(seed):
    '''
    check that seed lies within the ranges given for all parameters.
    '''
    return not any([seed[i]<min_ranges[i] or seed[i]>max_ranges[i] for i in range(len(seed))])

def normal_mutate(seed, dist, reps, type):
    '''
    Normal mutate stratgy for a seed.
    New seeds are selected from a frame surrounding the current seed.
    dimensions of the frame is fixed by the fuzz configuration.
    '''
    for i in range(reps):

        new_seed = tuple([random.uniform(seed[i]-dist[1], seed[i]+dist[1]) for i in range(len(seed))])

        discard = True
        for i in range(len(seed)):
            if not (new_seed[i]<=seed[i]+dist[0] and new_seed[i]>=seed[i]-dist[0]):
                discard = False
                break
        if discard:
            continue
        
        if new_seed not in good_inputs and new_seed not in bad_inputs and check_range(new_seed):
            score = calculate_relevance(new_seed)
            q.put((score, new_seed))
    return

def greedy_mutate(seed, dist, reps, type):
    '''
    greedy mutate strategy for a seed.
    Similar to normal mutate but the dimensions of the frame are now decided
    based on the proximity of the seed from a subset boundary. For this, distance
    from the seed to the nearest cluster center of the opposite type is used.
    '''
    if type=='good' or type=='bad':
        cluster = clusters['bad'] if type== 'good' else clusters['good']
        dists = list(map(lambda x: manhattan_distance(x, seed), cluster))
        if len(dists) != 0:
            min_dist = min(dists)

            # scale min_dist to a scale of 10
            min_dist *= min(10, 1/cluster_dist)
            
            # scale dist acc to this
            dist = [dist[i]*min_dist for i in range(len(dist))]
        return normal_mutate(seed, dist, reps, type)
        return

    # calculate the nearest cluster of the opposite type
    cluster = clusters['bad'] if type== 'good' else clusters['good']
    dists = list(map(lambda x: manhattan_distance(x, seed), cluster))
    if len(dists) == 0:
        return normal_mutate(seed, dist, reps, type)
    min_clust = cluster[dists.index(min(dists))]

    for i in range(reps):
        new_seed = tuple([random.uniform(min(min_clust[i], seed[i]), max([min_clust[i], seed[i]])) for i in range(len(seed))])

        if new_seed not in good_inputs and new_seed not in bad_inputs and check_range(new_seed):
            score = calculate_relevance(new_seed)
            q.put((score, new_seed))
    return

def mutate(seed, dist, reps, type):
    '''
    call the normal mutate or greedy mutate functions according to an epsilon greedy policy
    '''
    global epsilon
    prob = random.uniform(0,1)
    if prob < epsilon:
        normal_mutate(seed, dist, reps, type)
    else:
        greedy_mutate(seed, dist, reps, type)
    return



def add_to_cluster(seed, type):
    '''
    calculate distance of the seed from existing clusters of the same kind. 
    If minimum distance falls within the threshold, add the seed to the cluster.
    else the seed becomes a new cluster center.
    '''
    
    min_dist = cluster_dist+1
    cluster = clusters[type]

    if len(cluster) > 0:
        dists = list(map(lambda x: manhattan_distance(x, seed), cluster))
        min_dist = min(dists)
        min_clust = cluster[dists.index(min_dist)]
    if min_dist > cluster_dist:
        clusters[type].append(seed)



def plot_2D():
    '''
    function for plotting the fuzzed input space (for 2D and 3D)
    '''
    if not plot:
        return
    if inp_dim == 3:
        return plot_3D()
    inp_ax.scatter(np.array(good_inps)[:, 0], np.array(good_inps)[:, 1], marker='o', facecolors='None', edgecolors='y', s = dot_size, label = 'Useful ${\\theta}$') 
    inp_ax.scatter(np.array(bad_inps)[:, 0], np.array(bad_inps)[:, 1], marker='^', facecolors='None', edgecolors='r', s = dot_size, label = 'Non-Useful ${\\theta}$') 
    gcx = [clusters['good'][i][0] for i in range(len(clusters['good']))]
    gcy = [clusters['good'][i][1] for i in range(len(clusters['good']))]
    bcx = [clusters['bad'][i][0] for i in range(len(clusters['bad']))]
    bcy = [clusters['bad'][i][1] for i in range(len(clusters['bad']))]
    if not normal:
        inp_ax.scatter(gcx, gcy, marker="x", c='blue', s = 20 , label = 'Useful ${\\theta}$ cluster centers')
        inp_ax.scatter(bcx, bcy, marker="+", c='black', s = 20 , label = 'Non-Useful ${\\theta}$ cluster centers')
        inp_ax.legend(bbox_to_anchor =(0.5,-0.43), loc='lower center', ncol = 2, markerscale=2)
    else:
        inp_ax.scatter(gcx + bcx, gcy + bcy, marker="x", c='blue', s = 20 , label = 'Random points')
        inp_ax.legend(bbox_to_anchor =(0.5,-0.40), loc='lower center', ncol = 3, markerscale=2)
    
    inp_fig.savefig(input_map, bbox_inches='tight')
    out_fig.savefig(output_map, bbox_inches='tight')

def plot_3D():
    if not plot:
        return
    inp_ax.scatter(np.array(good_inps)[:, 0], np.array(good_inps)[:, 1], np.array(good_inps)[:, 2], marker='o', facecolors='None', edgecolors='y', s = dot_size, label = 'good points') 
    inp_ax.scatter(np.array(bad_inps)[:, 0], np.array(bad_inps)[:, 1], np.array(bad_inps)[:, 2], marker='^', facecolors='None', edgecolors='r',s = dot_size, label = 'bad points') 

    gcx = [clusters['good'][i][0] for i in range(len(clusters['good']))]
    gcy = [clusters['good'][i][1] for i in range(len(clusters['good']))]
    gcz = [clusters['good'][i][2] for i in range(len(clusters['good']))]
    bcx = [clusters['bad'][i][0] for i in range(len(clusters['bad']))]
    bcy = [clusters['bad'][i][1] for i in range(len(clusters['bad']))]
    bcz = [clusters['bad'][i][2] for i in range(len(clusters['bad']))]
    inp_ax.scatter(gcx, gcy, gcz, marker="x", c='b', s = 45 , label = 'good cluster centers')
    inp_ax.scatter(bcx, bcy, bcz, marker="+", c='black', s = 35 , label = 'bad cluster centers')
    inp_ax.legend(loc='upper right', fontsize=13)
    inp_fig.savefig(input_map, bbox_inches='tight')
    out_fig.savefig(output_map, bbox_inches='tight')



def restart():
    '''
    random restart. empties the existing queue and starts afresh with random seeds.
    '''
    q.queue.clear()
    seed = tuple([random.uniform(min_ranges[i], max_ranges[i]) for i in range(len(min_ranges))]) # TODO: convert to data type (or just cast to int in evaluate() as now)
    q.put((-1, (seed)))


# Main loop that starts the fuzzing and calls other helper functions
while not stopping_criteria():
    itr+=1
    last_new_offset+=1
    if q.empty():   # if the queue is empty, fill it with random seeds
        restart()
    score, seed = q.get() # sample a seed from the queue (FIFO)
    
    if seed in good_inputs or seed in bad_inputs:
        continue
    res = evaluate_seed(seed)
    if res: # valid seed
        good_inputs.add(seed)
        add_to_cluster(seed,'good')
        good_inps.append(seed)
        mutate(seed, s_dist, s_reps, 'good')

    else: # invalid seed
        bad_inputs.add(seed)
        add_to_cluster(seed,'bad')
        bad_inps.append(seed)
        mutate(seed, f_dist, f_reps, 'bad')

    if itr%restart_iter == 0:
        restart()
    
    # decay
    if itr%decay_iter==0:
        epsilon *= decay
    
plot_2D()

# save the fuzzed points
data = np.array(list(offsets))
np.save(data_fuzz, data)

with open(logger, 'a') as f:
    f.write(f'-----FUZZ for PROGRAM: {program_name}-----\n')
    f.write(f'Saved fuzzed offsets to {data_fuzz}\n')
    f.write(f'Number of fuzzed offsets: {len(offsets)}\n')
    f.write(f'Saved input map to file {input_map}\n')
    f.write(f'Saved output map to file {output_map}\n\n')

with open(execution_time_file, 'a') as f:
    f.write(f'{execution_time}\n')

total_input_space = np.prod(np.array(max_ranges) - np.array(min_ranges))
with open(space_evaluated_file, 'a') as f:
    f.write(f'{len(offsets)/total_input_space}\n')
