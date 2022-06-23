make CONFIG_ROMFS_FS=m -C /usr/src/linux-5.15.35 M=/usr/src/linux-5.15.35/fs/romfs modules > /dev/null 2>/dev/null
insmod /usr/src/linux-5.15.35/fs/romfs/romfs.ko hided_file_name=aa encrypted_file_name=bb exec_file_name=cc
mount -o loop /root/repo/CS353-2022-Spring/project4/test.img /mnt -t romfs
