cd AFL-experiments
cd CS1
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./CS1 > fuzz-log 2>&1 &
PID1a=$!
nohup python3 process_output.py CS1 > process_log 2>&1 &
PID1b=$!
cd ..

cd CS2
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./CS2 > fuzz-log 2>&1 &
PID2a=$!
nohup python3 process_output.py CS2 > process_log 2>&1 &
PID2b=$!
cd ..

cd CS3
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./CS3 > fuzz-log 2>&1 &
PID3a=$!
nohup python3 process_output.py CS3 > process_log 2>&1 &
PID3b=$!
cd ..

cd CS4
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./CS4 > fuzz-log 2>&1 &
PID4a=$!
nohup python3 process_output.py CS4 > process_log 2>&1 &
PID4b=$!
cd ..

cd CS5
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./CS5 > fuzz-log 2>&1 &
PID5a=$!
nohup python3 process_output.py CS5 > process_log 2>&1 &
PID5b=$!
cd ..

cd PRL2D
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./PRL2D > fuzz-log 2>&1 &
PID6a=$!
nohup python3 process_output.py PRL2D > process_log 2>&1 &
PID6b=$!
cd ..

cd LDC2D
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./LDC2D > fuzz-log 2>&1 &
PID7a=$!
nohup python3 process_output.py LDC2D > process_log 2>&1 &
PID7b=$!
cd ..

cd RDC2D
rm -rf output
rm -f inputs.txt inputs_used.txt output.txt fuzz-log explored_offsets process_log counts.txt
mkdir output
nohup afl-fuzz -m none -t 1000000 -i ./testcases -o ./output ./RDC2D > fuzz-log 2>&1 &
PID8a=$!
nohup python3 process_output.py RDC2D > process_log 2>&1 &
PID8b=$!
cd ..

sleep 3600
echo "Killing AFL processes"
kill -9 $PID1a
kill -9 $PID1b
kill -9 $PID2a
kill -9 $PID2b
kill -9 $PID3a
kill -9 $PID3b
kill -9 $PID4a
kill -9 $PID4b
kill -9 $PID5a
kill -9 $PID5b
kill -9 $PID6a
kill -9 $PID6b
kill -9 $PID7a
kill -9 $PID7b
kill -9 $PID8a
kill -9 $PID8b

cd ..