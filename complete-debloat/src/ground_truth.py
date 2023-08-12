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
import configparser
import argparse
import matplotlib
matplotlib.rcParams.update({'font.size': 16})

offsets = set()


# Reading config options from the config file and the command line
# command line options get preference over the config file
config = configparser.ConfigParser()
config.read('config.ini')

inp_dim = config.getint('Fuzz', 'inp_dim')
offset_dim = config.getint('Fuzz', 'offset_dim')

ranges = config.get('Fuzz', 'min_ranges')
min_ranges = [int(num) for num in ranges.split(',')]
ranges = config.get('Fuzz', 'max_ranges')
max_ranges = [int(num) for num in ranges.split(',')]

ranges = config.get('Fuzz', 'min_ranges_out')
min_ranges_out = [int(num) for num in ranges.split(',')]
ranges = config.get('Fuzz', 'max_ranges_out')
max_ranges_out = [int(num) for num in ranges.split(',')]
directory = config.get('Fuzz', 'directory')

program_name = config.get('Fuzz', 'program')
dot_size = config.getint('Fuzz', 'dot_size')


parser = argparse.ArgumentParser(description='Command line configuration parser')
parser.add_argument('--inp-dim', type = int , help ='input dimension')
parser.add_argument('--min-ranges', type = int, nargs='+', help ='minimum input param ranges')
parser.add_argument('--max-ranges', type = int, nargs='+', help ='maximum input param ranges')
parser.add_argument('--min-ranges-out', type = int, nargs='+', help ='minimum output param ranges')
parser.add_argument('--max-ranges-out', type = int, nargs='+', help ='maximum output param ranges')
parser.add_argument('--offset-dim', type = int, help ='offset dimension')
parser.add_argument('--program', type = str, help ='program to run')
parser.add_argument('--directory', type = str, help ='directory of the program')
parser.add_argument('--compute-truth', type = int, help ='directory of the program')

args, unkown = parser.parse_known_args()
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
if args.program:
    program_name = args.program
if args.directory:
    directory = args.directory
compute_truth = args.compute_truth

# initalising filenames for writing stats and reading data
data_fuzz = f'./data/{program_name}_fuzz.npy'
data_truth = f'./data/{program_name}_truth.npy'
data_hull = f'./data/{program_name}_hull.npy'
logger = f'./logs/{program_name}.log'
input_map = f'./input_map/{program_name}.png'
output_map = f'./output_map/{program_name}_truth.png'
program = f'./{directory}/{program_name}'

output_dir = f'./output/{program_name}'
precision_file = f'{output_dir}/precision'
recall_file = f'{output_dir}/recall'
time_file = f'{output_dir}/time'
plot_time_file = f'{output_dir}/bf_plot_time'
data_reduction = f'{output_dir}/data_reduction'
space_evaluated = f'{output_dir}/parameter_space_evaluated'
offsets_vs_time = f'{output_dir}/bf_offsets_vs_time'

# setting the plots
if offset_dim == 2:
    out_fig, out_ax = plt.subplots()
    out_ax.set_title("Output coverage")
    out_ax.set_xlim(min_ranges_out[0], max_ranges_out[0])
    out_ax.set_ylim(min_ranges_out[1], max_ranges_out[1])
    out_ax.set_xlabel("offsetX", fontweight='bold')
    out_ax.set_ylabel("offsetY", fontweight='bold')

if offset_dim == 3:
    out_fig = plt.figure()
    out_ax = out_fig.add_subplot(projection='3d')
    out_ax.set_title("Output coverage")
    out_ax.set_xlim(min_ranges_out[0], max_ranges_out[0])
    out_ax.set_ylim(min_ranges_out[1], max_ranges_out[1])
    out_ax.set_zlim(min_ranges_out[2], max_ranges_out[2])
    out_ax.set_xlabel("$x_1$", fontweight='bold')
    out_ax.set_ylabel("$x_2$", fontweight='bold')
    out_ax.set_zlabel("$x_3$", fontweight='bold')

program_start = time.time()

def evaluate_seed(seed):
    cmd = [f'{i}' for i in seed]
    cmd.insert(0, program)
    output = subprocess.run(cmd, stdout=subprocess.PIPE)
    output = output.stdout.decode('utf-8')

    if output == '':
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
            if len(offsets)%1000 == 0:
                with open(offsets_vs_time, 'a') as f:
                    f.write(f'{time.time() - program_start}: Offsets discovered: {len(offsets)}\n')

    return True



offsets.clear()
point = []

# brute force loop to iterate over all possible parameter combinations
def brute(dim):
    if dim == offset_dim:
        # print(point)
        evaluate_seed(point)
        return
    for i in range(min_ranges_out[dim], max_ranges_out[dim]):
        point.append(i)
        brute(dim+1)
        point.pop()

with open(logger, 'a') as f:
    f.write(f'-----GROUND TRUTH for PROGRAM: {program_name}------\n')
    f.write(f'Running Brute force for ground truth\n')

if compute_truth:
    brute(0)
    offsets = np.array(list(offsets))

    start = time.time()
    # if offset_dim == 2:
    #     out_ax.scatter(offsets[:, 0], offsets[:, 1], s=2, c='y')
    # else:
    #     out_ax.scatter(offsets[:, 0], offsets[:, 1], offsets[:, 2], s=2, c='y')

    # out_fig.savefig(output_map)
    np.save(data_truth, offsets)
    plot_duration = time.time() - start

    with open(plot_time_file, 'a') as f:
        f.write(f'{plot_duration}\n')


# calculating the metrics for the given program based on the brute force baseline
truth = np.load(data_truth).tolist()
fuzzed = np.load(data_hull).tolist()
truth = set(tuple(row) for row in truth)
fuzzed = set(tuple(row) for row in fuzzed)

intersection = len(truth & fuzzed)
union = len(truth | fuzzed)

recall = intersection/len(truth)
try:
    precision = intersection/len(fuzzed)
except:
    precision = 1

total_output_space = np.prod(np.array(max_ranges_out) - np.array(min_ranges_out))

with open(logger, 'a') as f:
    f.write(f'Saved ground truth graph to {output_map}\n')
    f.write(f'Total points in the ground truth: {len(truth)}\n')
    f.write(f'Total predicted points: {len(fuzzed)}\n')
    f.write(f'Size of intersection: {intersection}\n')
    f.write(f'Precision: {precision}, Recall: {recall}\n')
    f.write(f'Total space: {total_output_space}\n')
    f.write(f'Percent Reduction: {(total_output_space - len(fuzzed))/total_output_space}\n')

with open(precision_file, 'a') as f:
    f.write(f'{precision}\n')

with open(recall_file, 'a') as f:
    f.write(f'{recall}\n')
    
with open(data_reduction, 'a') as f:
    f.write(f'{(total_output_space - len(fuzzed))/total_output_space}\n')

