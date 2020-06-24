if [ $# == 7 ]; then
	input_val=$1
	input_dir=$2
	output_dir=$3
	m_method=$4
	s_method=$5
	ip=$6
	m2s_method=$7
#echo ""
#echo ${input_val} ${input_dir} ${output_dir} ${m_method} ${s_method} 127.0.0.1 ${m2s_method}

fi

files=$(ls $input_dir)
file_list="./input/file_list/${input_dir##*/}_list"
#echo ${file_list}

if [ ! -d "./output" ]; then
	mkdir "./output"
fi
if [ ! -d "./input/file_list" ]; then
	mkdir "./input/file_list"
fi
if [ ! -d ${output_dir} ]; then
	mkdir ${output_dir}
fi
if [ -f ${file_list} ]; then
	rm ${file_list}
fi

for filename in $files
do
	echo ${filename} >> ${file_list}
done

master_log=${input_val}"_master_"${m2s_method}\.txt
slave_log=${input_val}"_slave_"${m2s_method}\.txt

# echo method: ${m2s_method}  master: ${m_method}  slave: ${s_method}  
./user_program/master ${input_dir}/ ${m_method} < ${file_list} &\
./user_program/slave ${output_dir}/ ${s_method} ${ip} < ${file_list}
