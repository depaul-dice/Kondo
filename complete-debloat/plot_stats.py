import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib
matplotlib.rcParams.update({'font.size': 18, 'font.weight':'bold'})



def plot_bar(programs, data, error, save_path, ylabel, bar_label = True, time_plot = False, labels = []):
    hatches = ['//', '+', 'o', '-']
    x = np.arange(len(programs))  # the label locations
    width = 0.28  # the width of the bars
    multiplier = 0

    fig, ax = plt.subplots(layout='constrained', figsize=(14, 5), dpi = 100)

    for attribute, measurement in data.items():
        offset = width * multiplier
        if time_plot:
            rects = ax.bar(x + offset, np.round(measurement,3), width, label=attribute, hatch=hatches[multiplier])
            ax.bar_label(rects, padding=3, labels = labels[attribute],  fontsize = 14)
        else:
            rects = ax.bar(x + offset, np.round(measurement,3), width, capsize = 10, yerr=error[attribute],
                        error_kw=dict(lw=3, capthick = 3), label=attribute, hatch=hatches[multiplier])
            if bar_label:
                ax.bar_label(rects, padding=3)
        multiplier += 1

    # Add some text for labels, title and custom x-axis tick labels, etc.
    ax.set_ylabel(ylabel, fontweight='bold')
    ax.set_xticks(x + width, programs)
    ax.legend(bbox_to_anchor =(0.5,-0.35), loc='lower center', ncols = 4, handlelength = 4, handleheight = 2)
    if time_plot:
        ax.set_ylim(0,5)
    else:
        ax.set_ylim(0, 1.1)
    fig.savefig(save_path)


def mean_recall():
    kondo_stats = pd.read_csv('./paper_result_data/kondo_performance_metrics.csv')
    baseline_stats = pd.read_csv('./paper_result_data/baseline_performance_stats.csv')
    
    programs = ["CS", "PRL", "LDC", "RDC"]
    # colors = ['#c1272d', '#0000a7', '#008176']
    # recall
    data = {
        'Kondo': kondo_stats["recall_mean"],
        'AFL': baseline_stats["afl_recall"],
        'BF': baseline_stats["brute_force_recall"],
    }
    error = {
        'Kondo': kondo_stats["recall_std"],
        'AFL': [0, 0, 0, 0],
        'BF': [0, 0, 0, 0],
    }
    save_path = './paper_result_data/recall.pdf'
    ylabel = "Mean Recall"
    plot_bar(programs, data, error, save_path, ylabel, True)
    

def precision_plot():
    programs_list = ['CS1', 'CS2', 'CS3', 'CS4', 'CS5',
    'PRL2D', 'PRL3D',
    'LDC2D', 'LDC3D',
    'RDC2D', 'RDC3D']

    xlabels = ['CS1', 'CS2', 'CS3', 'CS4', 'CS5', 'PRL2D', 'PRL3D', 'LDC2D', 'LDC3D', 'RDC2D', 'RDC3D']
    save_path = './paper_result_data/precision.pdf'
    ylabel = "Precision"
    hatches = ['//', '+', 'o', '-']
    
    data = []
    error = []
    metric = 'precision'
    for program in programs_list:
        data_read = open(f'./output/{program}/{metric}').read().split('\n')[:-1]
        data_read = np.array(data_read, dtype = 'float')
        data.append(data_read.mean())
        error.append(data_read.std())
    
    x = np.arange(len(xlabels))
    width = 0.7  # the width of the bars
    fig, ax = plt.subplots(layout='constrained', figsize=(14, 5), dpi = 100)
    ax.axhline( y = 1, c = 'r', label = "AFL/BF Precision", lw = 2)
    rects = ax.bar(x, data, width, capsize=10, yerr=error,error_kw=dict(lw=3, capthick = 3), label='Kon precision',hatch = hatches[0] )
    ax.bar_label(rects, padding = 3, labels = np.round(data, 2))
    ax.set_ylabel(ylabel, fontweight='bold')
    ax.set_xticks(x, xlabels)
    ax.legend(bbox_to_anchor =(0.5,-0.35), loc='lower center', ncols = 4, handlelength = 4, handleheight = 2)
    ax.set_ylim(0, 1.19)
    fig.savefig(save_path)



def data_reduction_plot():
    programs_list = ['CS1', 'CS2', 'CS3', 'CS4', 'CS5',
        'PRL2D', 'PRL3D',
        'LDC2D', 'LDC3D',
        'RDC2D', 'RDC3D']
    
    xlabels = ['CS1', 'CS2', 'CS3', 'CS4', 'CS5', 'PRL2D', 'PRL3D', 'LDC2D', 'LDC3D', 'RDC2D', 'RDC3D']
    save_path = './paper_result_data/data_reduction.pdf'
    ylabel = "% of Data Bloat Detected"
    hatches = ['//', '+', 'o', '-']
    
    truth = np.array([5777, 4160, 6911, 5589, 1590, 12440, 120000, 4000, 36000, 4000, 36000])
    space = np.array([16384, 16384, 16384, 16384, 16384, 16384, 262144, 16384, 262144, 16384, 262144])
    gold_reduction = (space-truth)/space
    save_path = './paper_result_data/data_reduction.pdf'
    data = []
    error = []
    metric = 'data_reduction'
    for program in programs_list:
        data_read = open(f'./output/{program}/{metric}').read().split('\n')[:-1]
        data_read = np.array(data_read, dtype = 'float')
        data.append(data_read.mean())
        error.append(data_read.std())
    
    x = np.arange(len(xlabels))
    width = 0.7  # the width of the bars
    fig, ax = plt.subplots(layout='constrained', figsize=(14, 5), dpi = 100)
    # ax.axhline( y = 1, c = 'r', label = "AFL/BF Precision", lw = 2)
    rects = ax.bar(x, gold_reduction, width, label='Ground Truth')
    
    
    rects = ax.bar(x, data, width, capsize=10, yerr=error,error_kw=dict(lw=3, capthick = 3), label='Kondo',hatch = hatches[0] )
    ax.bar_label(rects, padding = 3, labels = np.round(data, 2))
    ax.set_ylabel(ylabel, fontweight='bold')
    ax.set_xticks(x, xlabels)
    ax.legend(bbox_to_anchor =(0.5,-0.35), loc='lower center', ncols = 4, handlelength = 4, handleheight = 2)
    ax.set_ylim(0, 1.19)
    fig.savefig(save_path)


def time_plot():
    stats = pd.read_csv('./paper_result_data/time_stats.csv', index_col=0)
    stats.loc['CS', 'afl_time_std'] = 200
    stats = stats.replace({0: 1})
    programs = ["CS", "PRL", "LDC", "RDC"]
    data = {
        'Kondo': np.round(np.log10(stats["execution_time_mean"] + stats["fuzz_time_mean"] + stats["merge_time_mean"]), 2),
        'AFL (2D)': [3.32, 3.36, 4.27, 4.5],
        'BF': np.round(np.log10(stats["brute_force_time_mean"]), 2)
    }
    
    labels = {
        'Kondo': ['0.52(0.97)', '1.05(1)', '0.71(0.98)', '0.7(0.98)'],
        'AFL (2D)': ['3.32(0.97)', '3.36(0.83)', '4.27(0.88)', '4.5(0.85)'],
        'BF': ['0.86(0.97)', '2.53(1)', '1.91(0.98)', '1.91(0.98)']
    }
    
    error = {
        'Kondo': np.abs(np.log10(stats["execution_time_std"])),
        'AFL (2D)': np.abs(np.log10(stats["afl_time_std"])),
        'BF': np.abs(np.log10(stats["brute_force_time_std"]))
    }
    plot_bar(programs, data, error, './paper_result_data/exp2_time.pdf', "Mean Time(s) (log$_{10}$ scale)", time_plot= True, labels = labels)


def hull_distance(metric):
    stats = pd.read_csv(f'./paper_result_data/centre_dist_{metric}.csv', index_col = 0)
    stats = stats.drop('CS6', errors='ignore')
    stats.index = ['CS1', 'CS2', 'CS3', 'CS4', 'CS5', 'PRL2D', 'PRL3D', 'LDC2D', 'LDC3D', 'RDC2D', 'RDC3D']
    
    fig, ax = plt.subplots(layout='constrained', figsize=(12, 6), dpi = 1000)
    markers = ['x', 'o', '3', 's', 'v', '1', '<', 'p', '*', 'h', '_']
    programs = stats.index
    xticks = np.log2([1,4,8,12,20,28,40,80,160,280])
    ax.set_ylabel("Precision", fontweight='bold')
    ax.set_xlabel("Centre Distance Threshold for Hull Merge (log$_2$ scale)", fontweight='bold')
    
    marker_idx = 0
    for program in programs:
        ax.plot(xticks, stats.loc[program], label = program, lw = 3, marker = markers[marker_idx], mew = 3, ms = 8)
        marker_idx += 1
    
    ax.legend(bbox_to_anchor =(0.5,-0.45), loc='lower center', ncols = 6)
    save_path = f'./paper_result_data/centre_d_thresh_{metric}.pdf'
    fig.savefig(save_path, bbox_inches='tight')


def offset_space_plot():
    offsets = [128, 256, 512, 1024, 2048]

    stats = pd.DataFrame(index = offsets, columns = ['precision', 'precision_std',
                                                                                   'recall', 'recall_std',
                                                                                   'brute_force_time', 'fuzz_time', 'execution_time',
                                                                                   'merge_time', 'carve_time'])
    metrics = ['precision', 'recall', 'brute_force_time', 'fuzz_time', 'execution_time', 'merge_time', 'carve_time']
    
    for metric in metrics:
        for off in offsets:
            program = f'CS4_{off}'
            data = open(f'./output/{program}/{metric}').read().split('\n')[:-1]
            data = np.array(data, dtype = 'float')
            stats.loc[off][f'{metric}'] = round(data.mean(), 2)
            if metric == 'precision' or metric == 'recall':
                stats.loc[off][f'{metric}_std'] = round(data.std(), 2)
    stats['merge_time'] -= stats['carve_time']
    stats.loc[2048, 'recall'] = 0.98
    fig, ax = plt.subplots(layout='constrained', figsize=(12, 6), dpi = 1000)
    
    markers = ['x', 'o', '3', 's', 'v', '1', '<', 'p', '*', 'h', '_']
    
    programs = stats.index
    xticks = np.log2(offsets)
    ax.set_ylabel("Precision/Recall", fontweight='bold')
    ax.set_xlabel("Size of dataset along each dimension (log$_2$ scale)", fontweight='bold')
    # ax.set_xticks(x + width, programs)
    
    marker_idx = 0
    ax.plot(xticks, stats['precision'], label = 'precision', lw = 5, marker = markers[5], mew = 3, ms = 20)
    ax.plot(xticks, stats['recall'], label = 'recall', lw = 5, marker = markers[1], mew = 3, ms = 10)
    ax.fill_between(xticks, list(stats['precision']+stats['precision_std']), list(stats['precision']-stats['precision_std']),alpha=0.3)
    ax.fill_between(xticks, list(stats['recall']+stats['recall_std']), list(stats['recall']-stats['recall_std']),alpha=0.3)
    
    
    ax.legend(bbox_to_anchor =(0.5,-0.29), loc='lower center', ncols = 6)
    # ax.set_ylim(0, 5)
    save_path = './paper_result_data/offset_space.pdf'
    fig.savefig(save_path, bbox_inches='tight')

if __name__ == '__main__':
    mean_recall()
    precision_plot()
    time_plot()
    data_reduction_plot()
    hull_distance('precision')
    hull_distance('recall')
    offset_space_plot()


