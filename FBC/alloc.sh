IFS=$(echo -en "\n\b") #Ϊ�˴����ļ����еĿո�TAB
cd /yl/test/try #��try���ļ�����copy��
j=0
# for i in `ls -1`
# do
	# echo "/test/try/"$i >> /yl/file.txt
	# touch "/test/copy/"$i
    # folder_list[j]="imap /copy/"${i}
    # j=`expr $j + 1`
# done

for dir in `find . -type d`
do 
	dir_r="/yl/test/copy"`echo $dir|sed 's/.//'`
	mkdir $dir_r
done
 #try git
 
	
for file in `find . -type f`
do  
	echo "/yl/test/copy"`echo $file|sed 's/.//'` >>/home/yl/file.txt
	touch "/yl/test/copy"`echo $file|sed 's/.//'`
	folder_list[j]="imap /copy"`echo $file|sed 's/.//'`
	j=`expr $j + 1`
done

cd /home/yl

# for ((i=1;i<j;i++))
# do
  # echo $i
  # echo ${folder_list[i]}
# done

dumpe2fs /dev/sdb3 | grep bitmap | awk '{print $4}' | awk -F ',' '{print $1}' > bitmap.txt
	
for ((i=0;i<j;i++))
do
	debugfs -R ${folder_list[i]} /dev/sdb3| grep located | awk '{print $4"\t"$6}' |awk -F ',' '{print $1"\t"$2}'>> test.txt
done

