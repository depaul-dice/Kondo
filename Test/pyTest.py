import h5py as h5

def openFile(path, mode):
    return h5.File(path, mode)

def openDataset(file, datasetName):
    return file[datasetName]

def main():
    fileObj = openFile("/home/rohan/newDev/Kondo/Test/Data/Mat1000x1000Comp.h5","r")
    dataset = openDataset(fileObj, "matrix")
    print(dataset[0:100, 0:100])
    #0:100,0:100
    #950:1000,400:700 
    #print(dataset[90,95])
    #print(dataset[45,67])
    #print(dataset[0,4])
if __name__ == "__main__":
    main()
