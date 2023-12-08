## Setup Instructions

### Dependencies
- Install [hdf5] from source, version 1.12.2
  
   `
   cd <unzipped hdf5 directory>
   `
   
   `
   CFLAGS=-g ./configure --prefix=/usr/local/hdf5
   `
   
   `
   sudo make
   `
   
   `
   sudo make install
   `
- Install python3.9 using
     
   `
   sudo add-apt-repository ppa:deadsnakes/ppa
   `
   
   `
   sudo apt-get install python3.9-dev
   `
   
   `
   sudo apt-get install python3.9-distutils
   `
- Install [h5py] 3.8.0 from source

   `
   export HDF5_DIR=/usr/local/hdf5/
   `
   
   `
   cd <unzipped h5py directory>
   `
   
   `
   python3.9 -m pip install -v .
   `
  
Visually inspect and ensure that the configured h5py has the following configuration



         Summary of the h5py configuration
      
          HDF5 include dirs: [
          "/usr/local/hdf5/include"
        ]
          HDF5 library dirs: [
          "/usr/local/hdf5/lib"
        ]
               HDF5 Version: (1, 12, 2)
                MPI Enabled: False
           ROS3 VFD Enabled: False
         DIRECT VFD Enabled: False
           Rebuild Required: True
             MS-MPI Enabled: False
        MS-MPI include dirs: []
        MS-MPI library dirs: []


The above will be in the output of the install command

- Install parse
`
python3.9 -m pip install parse
`
- Add WhiteList and BlackList to `Kondo/Test`
WhiteList.txt and BlackList.txt format

`
<num of entries>
entry1_absolute_path
entry2_absolute_path
`

### Compilation
- Compile Audit
`
cd Kondo/Audit
./compile.sh
`
- Compile RepeatLib
`
cd Kondo/ReExecute
./compile.sh
`
### Usage
#### Audit
`
LD_PRELOAD=<path to auditLib.so> <executable>
`

example:
`
LD_PRELOAD=../Audit/auditLib.so python3.9 pyTest.py
`
#### Data Store Creation
`
cd Kondo/DataStoreCreator
python3.9 createDataStore.py
`

#### ReExecution
`
LD_PRELOAD=<path to repeatLib.so> <executable>
`

example:
`
LD_PRELOAD=../ReExecute/repeatLib.so python3.9 pyTest.py
`

[hdf5]: https://www.hdfgroup.org/packages/hdf5-1122-source/
[h5py]: https://github.com/h5py/h5py/releases/tag/3.8.0
