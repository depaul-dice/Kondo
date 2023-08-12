echo -e "Installing python"
sudo apt-get -y install software-properties-common
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt-get -y update
sudo apt-get -y install python3.8
pip install -r requirements.txt
pip install numpy --user
sudo apt -y install clang lldb lld

# install gcc etc
sudo apt install -y build-essential

echo -e "\n\n##########################"
echo -e "##########Installing AFL######"
echo -e "##########################\n\n"
git clone https://github.com/google/AFL.git
cd AFL
sudo make
sudo make install
cd ..

echo -e "\n\n##########################"
echo -e "Compiling source programs"
echo -e "##########################\n\n"
make
# compile space variation programs
bash space_variation_exp/compile.sh

echo -e "\n\n##########################"
echo -e "\nCompiling AFL Programs"
echo -e "##########################\n\n"
bash AFL-experiments/compile.sh          # TODO: UNCOMMENT

mkdir -p logs
mkdir -p data

echo -e "\n\n##########################"
echo -e "\nRunning Kondo and BF"
echo -e "##########################\n\n"
python3 src/run.py

echo -e "\n\n##########################"
echo -e "\nSleeping to wait for AFL compilation"
echo -e "!!!!Please increase the sleep time if AFL compilation is turned on above. It usually takes a few hours."
echo -e "##########################\n\n"
sleep 60 # 1 min or until compilation of afl programs end

echo -e "\n\n##########################"
echo -e "\nRunning AFL"
echo -e "##########################\n\n"
bash AFL-experiments/fuzz.sh

echo -e "\n\n##########################"
echo -e "\nRunning offset space variation experiments"
echo -e "##########################\n\n"
python3 offset_space_exp.py

echo -e "\n\n##########################"
echo -e "\nComputing statistics from the generated data"
echo -e "##########################\n\n"
python3 compute_stats.py

echo -e "\n\n##########################"
echo -e "\nRunning Kondo with varied center hull distance threshold"
echo -e "##########################\n\n"
python3 vary_hull_thresh.py

echo -e "\n\n##########################"
echo -e "\nPlotting"
echo -e "##########################\n\n"
python3 plot_stats.py
                                                                                                                                                                                          71,1          Bot
