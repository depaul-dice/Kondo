import numpy as np
from matplotlib import pyplot as plt 
import os
import sys
import argparse
import time
import pandas as pd 

program_reps = 1
hull_centre_threshes = [ 1, 4, 8, 12, 20, 28, 40, 80, 160, 280]


def program_run(params_hull, params_fuzzer):

    for thresh in hull_centre_threshes:
        params_hull['centre_d_thresh'] = thresh
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

        if thresh == 1:
            os.system(f'rm -rf {output_dir}/*')
            os.system(f'rm {logger}')
            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

        # Starting Simulation
        
        for i in range(program_reps):
            print("Program:", program_name, ", Running iter: ", i)
            start = time.time()
            os.system(f'python3 src/merge.py {arg_str_fuzzer} {arg_str_hull} --plot 0')
            merge_time = time.time()
            if os.path.isfile(data_truth):
                os.system(f'python3 src/ground_truth.py {arg_str_fuzzer} --compute-truth 0')
            else:
                print("Computing Truth: ")
                os.system(f'python3 src/ground_truth.py {arg_str_fuzzer} --compute-truth 1')



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
        
        
if __name__ == '__main__':
    main()
    programs =  ['CS1', 'CS2', 'CS3', 'CS4', 'CS5', 'PRL2D', 'LDC2D', 'RDC2D', 'PRL3D', 'LDC3D', 'RDC3D']

    recall = pd.DataFrame(index = programs,  columns=hull_centre_threshes)
    precision = pd.DataFrame(index = programs, columns=hull_centre_threshes)
    for program in programs:
        rec = open(f'./output/{program}/recall', 'r').read().split('\n')[:-1]
        recall.loc[program] = rec
    
    for program in programs:
        prec = open(f'./output/{program}/precision', 'r').read().split('\n')[:-1]
        precision.loc[program] = prec

    precision.to_csv('./paper_result_data/centre_dist_precision.csv')
    recall.to_csv('./paper_result_data/centre_dist_recall.csv')