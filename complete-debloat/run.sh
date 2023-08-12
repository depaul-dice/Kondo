echo "Installing python"
sudo apt-get -y install software-properties-common
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt-get -y update
sudo apt-get -y install python3.8
pip install -r requirements.txt

# install gcc etc
sudo apt install -y build-essential

install AFL
git clone https://github.com/google/AFL.git
cd AFL 
make 
make install
cd ..

echo "Compiling source programs"
make
# compile space variation programs
bash space_variation_exp/compile.sh

echo "\nCompiling AFL Programs"
echo "\nUsing pre compiled binaries for AFL as compilation is slow. If compilation is desired, please\
the following line. llvm would need to be installed. Please see https://github.com/google/AFL/blob/master/docs/INSTALL"
# bash AFL-experiments/compile.sh          # TODO: UNCOMMENT

mkdir -p logs
mkdir -p data

echo "\nRunning Kondo and BF"
python3 src/run.py

echo "\nSleeping to wait for AFL compilation"
echo "\n\n!!!!Please increase the sleep time if AFL compilation is turned on above. It usually takes a few hours."
sleep 60 # 1 min or until compilation of afl programs end

echo "\nRunning AFL"
bash AFL-experiments/fuzz.sh

echo "\nRunning offset space variation experiments"
python3 offset_space_exp.py

echo "\nComputing statistics from the generated data"
python3 compute_stats.py

echo "\nRunning Kondo with varied center hull distance threshold"
python3 vary_hull_thresh.py

echo "\nPlotting"
python3 plot_stats.py
