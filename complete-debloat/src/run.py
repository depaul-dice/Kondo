'''
time will be recorded here. precision and recall would be written by individual program runs.
mean, std deviation and graphs etc of all the precision, recall and time data would be done here.
'''

import numpy as np
from matplotlib import pyplot as plt 
import os
import sys
import argparse
import time

program_reps = 1


parser = argparse.ArgumentParser(description='Command line configuration parser')
parser.add_argument('--program', type = str, help ='program to run')
parser.add_argument('--directory', type = str, help ='directory of the program')
parser.add_argument('--reps', type = int, help ='repition of runs')

args = parser.parse_args()
if args.program:
    program_name = args.program
if args.directory:
    directory = args.directory
if args.reps:
    program_reps = args.reps


def program_run(params_hull, params_fuzzer):
    arg_str_hull = ''
    for key, value in params_hull.items():
        key_arg = key.replace('_', '-')
        arg_str_hull = f'{arg_str_hull} --{key_arg}'
        if isinstance(value, list):
            for val in value:
                arg_str_hull = f'{arg_str_hull} {val}'
        else:
            arg_str_hull = f'{arg_str_hull} {value}'
    
    arg_str_fuzzer = ''
    for key, value in params_fuzzer.items():
        key_arg = key.replace('_', '-')
        arg_str_fuzzer = f'{arg_str_fuzzer} --{key_arg}'
        if isinstance(value, list):
            for val in value:
                arg_str_fuzzer = f'{arg_str_fuzzer} {val}'
        else:
            arg_str_fuzzer = f'{arg_str_fuzzer} {value}'
        
    # print(arg_str_fuzzer)
    # print(arg_str_hull)
    # return

    # making paths and dirs
    program_name = params_fuzzer['program']
    directory = params_fuzzer['directory']
    data_fuzz = f'./data/{program_name}_fuzz.npy'
    data_truth = f'./data/{program_name}_truth.npy'
    data_hull = f'./data/{program_name}_hull.npy'
    logger = f'./logs/{program_name}.log'
    input_map = f'./input_map/{program_name}.png'
    output_map = f'./output_map/{program_name}_fuzz.png'
    program = f'./{directory}/{program_name}'

    output_dir = f'./output/{program_name}'
    precision_file = f'{output_dir}/precision'
    recall_file = f'{output_dir}/recall'
    time_file = f'{output_dir}/time'
    fuzz_time_file = f'{output_dir}/fuzz_time'
    merge_time_file = f'{output_dir}/merge_time'
    bf_time_file = f'{output_dir}/brute_force_time'


    os.system(f'rm -rf {output_dir}/*')
    os.system(f'rm {logger}')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Starting Simulation
        
    for i in range(program_reps):
        print("Program:", program_name, ", Running iter: ", i)
        print('Running Kondo fuzzer')
        start = time.time()
        os.system(f'python3 src/fuzz.py {arg_str_fuzzer} --plot 0')
        fuzz_time = time.time()
        print('Running Kondo Carver')
        os.system(f'python3 src/merge.py {arg_str_fuzzer} {arg_str_hull} --plot 0')
        merge_time = time.time()
        print('Running Ground Truth')
        if os.path.isfile(data_truth):
            os.system(f'python3 src/ground_truth.py {arg_str_fuzzer} --compute-truth 0')
        else:
            print("Computing Truth: ")
            bf_start = time.time()
            os.system(f'python3 src/ground_truth.py {arg_str_fuzzer} --compute-truth 1')
            with open(bf_time_file, 'a') as f:
                f.write(f'{time.time() - bf_start}\n')

        with open(time_file, 'a') as f:
            f.write(f'{merge_time - start}\n')
        with open(fuzz_time_file, 'a') as f:
            f.write(f'{fuzz_time - start}\n')
        with open(merge_time_file, 'a') as f:
            f.write(f'{merge_time - fuzz_time}\n')

        
        # one final run for plotting
    # os.system(f'python3 src/fuzz.py {arg_str_fuzzer} --plot 1')
    # os.system(f'python3 src/merge.py {arg_str_fuzzer} {arg_str_hull} --plot 1')


def analyse_results():
    precisions = np.array(open(precision_file, 'r').read().split('\n')[:-1], dtype=float)
    recalls = np.array(open(recall_file, 'r').read().split('\n')[:-1], dtype=float)
    times = np.array(open(time_file, 'r').read().split('\n')[:-1], dtype=float)

    mean_precision = np.mean(precisions)
    mean_recall = np.mean(recalls)
    mean_time = np.mean(times)

    std_precision = np.std(precisions)
    std_recall = np.std(recalls)
    std_time = np.std(times)

    print(f'Program: {program}')
    print(f'Precision: mean = {mean_precision}, std dev = {std_precision}')
    print(f'Recall: mean = {mean_recall}, std dev = {std_recall}')
    print(f'Time: mean = {mean_time}, std dev = {std_time}')

    with open(logger, 'a') as f:
        f.write(f'Precision: mean = {mean_precision}, std dev = {std_precision}')
        f.write(f'Recall: mean = {mean_recall}, std dev = {std_recall}')
        f.write(f'Time: mean = {mean_time}, std dev = {std_time}')
    
    with open(precision_file, 'a') as f:
        f.write(f'Precision: mean = {mean_precision}, std dev = {std_precision}')
    
    with open(recall_file, 'a') as f:
        f.write(f'Recall: mean = {mean_recall}, std dev = {std_recall}')
    
    with open(time_file, 'a') as f:
        f.write(f'Time: mean = {mean_time}, std dev = {std_time}')

    # precision graph
    plt.xlabel('Run #')
    plt.ylabel('Precision recorded')
    plt.plot(np.arange(len(precisions)), precisions, color='b')
    plt.savefig(f'{precision_file}.png')
    plt.close()

    # recall graph
    plt.xlabel('Run #')
    plt.ylabel('Recall recorded')
    plt.plot(np.arange(len(recalls)), recalls, color='b')
    plt.savefig(f'{recall_file}.png')
    plt.close()

    # time graph
    plt.xlabel('Run #')
    plt.ylabel('Time recorded')
    plt.plot(np.arange(len(times)), times, color='b')
    plt.savefig(f'{time_file}.png')
    plt.close()



def main():
    params_hull = {
        'centre_d_thresh' : 20, 
        'boundary_d_thresh': 10,
        'granularity' : 16
    }
    params_fuzzer2D = {
        'inp_dim' : 2,
        'min_ranges' : [0,0],
        'max_ranges' : [128,128],
        'min_ranges_out' : [0,0],
        'max_ranges_out' : [128,128],
        'offset_dim' : 2,
        's_reps' : 8,
        'f_reps' : 5,
        's_dist' : [5,15],
        'f_dist' : [30,50],
        'max_iter' : 2000,
        'cluster_dist' : 30,
        'decay' : 0.95,
        'decay_iter' : 70,
        'restart_iter' : 100,
        # 'program' : program_name,
        # 'directory' : directory
    }

    params_fuzzer3D = {
        'inp_dim' : 3,
        'min_ranges' : [0,0,0],
        'max_ranges' : [64,64,64],
        'min_ranges_out' : [0,0,0],
        'max_ranges_out' : [64,64,64],
        'offset_dim' : 3,
        's_reps' : 8,
        'f_reps' : 5,
        's_dist' : [5,15],
        'f_dist' : [30,50],
        'max_iter' : 1000,
        'cluster_dist' : 30,
        'decay' : 0.95,
        'decay_iter' : 40,
        'restart_iter' : 40,
        # 'program' : program_name,
        # 'directory' : directory
    }

    programs2D = ['CS1', 'CS2', 'CS3', 'CS4', 'CS5']
    benchmarks2D = ['PRL2D', 'LDC2D', 'RDC2D']
    benchmarks3D = ['PRL3D', 'LDC3D', 'RDC3D']

    for program in programs2D:
        params_fuzzer2D['program'] = program
        params_fuzzer2D['directory'] = './programs/bin'
        program_run(params_hull, params_fuzzer2D)
        # analyse_results()
    
    for program in benchmarks2D:
        params_fuzzer2D['program'] = program
        params_fuzzer2D['directory'] = './benchmarks/bin'
        program_run(params_hull, params_fuzzer2D)

    for program in benchmarks3D:
        params_fuzzer3D['program'] = program
        params_fuzzer3D['directory'] = './benchmarks/bin'
        program_run(params_hull, params_fuzzer3D)

if __name__=="__main__":
    main()