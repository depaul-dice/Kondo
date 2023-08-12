import numpy as np
import time
from matplotlib import pyplot as plt
import signal
import sys

program = sys.argv[1]
truth = np.load(f'../truth/{program}_truth.npy')
truth_size = truth.shape[0]

offsets = np.zeros((64,64,64))
start = time.time()
def sigterm_handler(signal, frame):
    # write to  explored_offsets
    # with open('./explored_offsets', 'w') as f:
    #     for i in range(64):
    #         for j in range(64):
    #             if offsets[i][j] == 1:
    #                 f.write(f'{i} {j}\n')
    # with open('./explored_offsets_visual', 'w') as f:
    #     for i in range(64):
    #         for j in range(64):
    #             if offsets[i][j] == 1:
    #                 f.write(f'1 ')
    #             else:
    #                 f.write('0 ')
    #         f.write('\n')
    with open('./counts.txt', 'a') as f2:
        f2.write(f'{(time.time() - start)}: {(offsets==1).sum()}, Recall: {(offsets==1).sum()/(truth_size)}\n')
            
    # plt.figure(0)
    # plt.xlabel('seconds elapsed')
    # plt.ylabel('total number of offsets discovered')
    # plt.scatter(x,y)
    # plt.savefig('./exploration.png')
    # plt.close()
    # plt.figure(1)
    # plt.title('visual representation of offsets discovered')
    # plt.imshow(offsets)
    # plt.savefig('./heatmap.png')
    # plt.close()
    
# Register the signal handler function for SIGTERM
signal.signal(signal.SIGTERM, sigterm_handler)
signal.signal(signal.SIGINT, sigterm_handler)


x = []
y = []
start = time.time()
count = 0
count2 = 0


with open('./output.txt','r') as f:
    with open('./counts.txt', 'a') as f2:
        while True:
            count2+=1
            a = f.readline()
            while not a:
                time.sleep(1)
                a = f.readline()

            # print(f'a: {a}')
            if a =='\n':
                a = f.readline()

            try:
                a = a.split()
                # print(a)
                offset = (int(a[0]), int(a[1]), int(a[2]))
                if offsets[offset] != 1:
                    count+=1
                    offsets[offset] = 1
                    y.append(count)
                    x.append(time.time()-start)
                    
                    
                    # if count%10 == 0:
                    #     plt.figure(0)
                    #     plt.xlabel('seconds elapsed')
                    #     plt.ylabel('total number of offsets discovered')
                    #     plt.scatter(x,y)
                    #     plt.savefig('./exploration.png')
                    #     plt.close()
                    #     plt.figure(1)
                    #     plt.title('visual representation of offsets discovered')
                    #     plt.imshow(offsets)
                    #     plt.savefig('./heatmap.png')
                    #     plt.close()
            except:
                continue
            finally:
                if count2%800 == 0:
                    # print(count2)
                    f2.write(f'{(time.time() - start)}: {(offsets==1).sum()}, Recall: {(offsets==1).sum()/(truth_size)}\n')
                if count2%5000 == 0:
                    f2.flush()
        