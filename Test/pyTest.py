import h5py as h5
file = h5.File("/home/rohan/Documents/Kondo/Test/trial.h5")
dset = file["test"]
print(dset[4,7])
