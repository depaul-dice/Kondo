import numpy as np
import pandas as pd


def kondo_stats():
    '''
    Calculate the precision, recall and data reduction for kondo on all programs
    and save to a file for plotting
    '''
    metrics = ['recall', 'precision', 'data_reduction']
    
    stats = pd.DataFrame(
        index = ['CS', 'PRL', 'LDC', 'RDC'],
        columns = ['recall_mean', 'recall_std',
                   'precision_mean', 'precision_std',
                   'data_reduction_mean', 'data_reduction_std']
    )
    
    programs_list = {
        'CS' : ['CS1', 'CS2', 'CS3', 'CS4', 'CS5'],
        'PRL': ['PRL2D', 'PRL3D'],
        'LDC': ['LDC2D', 'LDC3D'],
        'RDC': ['RDC2D', 'RDC3D']
    }
    
    for key, programs in programs_list.items():
        for metric in metrics:
            metric_data = np.array([])
            for program in programs:
                data = open(f'./output/{program}/{metric}').read().split('\n')[:-1]
                data = np.array(data, dtype = 'float')
                metric_data = np.concatenate((metric_data, data))
            stats.loc[key][f'{metric}_mean'] = metric_data.mean()
            stats.loc[key][f'{metric}_std'] = metric_data.std()
    
    stats.to_csv('./paper_result_data/kondo_performance_metrics.csv')


def baseline_stats():
    '''
    Calculate the precision, recall and data reduction for BF and AFL on all programs
    and save to a file for plotting
    The values for this section are taken from AFL and BF logs.
    '''
    baseline_perf_stats = pd.DataFrame(index = ['CS', 'PRL', 'LDC', 'RDC'],
                                   columns = ['brute_force_recall', 'brute_force_precision', 'afl_recall', 'afl_precision']
                                   )
    baseline_perf_stats['brute_force_precision'] = 1
    baseline_perf_stats['afl_precision'] = 1

    CS_offset_space = np.array([128*128, 128*128, 128*128, 128*128, 128*128, 128*128])
    peripheral_offset_space = np.array([128*128, 64*64*64])
    left_offset_space = np.array([128*128, 64*64*64])
    right_offset_space = np.array([128*128, 64*64*64])
    
    
    bf_CS_num_offsets = np.array([4200, 2200, 5000, 3300, 3000, 1200])
    bf_CS_total = np.array([5777, 4160, 6911, 5589, 3318, 1590])
    bf_CS_recall = bf_CS_num_offsets.sum()/bf_CS_total.sum()
    baseline_perf_stats.loc['CS','brute_force_recall'] = bf_CS_recall
    baseline_perf_stats.loc['CS','brute_force_data_reduction'] = (CS_offset_space - bf_CS_num_offsets).sum()/CS_offset_space.sum()
    
    
    bf_peripheral_num_offsets = np.array([12000, 6400])
    bf_peripheral_total = np.array([12440, 120000])
    bf_peripheral_recall = bf_peripheral_num_offsets.sum()/bf_peripheral_total.sum()
    baseline_perf_stats.loc['PRL','brute_force_recall'] = bf_peripheral_recall
    baseline_perf_stats.loc['PRL','brute_force_data_reduction'] = (peripheral_offset_space - bf_peripheral_num_offsets).sum()/peripheral_offset_space.sum()
    
    bf_left_num_offsets = np.array([4000, 6000])
    bf_left_total = np.array([4000, 36000])
    bf_left_recall = bf_left_num_offsets.sum()/bf_left_total.sum()
    baseline_perf_stats.loc['LDC','brute_force_recall'] = bf_left_recall
    baseline_perf_stats.loc['LDC','brute_force_data_reduction'] = (left_offset_space - bf_left_num_offsets).sum()/left_offset_space.sum()
    
    bf_right_num_offsets = np.array([4000, 7000])
    bf_right_total = np.array([4000, 36000])
    bf_right_recall = bf_right_num_offsets.sum()/bf_right_total.sum()
    baseline_perf_stats.loc['RDC','brute_force_recall'] = bf_right_recall
    baseline_perf_stats.loc['RDC','brute_force_data_reduction'] = (right_offset_space - bf_right_num_offsets).sum()/right_offset_space.sum()

    afl_CS_num_offsets = np.array([419, 193, 337, 482, 685, 77])
    afl_CS_total = np.array([5777, 4160, 6911, 5589, 3318, 1590])
    afl_CS_recall = afl_CS_num_offsets.sum()/afl_CS_total.sum()
    baseline_perf_stats.loc['CS','afl_recall'] = afl_CS_recall
    baseline_perf_stats.loc['CS','afl_data_reduction'] = (CS_offset_space - afl_CS_num_offsets).sum()/CS_offset_space.sum()
    
    
    afl_peripheral_num_offsets = np.array([800, 2392])
    afl_peripheral_total = np.array([12440, 120000])
    afl_peripheral_recall = afl_peripheral_num_offsets.sum()/afl_peripheral_total.sum()
    baseline_perf_stats.loc['PRL','afl_recall'] = afl_peripheral_recall
    baseline_perf_stats.loc['PRL','afl_data_reduction'] = (peripheral_offset_space - afl_peripheral_num_offsets).sum()/peripheral_offset_space.sum()
    
    
    afl_left_num_offsets = np.array([2133, 6000])
    afl_left_total = np.array([4000, 36000])
    afl_left_recall = afl_left_num_offsets.sum()/afl_left_total.sum()
    baseline_perf_stats.loc['LDC','afl_recall'] = afl_left_recall
    baseline_perf_stats.loc['LDC','afl_data_reduction'] = (left_offset_space - afl_left_num_offsets).sum()/left_offset_space.sum()
    
    
    afl_right_num_offsets = np.array([1315, 7000])
    afl_right_total = np.array([4000, 36000])
    afl_right_recall = afl_right_num_offsets.sum()/afl_right_total.sum()
    baseline_perf_stats.loc['RDC','afl_recall'] = afl_right_recall
    baseline_perf_stats.loc['RDC','afl_data_reduction'] = (right_offset_space - afl_right_num_offsets).sum()/right_offset_space.sum()
    
    baseline_perf_stats.to_csv('./paper_result_data/baseline_performance_stats.csv')



def time_measurements():
    '''
    Read the time taken by all methods over al program and write to a csv
    '''

    metrics = ['execution_time', 'merge_time', 'fuzz_time', 'brute_force_time']
    
    stats = pd.DataFrame(
        index = ['CS', 'PRL', 'LDC', 'RDC'],
        columns = ['execution_time_mean', 'execution_time_std',
                   'merge_time_mean', 'merge_time_std',
                   'fuzz_time_mean', 'fuzz_time_std',
                   'brute_force_time_mean', 'brute_force_time_std',
                   'afl_time_mean', 'afl_time_std']
    )
    
    programs_list = {
        'CS' : ['CS1', 'CS2', 'CS3', 'CS4', 'CS5'],
        'PRL': ['PRL2D', 'PRL3D'],
        'LDC': ['LDC2D', 'LDC3D'],
        'RDC': ['RDC2D', 'RDC3D']
    }
    
    for key, programs in programs_list.items():
        for metric in metrics:
            metric_data = np.array([])
            for program in programs:
                data = open(f'./output/{program}/{metric}').read().split('\n')[:-1]
                data = np.array(data, dtype = 'float')
                metric_data = np.concatenate((metric_data, data))
            stats.loc[key][f'{metric}_mean'] = metric_data.mean()
            stats.loc[key][f'{metric}_std'] = metric_data.std()
    
    stats.loc['PRL', 'afl_time_mean'] = 0
    stats.loc['PRL', 'afl_time_std'] = 0
    stats.loc['LDC', 'afl_time_mean'] = 0
    stats.loc['LDC', 'afl_time_std'] = 0
    stats.loc['RDC', 'afl_time_mean'] = 0
    stats.loc['RDC', 'afl_time_std'] = 0
    CS_times = np.array([2581, 2048, 3264, 2518, 902, 1352])
    stats.loc['CS', 'afl_time_mean'] = CS_times.mean()
    stats.loc['CS', 'afl_time_std'] = CS_times.std()
    
    stats.to_csv('./paper_result_data/time_stats.csv')
    

kondo_stats()
baseline_stats()
time_measurements()
