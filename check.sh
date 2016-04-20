for testcase in open_testcase
do
	for var in ~/Archi2016_Project2/archiTA/testcase/${testcase}/*
	do
		cp ${var}/iimage.bin ./
		cp ${var}/dimage.bin ./
		~/Archi2016_Project2/pipeline/simulator/pipeline
		diff snapshot.rpt ${var}/snapshot.rpt
		diff error_dump.rpt ${var}/error_dump.rpt
		echo Test ${var} end
	done
done

rm *.bin *.rpt
