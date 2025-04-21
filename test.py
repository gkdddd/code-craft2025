import numpy as np
import pandas as pd

# 定义文件路径
file_path = 'd:\\workspace\\codeCraft2025_semifinals\\tmp.txt'
output_path = 'd:\\workspace\\codeCraft2025_semifinals\\tmp.csv'

# 初始化一个空的列表来存储数据
data = []

# 打开文件并读取数据
with open(file_path, 'r') as file:
    for line in file:
        # 将每行数据按空格分割并转换为整数
        row = list(map(int, line.split()))
        data.append(row)

# 将数据转换为NumPy数组
data_array = np.array(data)

# 将NumPy数组转换为DataFrame
df = pd.DataFrame(data_array)

# 将DataFrame存储到CSV文件中
df.to_csv(output_path, index=False, header=False)

# 打印确认信息
print(f"数据已存储到 {output_path}")