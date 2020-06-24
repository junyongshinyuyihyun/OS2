#array_input_dir=([0]=1 [1]=2 [2]=3 [3]=4)

array_input_dir=([0]=1 [1]=4)

array_master_method=([1]="fcntl" [0]="mmap")
array_slave_method=([1]="fcntl" [0]="mmap")


for a in {0,1}
	do
	input_val=${array_input_dir[a]}                
	input_dir="./input/sample_input_"${input_val}
	output_dir_tmp="./output/sample_output_"${input_val}"_"
echo ${input_val}



# for a in {4}
# 	do
# 	input_val=${a}                
# 	input_dir="./input/sample_input_"${a}
# 	output_dir_tmp="./output/sample_output_"${a}"_"
# echo ${a}
	for b in {0,1}
		do
		m_method=${array_master_method[b]}
		for c in {0,1}
			do
			s_method=${array_slave_method[c]}
			output_dir=${output_dir_tmp}${array_slave_method[c]}
			if [ ${m_method} == "fcntl" ] && [ ${s_method} == "fcntl" ]; then
				m2s_method="f2f"
			elif [ ${m_method} == "fcntl" ] && [ ${s_method} == "mmap" ]; then
				m2s_method="f2m"
			elif [ ${m_method} == "mmap" ] && [ ${s_method} == "fcntl" ]; then
				m2s_method="m2f"
			elif [ ${m_method} == "mmap" ] && [ ${s_method} == "mmap" ]; then
				m2s_method="m2m"
			else
				m2s_method="error"
			fi

			for i in {1..100}
				do
				echo "input_val: "${input_val}" iterate: "${i}" method: "${m2s_method}
				echo ""
				echo ${input_val} ${input_dir} ${output_dir} ${m_method} ${s_method} 127.0.0.1 ${m2s_method}
				bash ./run.sh ${input_val} ${input_dir} ${output_dir} ${m_method} ${s_method} 127.0.0.1 ${m2s_method}
				echo ""
#${m2s_method}
#echo ""
#diff_filename=${a}_${method}_diff\.txt
#echo ${diff_filename}
#echo ${input_dir} ${output_dir} ${array_input_dir[a]} ${array_input_method[b]} ${array_output_method[c]} 127.0.0.1
#bash ./run.sh ${input_val} ${input_dir} ${output_dir} ${array_input_method[b]} ${array_output_method[c]} 127.0.0.1
#bash ./diff.sh ${input_dir} ${output_dir} |tee -a ${input_val}_diff_${method}\.txt
				done
			done
		done
	done

# bash ./diff.sh ${input_dir} ${output_dir} | ${input_val}_diff_${method}\.txt
