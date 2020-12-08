# import matplotlib.pyplot as plt
# import serial
# import time
#
# ser = serial.Serial(
#     port='/dev/ttyACM0', \
#     baudrate=9600, \
#     parity=serial.PARITY_NONE, \
#     stopbits=serial.STOPBITS_ONE, \
#     bytesize=serial.EIGHTBITS, \
#     timeout=0)
#
# print("connected to: " + ser.portstr)
#
# # this will store the line
# counter = 0
# data = []
# strNum = ""
# f = open("PCM.txt", "w")
# file_samples = 0
# while True:
#     for c in ser.read():
#         strNum += chr(c)  # convert from ANSII
#         break
#     counter += 1
#
#     # print(strNum)
#     try:
#         # print(len(list(map(int, strNum.split(',')))))
#         if len(list(map(int, strNum.split(',')))) == 64:
#             # data.append()
#             # fig, ax = plt.subplots()
#             f.write(strNum + "\n")
#             file_samples += 1
#             # print(len(list(map(int, strNum.split(', ')))))
#             # ax.plot(list(map(int, strNum.split(', '))), '-')
#             # plt.show()
#             print(file_samples)
#             if file_samples == 50:
#                 f.close()
#                 break
#             strNum = ""
#     except:
#         pass
#     counter = 0
#
# ser.close()
# import numpy as np
# import matplotlib.pyplot as plt
# f = open("PCM.txt")
# dt = 1 / 3200
# t = np.arange(0, 1, dt)
# data = []
# for i in f.readlines():
#     data += list(map(int, i.split(',')))
# data = np.array(data[:len(data)])
# # print(data)
# n = len(t)
# fhat = np.fft.fft(data, n)
# PSD = fhat*np.conj(fhat) / n
# freq =(1/(dt*n))*np.arange(n)
# L = np.arange(1, np.floor(n/2), dtype=int)
# fig, axs = plt.subplots(2, 1)
# # plt.sca(axs[0])
# # plt.plot(t, data, color='c', label = 'Noisy')
# # plt.xlim(t[0], t[-1])
# # plt.legend()
#
#
# plt.sca(axs[1])
# plt.plot(10*freq[L], PSD[L], color="c", label="5322")
# plt.xlim(10*freq[L[0]], 10*freq[L[-1]])
# plt.legend()
# # PSD[500]=0
# # plt.plot(freq[200:920], PSD[200:920], color="c", label="h")
# plt.show()
#
#
#
import matplotlib.pyplot as plt
import serial
import time
import fft

ser = serial.Serial(
    port='/dev/ttyACM0', \
    baudrate=9600, \
    parity=serial.PARITY_NONE, \
    stopbits=serial.STOPBITS_ONE, \
    bytesize=serial.EIGHTBITS, \
    timeout=0)

print("connected to: " + ser.portstr)

# this will store the line
counter = 0
data = []
strNum = ""
f = open("PCM.txt", "w")
file_samples = 0
while True:
    for c in ser.read():
        strNum += chr(c)  # convert from ANSII
        break
    counter += 1

    # print(strNum)
    try:
        if len(list(map(int, strNum.split(',')))) == 320:
            # data.append()
            # fig, ax = plt.subplots()
            print("HERE")
            f.write(strNum + "\n")
            file_samples += 1
            # print(len(list(map(int, strNum.split(', ')))))
            # ax.plot(list(map(int, strNum.split(', '))), '-')
            # plt.show()
            fft.fft(list(map(int, strNum.split(','))))
            if file_samples == 50:
                f.close()
                break
            strNum = ""
    except ValueError:
        pass
    counter = 0

ser.close()
