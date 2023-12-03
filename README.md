make

sudo insmod echo.ko

sudo mknod /dev/echodev c [major] 0
ou
sudo mknod /dev/echodev c $(grep echodev /proc/devices | awk '{print $1}') 0

echo "hello, world!" | sudo tee /dev/echodev

dmesg | tail

