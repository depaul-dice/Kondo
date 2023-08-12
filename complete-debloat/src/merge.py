#!/usr/bin/env python
# coding: utf-8


import numpy as np
import time
from matplotlib import pyplot as plt
import random
from scipy.spatial import ConvexHull, convex_hull_plot_2d, Delaunay
from scipy.spatial import distance
import configparser
import argparse
import matplotlib
matplotlib.rcParams.update({'font.size': 16})

# Reading config options from the config file and the command line
# command line options get preference over the config file
config = configparser.ConfigParser()
config.read('config.ini')

centre_d_thresh = config.getint('Hull', 'centre_d_thresh')
boundary_d_thresh = config.getint('Hull', 'boundary_d_thresh')
granularity = config.getint('Hull', 'granularity')

inp_dim = config.getint('Fuzz', 'inp_dim')
offset_dim = config.getint('Fuzz', 'offset_dim')

ranges = config.get('Fuzz', 'min_ranges_out')
min_ranges_out = [int(num) for num in ranges.split(',')]

ranges = config.get('Fuzz', 'max_ranges_out')
max_ranges_out = [int(num) for num in ranges.split(',')]

program_name = config.get('Fuzz', 'program')
directory = config.get('Fuzz', 'directory')

parser = argparse.ArgumentParser(description='Command line configuration parser')
parser.add_argument('--inp-dim', type = int , help ='input dimension')
parser.add_argument('--min-ranges-out', type = int, nargs='+', help ='minimum output param ranges')
parser.add_argument('--max-ranges-out', type = int, nargs='+', help ='maximum output param ranges')
parser.add_argument('--offset-dim', type = int, help ='offset dimension')
parser.add_argument('--program', type = str, help ='program to run')
parser.add_argument('--directory', type = str, help ='directory of the program')
parser.add_argument('--centre-d-thresh', type = int, help ='centre distance threshold for merging')
parser.add_argument('--boundary-d-thresh', type = int, help ='boundary distance threshold for merging')
parser.add_argument('--granularity', type = int, help ='granularity of cells')
parser.add_argument('--plot', type = int, help ='whether to plot the graphs')



args, unknown = parser.parse_known_args()
if args.inp_dim:
    inp_dim = args.inp_dim
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
if args.centre_d_thresh:
    centre_d_thresh = args.centre_d_thresh
if args.boundary_d_thresh:
    boundary_d_thresh = args.boundary_d_thresh
if args.granularity:
    granularity = args.granularity
# if args.plot:
plot = args.plot


# initalising filenames for writing stats and reading data
data_fuzz = f'./data/{program_name}_fuzz.npy'
data_hull = f'./data/{program_name}_hull.npy'
data_truth = f'./data/{program_name}_truth.npy'
logger = f'./logs/{program_name}.log'
graph_output = f'./output_map/{program_name}_with_hull.png'
program = f'./{directory}/{program_name}'
output_dir = f'./output/{program_name}'
num_hulls_file = f'{output_dir}/num_hulls'
num_vertices_file = f'{output_dir}/num_vertices'
carve_time_file = f'{output_dir}/carve_time'



def plot_hull(hulls, data):
    if not plot:
        return
    if offset_dim == 2:
        plt.title("Output coverage")
        plt.xlim(min_ranges_out[0], max_ranges_out[0])
        plt.ylim(min_ranges_out[1], max_ranges_out[1])
        plt.xlabel("offsetX", fontweight='bold')
        plt.ylabel("offsetY", fontweight='bold')
        plt.scatter(data[:,0], data[:,1], c='y', s=2)
        for i in range(len(hulls)):
            points = hulls[i][1]
            # points = np.append(points, points[0])
            points = np.concatenate( (points, [points[0]]))
            plt.plot(points[:,0], points[:,1], 'r--', lw=2)
    if offset_dim == 3:
        fig = plt.figure()
        ax = fig.add_subplot(projection='3d')
        ax.set_title("Output Coverage with hulls")
        ax.set_xlabel("offsetX", fontweight='bold')
        ax.set_ylabel("offsetY", fontweight='bold')
        ax.set_zlabel("offsetZ", fontweight='bold')
        ax.set_xlim(min_ranges_out[0], max_ranges_out[0])
        ax.set_ylim(min_ranges_out[1], max_ranges_out[1])
        ax.set_zlim(min_ranges_out[2], max_ranges_out[2])
        ax.scatter(data[:,0], data[:,1], data[:,2], c='y', s=2)
        for hull in hulls:
            # print(hull)
            for s in hull[2]:
                s = np.append(s, s[0])  # Close the loop
                ax.plot(hull[3][s, 0], hull[3][s, 1], hull[3][s, 2], 'r-')
    plt.savefig(graph_output)


def calculate_hull(points):
    '''
    Given a set of points in an NxK array, calculate a basic convex hull
    blanketing all the points.

    return: a tuple containing the centroid of hull, the vertices, hull simplices and original set of points
    '''
    hull = ConvexHull(points)
    points2 = points[hull.vertices]
    mid = np.average(points2, axis = 0)
    return (mid, points2, hull.simplices, points)



def create_random_array(N, K, min_vals, max_vals):
    '''
    helper function to create a random array. Used for testing purposes.
    '''
    random_array = np.empty((N, K), dtype=int)
    for i in range(K):
        random_array[:, i] = np.random.randint(min_vals[i], max_vals[i], size=N)
    return random_array


data = np.load(data_fuzz)
cells = {}

# split all the points into cells. the length of the cell is decided by the "granularity" 
# variable in the fuzz configuration
for d in data.tolist():
    index = tuple([ d[i]//granularity for i in range(offset_dim) ])
    if index in cells:
        cells[index].append(d)
    else:
        cells[index] = [d]

hulls = []

# compute baseline hulls:
for cell in cells.values():
    if len(cell) < 3:
        points = np.array(list(cell))
        mid = np.average(points, axis = 0)
        hulls.append( (mid, points, [], points) )
    else:
        try:
            hulls.append( calculate_hull(np.array(list(cell))) )
        except:
            points = np.array(list(cell))
            mid = np.average(points, axis = 0)
            hulls.append( (mid, points, [], points) )


n = len(hulls)

hulls_c = hulls.copy()


def near(a, b):
    '''
    given two hulls a and b, returns true if they are near according to the distance thresholds in the fuzz config.
    Two hulls are near if their centres are near or the minimum distance between their vertices falls within
    the threshold/
    '''
    centre_dist =  distance.euclidean(a[0], b[0])
    vert_dist = distance.cdist(a[1], b[1]).min()
    return (centre_dist <= centre_d_thresh or (vert_dist <= boundary_d_thresh and centre_dist <= 3*centre_d_thresh))

hulls2 = []
# the main loop which iteratively merges the hulls
# in each iteration, two hulls are checked if they are close and accordingly they are merged
while len(hulls_c) > 0:
    hull_a = hulls_c.pop()
    j = 0
    to_append = True
    while j < len(hulls_c):
        hull_b = hulls_c[j]
        if near(hull_a, hull_b):
            try:
                new_hull = calculate_hull(np.concatenate( (hull_a[1], hull_b[1]), axis = 0))
            except:
                break
            del hulls_c[j]
            hulls_c.append(new_hull)
            to_append = False
            break
        j += 1
    if to_append:
        hulls2.append(hull_a)
    
plot_hull(hulls2, data)


offsets = set()
delaunays = []

for hull in hulls2:
    try:
        delaunays.append(Delaunay(hull[1]))
    except:
        continue

def in_hull(point):
    '''
    checks if the given point lies inside any of the hulls that we computed in the last step.
    '''
    inside = False
    for dela in delaunays:
        inside = inside or (dela.find_simplex(point)>=0)
    return inside


# ----------------------- Carving ------------------------------
'''
iterate over all points and check for each point if it lies inside any hull. save the points which 
are a member of any hull as the final carved dataset.
'''
carve_start = time.time()
# check for all points if they are inside hull
point= []
def brute(dim):
    if dim == offset_dim:
        # print(point)
        if in_hull(point):
            offsets.add(tuple(point))
        return
    for i in range(min_ranges_out[dim], max_ranges_out[dim]):
        point.append(i)
        brute(dim+1)
        point.pop()

brute(0)

offsets = np.array(list(offsets))
np.save(data_hull, offsets)

with open(carve_time_file, 'a') as f:
    f.write(f'{time.time() - carve_start}\n')


with open(logger, 'a') as f:
    f.write(f'-----HULL for PROGRAM: {program_name}------\n')
    f.write(f'Saved predicted valid offsets to {data_hull}\n')
    f.write(f'Saved predicted valid offsets graph to {graph_output}\n')
    f.write(f'Total number of predicted offsets: {offsets.shape[0]}\n\n')

with open(num_hulls_file, 'a') as f:
    f.write(f'{len(hulls2)}\n')

with open(num_vertices_file, 'a') as f:
    for hull in hulls2:
        f.write(f'{len(hull[1])},')
    f.write('\n')
