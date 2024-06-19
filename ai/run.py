from cv2 import dnn
import numpy as np
import matplotlib.pyplot as plt

net = dnn.readNetFromONNX("mouse.onnx")
matblob = np.array([[242, 263]])
net.setInput(matblob)

print('input = {}'.format(matblob))
output = net.forward()
print('output = {}'.format(output))
points = output[0]

# 将数据分解为x和y坐标
x = points[:, 0].tolist()
y = points[:, 1].tolist()

# 根据点的索引生成颜色数组
colors = [i for i in range(len(points))]

# 创建散点图，其中c参数控制每个点的颜色
plt.scatter(x, y, c=colors, cmap='viridis')

# 添加标题和标签
plt.title('Scatter Plot with Color Gradient')
plt.xlabel('X Axis')
plt.ylabel('Y Axis')

# 设置坐标轴的刻度
plt.xticks(np.arange(0, 600, 15))
plt.yticks(np.arange(0, 600, 15))

# 显示颜色条
plt.colorbar()

# 显示图表
plt.show()
