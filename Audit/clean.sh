rm auditLib.so
./compile.sh
rm -rf ../AuditLog
rm ../Test/trial.txt
touch ../Test/trial.txt
cat ../Test/baseText.txt >> ../Test/trial.txt