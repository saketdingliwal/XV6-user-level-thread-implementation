make clean
make

./test_assig1.sh assig1_1|grep -o 'sys_[^ ]* [[:digit:]]*' > res_assig1_1
./test_assig1.sh assig1_3|grep -i Sum > res_assig1_3
./test_assig1.sh assig1_5|grep -i pid > res_assig1_5

total_test=0

for t in 1 3 5
do
	echo -n "Test #${t}: "

	if diff -Z <(sort out_assig1_$t) <(sort res_assig1_$t) > /dev/null
	then
		echo -e "\e[0;32mPASS\e[0m"
		((total_test++))
	else
		echo -e "\e[0;31mFAIL\e[0m"
	fi
done
echo "$total_test" test cases passed
