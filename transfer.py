def dat_to_cpp(filename, var_name="tag_request_busy_rate"):
    with open(filename, 'r') as f:
        lines = f.readlines()

    dims = lines[0].strip().split()
    x, y = int(dims[0]), int(dims[1])
    data_lines = lines[1:]

    if len(data_lines) != x:
        raise ValueError("行数不匹配")

    cpp_code = f"std::vector<std::vector<double>> {var_name} = {{\n"

    for line in data_lines:
        values = [v for v in line.strip().split()]
        if len(values) != y:
            raise ValueError("列数不匹配")
        cpp_code += "    { " + ", ".join(values) + " },\n"

    cpp_code += "};"
    return cpp_code


# 用法示例
if __name__ == "__main__":
    cpp_text = dat_to_cpp("tag_request_busy_rate.dat")  # 替换成你的文件名
    print(cpp_text)