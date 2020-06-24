if [ $# == 2 ]; then
	input_dir=$1
	output_dir=$2
else
	input_dir="./input/sample_input_1"
	output_dir="./output/sample_output_1_mmap"
fi
files=$(ls $input_dir)
file_list="./input/file_list/${input_dir##*/}"


for filename in $files
do
	diff ${input_dir}/${filename} ${output_dir}/received_${filename#*_}
done

