#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <random>
#include <ctime>
#include <cctype>
#include <sstream>
#include <limits>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include <utility>

//Ide修改

/**
 * 文本图分析程序
 * 
 * 本程序从文件中读取文本，根据单词相联关系构建有向图，
 * 并提供以下分析功能：
 * - 显示有向图结构
 * - 查找桥接词
 * - 根据桥接词生成新文本
 * - 查找单词之间的最短路径
 * - 计算PageRank值
 * - 在图上执行随机游走
 * 
 * 包含所有可选功能：
 * - 将有向图可视化并保存为图形文件
 * - 计算并显示所有最短路径
 * - 计算单词到所有其他单词的最短路径
 * - 使用TF-IDF优化PageRank初始值
 */
class DirectedGraph {
private:
    // 邻接表：源单词 -> (目标单词, 出现频率)
    std::map<std::string, std::map<std::string, int>> adjacencyMap;
    std::set<std::string> vertices; // 存储所有顶点（单词）

public:
    DirectedGraph() {}

    // 向图中添加顶点
    void addVertex(const std::string& vertex) {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        vertices.insert(lowerVertex);
        if (adjacencyMap.find(lowerVertex) == adjacencyMap.end()) {
            adjacencyMap[lowerVertex] = std::map<std::string, int>();
        }
    }

    // 向图中添加边，权重为1，如果边已存在则增加权重
    void addEdge(const std::string& source, const std::string& target) {
        std::string lowerSource = source;
        std::string lowerTarget = target;
        std::transform(lowerSource.begin(), lowerSource.end(), lowerSource.begin(), ::tolower);
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        // 如果顶点不存在则添加
        addVertex(lowerSource);
        addVertex(lowerTarget);
        
        // 添加或增加边的权重
        adjacencyMap[lowerSource][lowerTarget]++;
    }

    // 获取图中所有顶点
    const std::set<std::string>& getVertices() const {
        return vertices;
    }

    // 获取从source到target的边的权重
    int getEdgeWeight(const std::string& source, const std::string& target) const {
        std::string lowerSource = source;
        std::string lowerTarget = target;
        std::transform(lowerSource.begin(), lowerSource.end(), lowerSource.begin(), ::tolower);
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        if (adjacencyMap.find(lowerSource) == adjacencyMap.end()) {
            return 0;
        }
        const auto& edges = adjacencyMap.at(lowerSource);
        if (edges.find(lowerTarget) == edges.end()) {
            return 0;
        }
        return edges.at(lowerTarget);
    }

    // 获取顶点的所有出边
    const std::map<std::string, int>& getOutgoingEdges(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        
        static const std::map<std::string, int> emptyMap;
        if (adjacencyMap.find(lowerVertex) == adjacencyMap.end()) {
            return emptyMap;
        }
        return adjacencyMap.at(lowerVertex);
    }

    // 检查图中是否包含顶点
    bool containsVertex(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        return vertices.find(lowerVertex) != vertices.end();
    }

    // 获取指向给定顶点的所有顶点
    std::set<std::string> getIncomingVertices(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        std::set<std::string> incomingVertices;

        for (const auto& source : vertices) {
            if (getEdgeWeight(source, lowerVertex) > 0) {
                incomingVertices.insert(source);
            }
        }

        return incomingVertices;
    }

    // 获取顶点的出度（出边数量）
    int getOutDegree(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        if (adjacencyMap.find(lowerVertex) == adjacencyMap.end()) {
            return 0;
        }
        return adjacencyMap.at(lowerVertex).size();
    }
    
    // 获取图的总边数
    int getTotalEdges() const {
        int total = 0;
        for (const auto& pair : adjacencyMap) {
            total += pair.second.size();
        }
        return total;
    }
};

class TextGraphAnalysis {
private:
    DirectedGraph graph;
    std::mt19937 rng; // 随机数生成器
    static constexpr double DAMPING_FACTOR = 0.85; // PageRank算法的阻尼因子d
    static constexpr int MAX_ITERATIONS = 100; // PageRank迭代的最大次数
    static constexpr double CONVERGENCE_THRESHOLD = 0.0001; // PageRank收敛阈值
    
    // 存储单词出现频率，用于TF-IDF计算
    std::map<std::string, int> wordFrequency;
    int totalWords = 0;

    // 存储原始文本，用于词频计算
    std::vector<std::string> textWords;
    
    // 导出DOT文件的默认路径
    std::string defaultDotPath = "graph.dot";
    std::string defaultImagePath = "graph.png";

public:
    TextGraphAnalysis() : rng(std::random_device()()) {
        // 用随机数种子初始化随机数生成器
    }

    // 处理文本文件并构建有向图
    void processFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件: " + filePath);
        }

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + " "; // 将换行符替换为空格
        }

        // 将文本转换为小写，并将标点符号替换为空格
        std::string text = content;
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        
        // 将非字母字符替换为空格
        for (auto& c : text) {
            if (!std::isalpha(c)) {
                c = ' ';
            }
        }

        // 将多个连续空格替换为单个空格
        std::string cleanedText;
        bool lastWasSpace = true;
        for (char c : text) {
            if (c == ' ') {
                if (!lastWasSpace) {
                    cleanedText += c;
                    lastWasSpace = true;
                }
            } else {
                cleanedText += c;
                lastWasSpace = false;
            }
        }
        if (!cleanedText.empty() && cleanedText.back() == ' ') {
            cleanedText.pop_back();
        }

        // 将文本分割成单词
        std::vector<std::string> words;
        std::istringstream iss(cleanedText);
        std::string word;
        while (iss >> word) {
            words.push_back(word);
            textWords.push_back(word);
            wordFrequency[word]++;
            totalWords++;
        }

        // 根据相邻单词构建有向图
        for (size_t i = 0; i < words.size() - 1; ++i) {
            if (!words[i].empty() && !words[i+1].empty()) {
                graph.addEdge(words[i], words[i+1]);
            }
        }
    }

    // 显示有向图
    void showDirectedGraph() {
        std::cout << "有向图结构:" << std::endl;
        std::cout << "===========" << std::endl;
        std::cout << "顶点数量: " << graph.getVertices().size() << std::endl;

        // 打印每个顶点及其出边
        std::vector<std::string> sortedVertices(graph.getVertices().begin(), graph.getVertices().end());
        std::sort(sortedVertices.begin(), sortedVertices.end());

        for (const auto& vertex : sortedVertices) {
            std::cout << "\n顶点: " << vertex << std::endl;
            const auto& edges = graph.getOutgoingEdges(vertex);
            if (edges.empty()) {
                std::cout << "  无出边" << std::endl;
            } else {
                std::cout << "  出边:" << std::endl;
                std::vector<std::string> sortedTargets;
                for (const auto& edge : edges) {
                    sortedTargets.push_back(edge.first);
                }
                std::sort(sortedTargets.begin(), sortedTargets.end());

                for (const auto& target : sortedTargets) {
                    std::cout << "    → " << target << " (权重: " << edges.at(target) << ")" << std::endl;
                }
            }
        }
        std::cout << "\n===========" << std::endl;
    }

    // 导出图为DOT格式文件（可选功能）
    bool exportGraphToDOT(const std::string& dotFilePath = "") {
        std::string filePath = dotFilePath.empty() ? defaultDotPath : dotFilePath;
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法创建DOT文件: " << filePath << std::endl;
            return false;
        }

        file << "digraph TextGraph {" << std::endl;
        file << "  node [shape=ellipse, style=filled, fillcolor=lightblue];" << std::endl;
        file << "  edge [color=gray];" << std::endl;

        // 添加所有顶点
        for (const auto& vertex : graph.getVertices()) {
            file << "  \"" << vertex << "\";" << std::endl;
        }

        // 添加所有边
        for (const auto& vertex : graph.getVertices()) {
            const auto& edges = graph.getOutgoingEdges(vertex);
            for (const auto& edge : edges) {
                file << "  \"" << vertex << "\" -> \"" << edge.first 
                     << "\" [label=\"" << edge.second << "\", weight=" << edge.second << "];" << std::endl;
            }
        }

        file << "}" << std::endl;
        file.close();
        
        std::cout << "已将图导出为DOT文件: " << filePath << std::endl;
        return true;
    }

    // 使用Graphviz将DOT文件转换为图像（可选功能）
    bool generateGraphImage(const std::string& dotFilePath = "", const std::string& imageFilePath = "") {
        std::string dotPath = dotFilePath.empty() ? defaultDotPath : dotFilePath;
        std::string imgPath = imageFilePath.empty() ? defaultImagePath : imageFilePath;
        
        // 构建调用Graphviz的命令
        std::string command = "dot -Tpng \"" + dotPath + "\" -o \"" + imgPath + "\"";
        
        std::cout << "正在执行命令: " << command << std::endl;
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "成功生成图像文件: " << imgPath << std::endl;
            return true;
        } else {
            std::cerr << "生成图像失败！请确保已安装Graphviz并添加到PATH中！" << std::endl;
            return false;
        }
    }

    // 查询桥接词
    std::string queryBridgeWords(const std::string& word1, const std::string& word2) {
        std::string lowerWord1 = word1;
        std::string lowerWord2 = word2;
        std::transform(lowerWord1.begin(), lowerWord1.end(), lowerWord1.begin(), ::tolower);
        std::transform(lowerWord2.begin(), lowerWord2.end(), lowerWord2.begin(), ::tolower);

        // 检查单词是否存在于图中
        if (!graph.containsVertex(lowerWord1) && !graph.containsVertex(lowerWord2)) {
            return "图中不存在 \"" + word1 + "\" 和 \"" + word2 + "\"!";
        } else if (!graph.containsVertex(lowerWord1)) {
            return "图中不存在 \"" + word1 + "\"!";
        } else if (!graph.containsVertex(lowerWord2)) {
            return "图中不存在 \"" + word2 + "\"!";
        }

        // 查找桥接词
        std::vector<std::string> bridgeWords;
        for (const auto& vertex : graph.getVertices()) {
            if (graph.getEdgeWeight(lowerWord1, vertex) > 0 && graph.getEdgeWeight(vertex, lowerWord2) > 0) {
                bridgeWords.push_back(vertex);
            }
        }

        // 格式化结果
        if (bridgeWords.empty()) {
            return "从 \"" + word1 + "\" 到 \"" + word2 + "\" 没有桥接词!";
        } else if (bridgeWords.size() == 1) {
            return "从 \"" + word1 + "\" 到 \"" + word2 + "\" 的桥接词是: \"" + bridgeWords[0] + "\".";
        } else {
            std::string result = "从 \"" + word1 + "\" 到 \"" + word2 + "\" 的桥接词有: ";
            for (size_t i = 0; i < bridgeWords.size() - 1; ++i) {
                result += "\"" + bridgeWords[i] + "\", ";
            }
            result += "和 \"" + bridgeWords.back() + "\".";
            return result;
        }
    }

    // 根据桥接词生成新文本
    std::string generateNewText(const std::string& inputText) {
        // 处理输入文本
        std::string text = inputText;
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        
        // 将非字母字符替换为空格
        for (auto& c : text) {
            if (!std::isalpha(c)) {
                c = ' ';
            }
        }

        // 将多个连续空格替换为单个空格
        std::string cleanedText;
        bool lastWasSpace = true;
        for (char c : text) {
            if (c == ' ') {
                if (!lastWasSpace) {
                    cleanedText += c;
                    lastWasSpace = true;
                }
            } else {
                cleanedText += c;
                lastWasSpace = false;
            }
        }
        if (!cleanedText.empty() && cleanedText.back() == ' ') {
            cleanedText.pop_back();
        }

        // 将文本分割成单词
        std::vector<std::string> words;
        std::istringstream iss(cleanedText);
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        // 处理0或1个单词的情况
        if (words.size() <= 1) {
            return inputText;
        }

        std::string newText = words[0];

        // 处理每对相邻单词
        for (size_t i = 0; i < words.size() - 1; ++i) {
            const std::string& word1 = words[i];
            const std::string& word2 = words[i + 1];

            // 查找桥接词
            std::vector<std::string> bridgeWords;
            if (graph.containsVertex(word1) && graph.containsVertex(word2)) {
                for (const auto& vertex : graph.getVertices()) {
                    if (graph.getEdgeWeight(word1, vertex) > 0 && graph.getEdgeWeight(vertex, word2) > 0) {
                        bridgeWords.push_back(vertex);
                    }
                }
            }

            // 如果找到桥接词，随机选择一个插入
            if (!bridgeWords.empty()) {
                std::uniform_int_distribution<> dist(0, bridgeWords.size() - 1);
                std::string bridgeWord = bridgeWords[dist(rng)];
                newText += " " + bridgeWord;
            }

            // 添加下一个单词
            newText += " " + word2;
        }

        return newText;
    }

    // 计算两个单词之间的所有最短路径（可选功能增强版）
    std::string calcShortestPath(const std::string& word1, const std::string& word2) {
        std::string lowerWord1 = word1;
        std::string lowerWord2 = word2;
        std::transform(lowerWord1.begin(), lowerWord1.end(), lowerWord1.begin(), ::tolower);
        std::transform(lowerWord2.begin(), lowerWord2.end(), lowerWord2.begin(), ::tolower);

        // 当仅提供一个单词时，计算到所有其他单词的最短路径（可选功能）
        if (word2.empty()) {
            return calcPathsFromSingleWord(lowerWord1);
        }

        // 检查单词是否存在于图中
        if (!graph.containsVertex(lowerWord1) && !graph.containsVertex(lowerWord2)) {
            return "图中不存在 \"" + word1 + "\" 和 \"" + word2 + "\"!";
        } else if (!graph.containsVertex(lowerWord1)) {
            return "图中不存在 \"" + word1 + "\"!";
        } else if (!graph.containsVertex(lowerWord2)) {
            return "图中不存在 \"" + word2 + "\"!";
        }

        // 使用修改版的Dijkstra算法找出所有最短路径
        return findAllShortestPaths(lowerWord1, lowerWord2);
    }

    // 计算单个单词到所有其他单词的最短路径（可选功能）
    std::string calcPathsFromSingleWord(const std::string& word) {
        if (!graph.containsVertex(word)) {
            return "图中不存在 \"" + word + "\"!";
        }

        std::string result = "从 \"" + word + "\" 到所有其他单词的最短路径:\n";
        result += "==================================\n";

        // 对每个其他单词计算最短路径
        for (const auto& target : graph.getVertices()) {
            if (target != word) {
                std::string pathResult = findAllShortestPaths(word, target);
                // 如果有路径，添加到结果中
                if (pathResult.find("不存在路径") == std::string::npos) {
                    result += pathResult + "\n----------------------------------\n";
                }
            }
        }

        return result;
    }

    // 查找两个单词之间的所有最短路径（可选功能）
    std::string findAllShortestPaths(const std::string& source, const std::string& target) {
        // 距离映射
        std::map<std::string, int> distances;
        // 前驱节点映射（存储多个前驱节点以跟踪所有路径）
        std::map<std::string, std::vector<std::string>> previousNodes;
        
        // 优先队列的自定义比较器
        auto comparator = [&distances](const std::string& a, const std::string& b) {
            return distances[a] > distances[b]; // 最小堆
        };
        std::priority_queue<std::string, std::vector<std::string>, decltype(comparator)> unvisited(comparator);

        // 初始化距离
        for (const auto& vertex : graph.getVertices()) {
            distances[vertex] = std::numeric_limits<int>::max();
            previousNodes[vertex] = std::vector<std::string>();
        }
        distances[source] = 0;
        unvisited.push(source);

        // 已处理的节点集合
        std::set<std::string> processed;

        // Dijkstra算法
        while (!unvisited.empty()) {
            std::string current = unvisited.top();
            unvisited.pop();

            // 如果已经处理过，跳过
            if (processed.find(current) != processed.end()) {
                continue;
            }
            
            // 标记为已处理
            processed.insert(current);

            // 探索邻居
            for (const auto& edge : graph.getOutgoingEdges(current)) {
                const std::string& neighbor = edge.first;
                int weight = edge.second;
                int distance = distances[current] + weight;

                // 如果找到更短的路径
                if (distance < distances[neighbor]) {
                    // 更新距离和前驱节点
                    distances[neighbor] = distance;
                    previousNodes[neighbor].clear();
                    previousNodes[neighbor].push_back(current);

                    // 添加到优先队列
                    unvisited.push(neighbor);
                }
                // 如果找到相同长度的路径
                else if (distance == distances[neighbor]) {
                    // 添加另一个前驱节点
                    previousNodes[neighbor].push_back(current);
                }
            }
        }

        // 检查是否存在路径
        if (distances[target] == std::numeric_limits<int>::max()) {
            return "从 \"" + source + "\" 到 \"" + target + "\" 不存在路径!";
        }

        // 重构所有最短路径
        std::vector<std::vector<std::string>> allPaths;
        std::vector<std::string> currentPath;
        reconstructAllPaths(source, target, previousNodes, currentPath, allPaths);

        // 格式化结果
        std::string result = "从 \"" + source + "\" 到 \"" + target + "\" 的最短路径 (长度: " + 
                             std::to_string(distances[target]) + "):\n";
        
        for (size_t i = 0; i < allPaths.size(); ++i) {
            result += "路径 " + std::to_string(i + 1) + ": ";
            for (size_t j = 0; j < allPaths[i].size() - 1; ++j) {
                result += allPaths[i][j] + " → ";
            }
            result += allPaths[i].back() + "\n";
        }

        return result;
    }

    // 递归重构所有最短路径
    void reconstructAllPaths(const std::string& source, const std::string& current,
                             const std::map<std::string, std::vector<std::string>>& previousNodes,
                             std::vector<std::string>& currentPath,
                             std::vector<std::vector<std::string>>& allPaths) {
        // 将当前节点添加到路径中
        currentPath.push_back(current);

        // 基本情况：到达源节点
        if (current == source) {
            // 创建路径副本并反转
            std::vector<std::string> completePath = currentPath;
            std::reverse(completePath.begin(), completePath.end());
            allPaths.push_back(completePath);
        } else {
            // 递归探索所有前驱节点
            for (const auto& prev : previousNodes.at(current)) {
                reconstructAllPaths(source, prev, previousNodes, currentPath, allPaths);
            }
        }

        // 回溯：从路径中移除当前节点
        currentPath.pop_back();
    }

    // 计算单词的PageRank值，支持TF-IDF优化（可选功能）
    double calPageRank(const std::string& word) {
        std::string lowerWord = word;
        std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);

        if (!graph.containsVertex(lowerWord)) {
            return 0.0;
        }

        int n = graph.getVertices().size();
        std::map<std::string, double> pageRank;
        std::map<std::string, double> newPageRank;

        // 初始化PageRank值，使用TF-IDF优化（可选功能）
        bool useTfIdf = true; // 设置为true使用TF-IDF优化
        
        if (useTfIdf) {
            // 计算TF-IDF
            double totalSum = 0.0;
            std::map<std::string, double> tfidf;
            
            for (const auto& vertex : graph.getVertices()) {
                // 计算TF（词频）
                double tf = static_cast<double>(wordFrequency[vertex]) / totalWords;
                
                // 计算IDF（逆文档频率）- 简化版本
                double idf = 1.0; // 基础值
                
                // 根据节点的连接情况调整IDF
                int inDegree = graph.getIncomingVertices(vertex).size();
                int outDegree = graph.getOutDegree(vertex);
                
                // 重要节点（高入度、高出度）有更高的IDF
                if (inDegree > 0 || outDegree > 0) {
                    idf = log10(n / (1.0 + inDegree + outDegree));
                    idf = std::max(idf, 0.5); // 确保IDF不会太小
                }
                
                // 计算TF-IDF
                double score = tf * idf;
                tfidf[vertex] = score;
                totalSum += score;
            }
            
            // 归一化TF-IDF值作为初始PageRank
            for (const auto& vertex : graph.getVertices()) {
                pageRank[vertex] = tfidf[vertex] / totalSum;
            }
        } else {
            // 使用标准均匀分布
            for (const auto& vertex : graph.getVertices()) {
                pageRank[vertex] = 1.0 / n;
            }
        }

        // 迭代计算
        for (int iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
            double maxDifference = 0.0;

            // 计算每个顶点的新PageRank值
            for (const auto& vertex : graph.getVertices()) {
                // 计算所有指向该顶点的页面的PR(v)/L(v)之和
                double sum = 0.0;
                std::set<std::string> incomingVertices = graph.getIncomingVertices(vertex);

                for (const auto& source : incomingVertices) {
                    int outDegree = graph.getOutDegree(source);
                    if (outDegree > 0) {
                        sum += pageRank[source] / outDegree;
                    }
                }

                // PageRank公式: PR(u) = (1-d)/N + d * sum(PR(v)/L(v))
                double newRank = (1 - DAMPING_FACTOR) / n + DAMPING_FACTOR * sum;
                newPageRank[vertex] = newRank;

                // 跟踪最大变化
                maxDifference = std::max(maxDifference, std::abs(newRank - pageRank[vertex]));
            }

            // 更新PageRank值
            pageRank = newPageRank;

            // 检查收敛性
            if (maxDifference < CONVERGENCE_THRESHOLD) {
                std::cout << "PageRank在" << (iteration + 1) << "次迭代后收敛！" << std::endl;
                break;
            }
        }

        return pageRank[lowerWord];
    }

    // 在图上执行随机游走
    std::string randomWalk() {
        if (graph.getVertices().empty()) {
            return "图为空!";
        }

        // 随机选择起始顶点
        std::vector<std::string> vertices(graph.getVertices().begin(), graph.getVertices().end());
        std::uniform_int_distribution<> dist(0, vertices.size() - 1);
        std::string current = vertices[dist(rng)];

        // 跟踪已访问的边以检测循环
        std::set<std::string> visitedEdges;
        std::vector<std::string> path;
        path.push_back(current);

        // 继续游走直到重复边或到达没有出边的顶点
        while (true) {
            const auto& outgoingEdges = graph.getOutgoingEdges(current);

            // 如果没有出边则停止
            if (outgoingEdges.empty()) {
                break;
            }

            // 随机选择一条出边
            std::vector<std::string> targets;
            for (const auto& edge : outgoingEdges) {
                targets.push_back(edge.first);
            }
            std::uniform_int_distribution<> edgeDist(0, targets.size() - 1);
            std::string next = targets[edgeDist(rng)];

            // 创建边标识符
            std::string edge = current + " → " + next;

            // 如果已经访问过这条边则停止
            if (visitedEdges.find(edge) != visitedEdges.end()) {
                break;
            }

            // 将边添加到已访问集合，将顶点添加到路径
            visitedEdges.insert(edge);
            path.push_back(next);
            current = next;
        }

        // 格式化结果
        std::string result = "随机游走:\n";
        for (const auto& word : path) {
            result += word + " ";
        }

        // 将游走保存到文件
        std::ofstream file("random_walk.txt");
        if (file.is_open()) {
            for (const auto& word : path) {
                file << word << " ";
            }
            file.close();
            result += "\n\n随机游走已保存到文件: random_walk.txt";
        } else {
            result += "\n\n保存随机游走到文件失败！";
        }

        return result;
    }
    
    // 高亮显示最短路径的DOT文件生成（可选功能扩展）
    bool exportPathToDOT(const std::string& source, const std::string& target, 
                         const std::vector<std::vector<std::string>>& paths, 
                         const std::string& dotFilePath = "") {
        std::string filePath = dotFilePath.empty() ? "path_" + source + "_to_" + target + ".dot" : dotFilePath;
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法创建DOT文件: " << filePath << std::endl;
            return false;
        }

        file << "digraph TextGraph {" << std::endl;
        file << "  node [shape=ellipse, style=filled, fillcolor=lightblue];" << std::endl;
        file << "  edge [color=gray];" << std::endl;

        // 添加所有顶点
        for (const auto& vertex : graph.getVertices()) {
            // 高亮源和目标顶点
            if (vertex == source || vertex == target) {
                file << "  \"" << vertex << "\" [fillcolor=gold];" << std::endl;
            } else {
                // 检查顶点是否在任何路径上
                bool onPath = false;
                for (const auto& path : paths) {
                    if (std::find(path.begin(), path.end(), vertex) != path.end()) {
                        onPath = true;
                        break;
                    }
                }
                
                if (onPath) {
                    file << "  \"" << vertex << "\" [fillcolor=lightgreen];" << std::endl;
                } else {
                    file << "  \"" << vertex << "\";" << std::endl;
                }
            }
        }

        // 定义不同颜色的路径
        std::vector<std::string> pathColors = {"red", "blue", "green", "purple", "orange", "brown"};
        
        // 添加所有边
        for (const auto& vertex : graph.getVertices()) {
            const auto& edges = graph.getOutgoingEdges(vertex);
            for (const auto& edge : edges) {
                // 检查边是否在任何路径上
                bool onAnyPath = false;
                int pathIndex = -1;
                
                for (size_t i = 0; i < paths.size(); ++i) {
                    const auto& path = paths[i];
                    for (size_t j = 0; j < path.size() - 1; ++j) {
                        if (path[j] == vertex && path[j+1] == edge.first) {
                            onAnyPath = true;
                            pathIndex = i;
                            break;
                        }
                    }
                    if (onAnyPath) break;
                }
                
                if (onAnyPath) {
                    std::string color = pathColors[pathIndex % pathColors.size()];
                    file << "  \"" << vertex << "\" -> \"" << edge.first 
                         << "\" [label=\"" << edge.second << "\", weight=" << edge.second 
                         << ", color=" << color << ", penwidth=2.0];" << std::endl;
                } else {
                    file << "  \"" << vertex << "\" -> \"" << edge.first 
                         << "\" [label=\"" << edge.second << "\", weight=" << edge.second << "];" << std::endl;
                }
            }
        }

        // 添加图例
        file << "  subgraph cluster_legend {" << std::endl;
        file << "    label=\"路径图例\";" << std::endl;
        file << "    style=filled;" << std::endl;
        file << "    color=lightgrey;" << std::endl;
        
        for (size_t i = 0; i < std::min(paths.size(), pathColors.size()); ++i) {
            file << "    \"路径" << (i+1) << "\" [shape=plaintext, fillcolor=white, label=\"路径" << (i+1) << "\"];" << std::endl;
            file << "    \"颜色" << (i+1) << "\" [shape=point, fillcolor=" << pathColors[i] << ", color=" << pathColors[i] << ", width=0.3];" << std::endl;
            file << "    \"路径" << (i+1) << "\" -> \"颜色" << (i+1) << "\" [style=invis];" << std::endl;
        }
        
        file << "  }" << std::endl;

        file << "}" << std::endl;
        file.close();
        
        std::cout << "已将路径导出为DOT文件: " << filePath << std::endl;
        return true;
    }
}; // TextGraphAnalysis 类结束

// 主函数
int main(int argc, char* argv[]) {
    TextGraphAnalysis textGraph;

    // 处理输入文件
    std::cout << "欢迎使用文本图分析程序" << std::endl;
    std::cout << "======================" << std::endl;

    // 获取文件路径
    std::string filePath;
    if (argc > 1) {
        filePath = argv[1];
    } else {
        std::cout << "请输入文本文件的路径: ";
        std::getline(std::cin, filePath);
    }

    // 处理文件
    try {
        std::cout << "正在处理文件: " << filePath << std::endl;
        textGraph.processFile(filePath);
        std::cout << "文件处理成功！" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    // 菜单循环
    bool exit = false;
    while (!exit) {
        std::cout << "\n菜单:" << std::endl;
        std::cout << "1. 显示有向图" << std::endl;
        std::cout << "2. 导出有向图（可选功能）" << std::endl;
        std::cout << "3. 查询桥接词" << std::endl;
        std::cout << "4. 生成新文本" << std::endl;
        std::cout << "5. 计算最短路径" << std::endl;
        std::cout << "6. 计算PageRank" << std::endl;
        std::cout << "7. 随机游走" << std::endl;
        std::cout << "0. 退出" << std::endl;
        std::cout << "请输入选择: ";

        int choice;
        std::string input;
        std::getline(std::cin, input);
        try {
            choice = std::stoi(input);
        } catch (const std::exception&) {
            std::cout << "输入无效！请输入一个数字！" << std::endl;
            continue;
        }

        switch (choice) {
            case 0:
                exit = true;
                break;

            case 1:
                textGraph.showDirectedGraph();
                break;

            case 2: {
                // 可选功能：导出图
                std::string dotFilePath, imageFilePath;
                std::cout << "请输入DOT文件的保存路径（默认为graph.dot）: ";
                std::getline(std::cin, dotFilePath);
                
                if (textGraph.exportGraphToDOT(dotFilePath)) {
                    std::cout << "是否要生成图像文件？(y/n): ";
                    std::string generateImage;
                    std::getline(std::cin, generateImage);
                    
                    if (generateImage == "y" || generateImage == "Y") {
                        std::cout << "请输入图像文件的保存路径（默认为graph.png）: ";
                        std::getline(std::cin, imageFilePath);
                        textGraph.generateGraphImage(dotFilePath, imageFilePath);
                    }
                }
                break;
            }

            case 3: {
                std::string word1, word2;
                std::cout << "请输入第一个单词: ";
                std::getline(std::cin, word1);
                std::cout << "请输入第二个单词: ";
                std::getline(std::cin, word2);
                std::cout << textGraph.queryBridgeWords(word1, word2) << std::endl;
                break;
            }

            case 4: {
                std::string inputText;
                std::cout << "请输入文本: ";
                std::getline(std::cin, inputText);
                std::cout << "生成的文本: " << textGraph.generateNewText(inputText) << std::endl;
                break;
            }

            case 5: {
                std::string startWord, endWord;
                std::cout << "请输入起始单词（如果只想计算单个单词到所有单词的路径，请只输入起始单词并留空目标单词）: ";
                std::getline(std::cin, startWord);
                if (!startWord.empty()) {
                    std::cout << "请输入目标单词（留空将计算到所有单词的最短路径）: ";
                    std::getline(std::cin, endWord);
                    
                    std::string pathResult = textGraph.calcShortestPath(startWord, endWord);
                    std::cout << pathResult << std::endl;
                    
                    // 删除这里以下所有关于可视化的代码
                }
                break;
            }

            case 6: {
                std::string word;
                std::cout << "请输入要计算PageRank的单词: ";
                std::getline(std::cin, word);
                double pageRank = textGraph.calPageRank(word);
                std::cout << "\"" << word << "\" 的PageRank值: " << std::fixed << std::setprecision(4) << pageRank << std::endl;
                break;
            }

            case 7:
                std::cout << textGraph.randomWalk() << std::endl;
                break;

            default:
                std::cout << "选择无效！请重试！" << std::endl;
        }
    }

    std::cout << "再见!" << std::endl;
    return 0;
}