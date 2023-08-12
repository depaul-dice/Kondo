import numpy as np
from matplotlib import pyplot as plt 
import os
import sys
import argparse
import time

program_reps = 1
offset_spaces = [128, 256, 512, 1024, 2048]


def program_run(params_hull, params_fuzzer, off):

    # for off in offset_spaces:
    params_fuzzer['max_ranges'] = [off, off]
    params_fuzzer['max_ranges_out'] = [off, off]
    params_fuzzer['s_dist'] *= off//128
    params_fuzzer['f_dist'] *= off//128
    # params_fuzzer['max_iter'] = 2000 * off//128
    params_fuzzer['max_iter'] = 10000
    params_fuzzer['restart_iter'] = 500
    params_hull['centre_d_thresh'] += off//128
    params_hull['boundary_d_thresh'] += off//128
    
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

    
    # os.system(f'rm -rf {output_dir}/*')
    os.system(f'rm {output_dir}/recall')
    os.system(f'rm -rf {output_dir}/precision')
    os.system(f'rm -rf {output_dir}/time')
    os.system(f'rm -rf {output_dir}/fuzz_time')
    os.system(f'rm -rf {output_dir}/execution_time')
    os.system(f'rm -rf {output_dir}/merge_time')
    os.system(f'rm -rf {output_dir}/num_hulls')
    os.system(f'rm -rf {output_dir}/num_vertices')
    os.system(f'rm -rf {output_dir}/parameter_space_evaluated')
    os.system(f'rm -rf {output_dir}/data_reduction')
    
    
    # os.system(f'rm {logger}')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Starting Simulation
    
    for i in range(program_reps):
        print("Program:", program_name, ", Running iter: ", i)
        start = time.time()
        os.system(f'python3 src/fuzz.py {arg_str_fuzzer} --plot 0')
        fuzz_time = time.time()
        os.system(f'python3 src/merge.py {arg_str_fuzzer} {arg_str_hull} --plot 0')
        merge_time = time.time()
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
            
    # os.system(f'python3 src/fuzz.py {arg_str_fuzzer} --plot 1')
    # os.system(f'python3 src/merge.py {arg_str_fuzzer} {arg_str_hull} --plot 1')
    
    
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


    for off in offset_spaces:
        params_fuzzer2D['program'] = f'CS4_{off}'
        params_fuzzer2D['directory'] = './space_variation_exp/bin'
        program_run(params_hull, params_fuzzer2D, off)
        # analyse_results()
        
        
if __name__ == '__main__':
    main()