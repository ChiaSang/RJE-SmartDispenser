import serial
import time

serialPort = '/dev/cu.wchusbserial1440'
# serialPort = '/dev/cu.SLAB_USBtoUART'
baudrate = 115200
databit = 8
parity = 'N'
stopbit = 1

if __name__ == '__main__':
    s = serial.Serial(port=serialPort,
                      baudrate=baudrate,
                      bytesize=databit,
                      parity=parity,
                      stopbits=stopbit)
    switch_state = 'false'

    while True:
        s.write(b'get_down\r')
        res = s.read_all().decode('utf-8')
        # print("接收: ", res)
        if (res == 'down get_properties 2 1\r'):
            print("Yes!", res)
            s.write(b'result 2 1 0 0\r')
            print(s.read_all().decode('utf-8'))
        elif (res == 'down get_properties 2 2\r'):
            print("Yes!", res)
            onoff_str = 'result 2 2 0 ' + '"' + switch_state + '"' + '\r'
            s.write(bytes(onoff_str, 'ascii'))
            print(s.read_all().decode('utf-8'))
        elif ('down set_properties 2 2' in res):
            print("Yes!", res)
            switch_state = str(res.split(" ")[-1]).replace('\r', '')
            onoff_str = 'result 2 2 ' + '"' + switch_state + '"' + '\r'
            properties_changed = 'properties_changed 2 2 ' + '"' + switch_state + '"' + '\r'
            s.write(bytes(onoff_str, 'ascii'))
            s.write(bytes(properties_changed, 'ascii'))
            print(s.read_all().decode('utf-8'))
        time.sleep(0.2)
