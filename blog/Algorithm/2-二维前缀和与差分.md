### 📌 二维区间和计算公式（子矩阵和）

设二维前缀和数组为：

$$
\text{prefix}_{i,j} = \sum_{x=1}^{i} \sum_{y=1}^{j} a_{x,y}
$$

要计算子矩阵 \([x_1, y_1]\) 到 \([x_2, y_2]\)（闭区间）的所有元素之和：

$$
\text{sum}_{(x_1,y_1),(x_2,y_2)} =
\text{prefix}_{x_2, y_2}
- \text{prefix}_{x_1-1, y_2}
- \text{prefix}_{x_2, y_1-1}
+ \text{prefix}_{x_1-1, y_1-1}
$$

> 其中，若某项下标为 0，可以视为 0，例如

$$
(\text{prefix}_{0,j} = 0)，(\text{prefix}_{i,0} = 0)。
$$



