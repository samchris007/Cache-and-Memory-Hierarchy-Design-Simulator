import matplotlib.pyplot as plt
import numpy as np

# Adjust log2_size to match the length of miss rates
log2_size = np.arange(10, 14)  # Adjust this to match the 4-element arrays
# Sample data for cache sizes and miss rates based on associativity
mr_dir_map = [0.1935, 0.1477, 0.1002, 0.067]
ht_dir = [0.114797, 0.12909, 0.147005, 0.16383]
mr_2_way = [0.156, 0.1071, 0.0753, 0.0473]
ht_2_way = [0.140329, 0.161691, 0.181131, 0.194195]
mr_4_way = [0.1427, 0.0962, 0.0599, 0.0425]
ht_4_way = [0.14682, 0.195446, 0.185685, 0.211173]
mr_8_way = [0.1363, 0.0907, 0.0536, 0.0395]
ht_8_way = [0.155484, 0.180686, 0.189065, 0.212911]
mr_l2_1 = [0.1434, 0.1878, 0.2763, 0.3967]
mr_l2_2 = [0.1775, 0.2585, 0.3688, 0.5714]
mr_l2_4 = [0.1941, 0.2879, 0.4598, 0.6461]
mr_l2_8 = [0.2036, 0.3053, 0.5131, 0.6697]

ht_l1 = [0.14682, 0.154496, 0.185685, 0.211173]
ht_l2 = [ 0.254354, 0.288511, 0.341213]

mr_l1 = [0.1427, 0.0962, 0.0599, 0.0425]
mr_l2_16kb = [0.1941,0.2879, 0.4598, 0.6461]
mr_l2_32kb = [0.1840, 0.2728, 0.4381, 0.6181]
mr_l2_64kb = [0.1814, 0.2691, 0.4326, 0.6105]

mr_l1 =np.array(mr_l1)
mr_l2_16kb =np.array(mr_l2_16kb)
mr_l2_32kb =np.array(mr_l2_32kb)
mr_l2_64kb =np.array(mr_l2_64kb)

aat_16kb = ht_l1 + (mr_l1 * ( ht_l2[0]+ (mr_l2_16kb * 20.1)))
aat_32kb = ht_l1 + (mr_l1 * (ht_l2[1] + (mr_l2_32kb * 20.1)))
aat_64kb = ht_l1 + (mr_l1 * (ht_l2[2] + (mr_l2_64kb * 20.1)))

# Convert to numpy arrays for element-wise operations
mr_dir_map = np.array(mr_dir_map)
mr_2_way = np.array(mr_2_way)
mr_4_way = np.array(mr_4_way)
mr_8_way = np.array(mr_8_way)
mr_l2_1 = np.array(mr_l2_1)
mr_l2_2 = np.array(mr_l2_2)
mr_l2_4 = np.array(mr_l2_4)
mr_l2_8 = np.array(mr_l2_8)

# Compute AAT for each cache type
aat_dir = ht_dir + (mr_dir_map * (0.254354 + (mr_l2_1 * 20.1)))
aat_2 = ht_2_way + (mr_2_way * (0.254354 + (mr_l2_2 * 20.1)))
aat_4 = ht_4_way + (mr_4_way * (0.254354 + (mr_l2_4 * 20.1)))
aat_8 = ht_8_way + (mr_8_way * (0.254354 + (mr_l2_8 * 20.1)))

colors = [
    '#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', 
    '#7f7f7f', '#bcbd22', '#17becf', '#393b79', '#8c6d31', '#b5cf6b', '#e6550d', 
    '#637939', '#ff9896', '#98df8a', '#ffbb78', '#c49c94', '#f7b6d2', '#dbdb8d', 
    '#9edae5', '#c5b0d5', '#1b9e77', '#d95f02', '#7570b3', '#e7298a', '#66a61e', 
    '#e6ab02', '#a6761d'
]

# Plot the results
#plt.plot(log2_size, aat_dir, label='Direct-Mapped', marker='o')
plt.plot(log2_size, aat_16kb, label='L2 16KB', marker='o', color = colors[11])
plt.plot(log2_size, aat_32kb, label='L2 32KB', marker='o', color = colors[15])
plt.plot(log2_size, aat_64kb, label='L2 64KB', marker='o', color = colors[19])

# Labeling the plot
plt.xlabel('L1 log2(Cache Size)')
plt.ylabel('AAT')
plt.title('AAT vs L1 log2(Cache Size)')
plt.legend(title='L2 Cache Size')
plt.grid(True)
plt.show()

# Print the computed AAT values
print(aat_16kb)
print(aat_32kb)
print(aat_64kb)