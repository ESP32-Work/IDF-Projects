. /home/ibrahim/esp/v5.2.2/esp-idf/export.sh 

idf.py create-project -p . LED_Blink

idf.py build

idf.py -p /dev/ttyUSB0 flash

idf.py monitor