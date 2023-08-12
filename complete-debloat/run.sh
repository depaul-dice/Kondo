echo "Installing python"
sudo apt-get -y install software-properties-common
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt-get -y update
sudo apt-get -y install python3.8
pip install -r requirements.txt
pip install numpy --user
sudo apt -y install clang lldb lld

# install gcc etc
sudo apt install -y build-essential

echo "\n\n##########################"
echo "##########Installing AFL######"
echo "\n\n##########################"
git clone https://github.com/google/AFL.git
cd AFL
sudo make
sudo make install
cd ..

echo "\n\n##########################"
echo "Compiling source programs"
echo "\n\n##########################"
make
# compile space variation programs
bash space_variation_exp/compile.sh

echo "\n\n##########################"
echo "\nCompiling AFL Programs"
echo "\n\n##########################"
bash AFL-experiments/compile.sh          # TODO: UNCOMMENT

mkdir -p logs
mkdir -p data

echo "\n\n##########################"
echo "\nRunning Kondo and BF"
echo "\n\n##########################"
python3 src/run.py

echo "\n\n##########################"
echo "\nSleeping to wait for AFL compilation"
echo "\n\n!!!!Please increase the sleep time if AFL compilation is turned on above. It usually takes a few hours."
echo "\n\n##########################"
sleep 60 # 1 min or until compilation of afl programs end

echo "\n\n##########################"
echo "\nRunning AFL"
echo "\n\n##########################"
bash AFL-experiments/fuzz.sh

echo "\n\n##########################"
echo "\nRunning offset space variation experiments"
echo "\n\n##########################"
python3 offset_space_exp.py

echo "\n\n##########################"
echo "\nComputing statistics from the generated data"
echo "\n\n##########################"
python3 compute_stats.py

echo "\n\n##########################"
echo "\nRunning Kondo with varied center hull distance threshold"
echo "\n\n##########################"
python3 vary_hull_thresh.py

echo "\n\n##########################"
echo "\nPlotting"
echo "\n\n##########################"
python3 plot_stats.py
                                                                                                                                                                                          71,1          Bot
