#!/usr/bin/env python
# coding: utf-8

# In[28]:


import cc3d
import netCDF4
import h5py as h5
import glob
import numpy
import sys
import datetime
import pickle
import time
import pandas


# In[29]:
data_dir = '/home/rohan/newDev/Kondo/Test/Data/imerg2022Small/'
out_data_dir = '/home/rohan/newDev/Kondo/Test/featureDB' # local output directory for now 
#data_dir = '/shared/flexfs/bayesics/tablespace/xcal/imerg' #'/shared/IMERG-samples/' #'/shared/STARE/xcal'

# either the imerg or the pickles 
# the pickle is the cube of the original imerg

# INPUT IMERG: 3600*1800, each file contains this grid, corresponding to 30 miuntes time slice -> value of percipation 
# 1. First podding: chunk into trixels
# 2. parallel to this: create the features. load data into cube, and find components


# # Loading the data

# In[30]:


def load_imerg(file_path, variable_name):
    h5File = h5.File(file_path, "r")
    print(h5File.keys())
    data = h5File["Grid"][variable_name][:][0].T[::-1]
    header = {'DOI': '10.5067/GPM/IMERG/3B-HH-L/06', 'DOIauthority': 'http://dx.doi.org/', 'DOIshortName': '3IMERGHH_LATE', 'AlgorithmID': '3IMERGHH', 'AlgorithmVersion': '3IMERGH_6.3', 'FileName': '3B-HHR-L.MS.MRG.3IMERG.20220522-S020000-E022959.0120.V06C.RT-H5', 'SatelliteName': 'MULTI', 'InstrumentName': 'MERGED', 'GenerationDateTime': '2022-05-22T17:32:23.000Z', 'StartGranuleDateTime': '2022-05-22T02:00:00.000Z', 'StopGranuleDateTime': '2022-05-22T02:29:59.999Z', 'GranuleNumber': '', 'NumberOfSwaths': '0', 'NumberOfGrids': '1', 'GranuleStart': '', 'TimeInterval': 'HALF_HOUR', 'ProcessingSystem': 'PPS-NRT', 'ProductVersion': '06C', 'EmptyGranule': 'NOT_EMPTY', 'MissingData': ''}
    # netcdf = netCDF4.Dataset(file_path, 'r', format='NETCDF4')
    # data = netcdf.groups['Grid'][variable_name][:][0].T[::-1]
    # file_header = netcdf.FileHeader.split(';\n')
    # file_header.remove('')
    # header = {r.split('=')[0]: r.split('=')[1] for r in file_header}
    return data, header


# In[31]:


# file_paths = sorted(glob.glob(data_dir + 'imerg2022/3B-HHR*'))
file_paths = [
    "/home/rohan/newDev/Kondo/Test/Data/imerg2022Small/imerg2022/3B-HHR-L.MS.MRG.3IMERG.20220607-S113000-E115959.0690.V06C.HDF5",
    "/home/rohan/newDev/Kondo/Test/Data/imerg2022Small/imerg2022/3B-HHR-L.MS.MRG.3IMERG.20220623-S233000-E235959.1410.V06C.HDF5"
]
print(file_paths)
variable_name = 'precipitationCal'

stack = []
headers = []
timestamps = []
i = 0
for file_path in file_paths:
    if(i==2):
        break
    i+=1
    print('{}'.format(len(file_paths)-len(stack)), end='\r')
    sys.stdout.flush()
    data, header = load_imerg(file_path, variable_name)
    stack.append(data)
    headers.append(header)
    timestamps.append(header['StartGranuleDateTime'])
    
data = numpy.array(stack)

timestamps_dt = [datetime.datetime.strptime(date, '%Y-%m-%dT%H:%M:%S.000Z') for date in timestamps]
timestamps_np = numpy.array(timestamps_dt, dtype='datetime64[m]')


# In[32]:


#data = numpy.flip(data, 1)


# In[33]:


### with open('{}/pickles/data.pickle'.format(out_data_dir), 'wb') as f:
###    pickle.dump(data, f)
    
with open('{}/pickles/timestamps.pickle'.format(out_data_dir), 'wb+') as f:
    pickle.dump(timestamps_dt, f)


# In[34]:


### with open('{}/pickles/data.pickle'.format(out_data_dir), 'rb') as f:
###    imerg = pickle.load(f)


# # Thresholding and CCLs

# In[18]:


### data = imerg


# In[19]:


thresh = 0.5
data[data<thresh] = 0
data[data>=thresh] = 1

connectivity = 6 
min_voxels = 1000


# In[20]:


start = time.time()
labels, N = cc3d.connected_components(data, delta=0,connectivity=connectivity, return_N=True)
print(time.time() - start)


# In[21]:


labels


# In[22]:


start = time.time()
cc3d.dust(labels, threshold=min_voxels, connectivity=connectivity, in_place=True)
print(time.time() - start)


# In[23]:


labels


# In[24]:


label_names = numpy.unique(labels[labels>0])
label_names.shape


# In[25]:


start = time.time()
###largest_10, N = cc3d.largest_k(labels, k=10, connectivity=connectivity, delta=0, return_N=True)
##largest_100, N = cc3d.largest_k(labels, k=100, connectivity=connectivity, delta=0, return_N=True)
print(time.time() - start)


# # Pickle

# In[27]:



### with open('{}/pickles/largest_10.pickle'.format(output_data_dir), 'wb') as f:
###    pickle.dump(largest_10, f)
    
### with open('{}/pickles/largest_100.pickle'.format(output_data_dir), 'wb') as f:
###    pickle.dump(largest_100, f)

with open('{}/pickles/labels.pickle'.format(out_data_dir), 'wb+') as f:
    pickle.dump(labels, f)


