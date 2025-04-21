import pandas as pd

# 读取当前目录下的 CSV 文件
file_path = "tmp.csv"

# 读取 CSV 文件，没有表头
df = pd.read_csv(file_path, header=None)

# 获取每列元素的排序索引
sorted_indices = df.apply(lambda col: col.argsort(), axis=0)

# 将索引值从 0 开始改为从 1 开始
sorted_indices += 1

print(sorted_indices)

# 输出到 C++ 格式的文件
output_file = "rank_output_column_first_sorted_indices.hpp"

with open(output_file, "w") as f:
    f.write("#ifndef RANK_OUTPUT_ARRAY_HPP\n")
    f.write("#define RANK_OUTPUT_ARRAY_HPP\n\n")
    f.write("#include <array>\n\n")
    f.write("const std::array<std::array<int, " + str(sorted_indices.shape[1]) + ">, " + str(sorted_indices.shape[0]) + "> rank = {{\n")
    
    for i in range(sorted_indices.shape[0]):  # 遍历行
        row = ", ".join(map(str, sorted_indices.iloc[i].tolist()))
        f.write(f"    {{{row}}},\n")
    
    f.write("}};\n\n")
    f.write("#endif // RANK_OUTPUT_ARRAY_HPP\n")

print(f"表格内容已转换为 C++ std::array 格式并写入到 {output_file}")