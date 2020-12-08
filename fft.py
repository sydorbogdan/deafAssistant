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


import numpy as np
import matplotlib.pyplot as plt

f = open("PCM.txt")


def calculate_db_coefficient(data):
    data = np.array(data, dtype=np.int64)
    suma = 0
    for i in data:
        suma += i * i
    sample_length = data.shape[0]
    rms = suma / (sample_length * 32768)
    print(10 * np.log10(rms))
    return 10 * np.log10(rms)


def calculate_vibro(freq, PSD, data):
    vibros = [200, 800, 3500, 5500]
    sums = np.array([0, 0, 0, 0])
    j = 0
    for i in range(len(freq)):
        sums[j] += PSD[i] // len(freq)
        if freq[i] > vibros[j]:
            j += 1
        if j == 4:
            break
    sums[0] *= 13.5
    sums[1] *= 4.5
    sums[2] *= 1
    sums[3] *= 1.35
    # np.random.seed(42)
    # x = np.random.normal(size=1000)

    # print(sums)
    return sums / sum(sums) * calculate_db_coefficient(data)


def fft(data):
    dt = 1 / len(data)
    t = np.arange(0, 1, dt)
    data = np.array(data)
    # print(data)
    n = len(t)
    fhat = np.fft.fft(data, n)
    PSD = fhat * np.conj(fhat) / n
    freq = (1 / (dt * n)) * np.arange(n)
    L = np.arange(1, np.floor(n / 2), dtype=int)
    fig, axs = plt.subplots(2, 1)


    plt.sca(axs[1])
    freq = 65 * freq
    plt.plot(freq[L], PSD
    [L], color="c", label="5322")
    plt.xlim(freq[L[0]], freq[L[-1]])
    plt.legend()
    PSD[0] = 0
    # PSD[500]=0
    # plt.plot(freq[200:920], PSD[200:920], color="c", label="h")
    plt.sca(axs[0])
    # plt.plot(t, data, color='c', label = 'Noisy')
    # plt.xlim(t[0], t[-1])

    vibro_data = calculate_vibro(freq, PSD, data)
    print(vibro_data)
    x = np.arange(4)
    plt.bar(x, height=vibro_data)
    plt.ylabel('Freq')
    plt.xlabel('Data')
    plt.legend()
    plt.show()



lst = f.readline()
