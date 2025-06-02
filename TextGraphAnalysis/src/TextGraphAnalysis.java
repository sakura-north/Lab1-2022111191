import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;          // ← 添加这个
import java.nio.file.Paths;         // ← 添加这个
import java.security.SecureRandom;  // ← 添加这个
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Random;
import java.util.Scanner;
import java.util.Set;

/**
 * 文本图分析系统.
 * 本系统从文本文件中读取单词，根据单词间的邻接关系构建有向图，
 * 并提供一系列分析功能：
 * - 显示有向图结构
 * - 查询桥接词
 * - 根据桥接词生成新文本
 * - 查询单词之间的最短路径
 * - 计算PageRank值
 * - 在图上进行随机游走
 * 包含所有可选功能：
 * - 将有向图可视化并保存为文件
 * - 计算并显示所有最短路径
 * - 计算单词到所有其他单词的最短路径
 * - 使用TF-IDF优化PageRank初始值
 */
@SuppressWarnings({"checkstyle:Indentation", "checkstyle:OneTopLevelClass",
        "checkstyle:JavadocParagraph", "checkstyle:SummaryJavadoc"})
class DirectedGraph {
    // 邻接表：原单词 -> (目标单词, 出现频率)
    @SuppressWarnings("checkstyle:Indentation")
    private Map<String, Map<String, Integer>> adjacencyMap;
    @SuppressWarnings("checkstyle:Indentation")
    private Set<String> vertices; // 存储所有定义的单词

    @SuppressWarnings("checkstyle:Indentation")
    public DirectedGraph() {
        this.adjacencyMap = new HashMap<>();
        this.vertices = new HashSet<>();
    }

    // 向图中添加顶点
    @SuppressWarnings("checkstyle:Indentation")
    public void addVertex(String vertex) {
        String lowerVertex = vertex.toLowerCase();
        vertices.add(lowerVertex);
        if (!adjacencyMap.containsKey(lowerVertex)) {
            adjacencyMap.put(lowerVertex, new HashMap<>());
        }
    }

    // 向图中添加边，权重为1，如果边已存在则增加权重
    @SuppressWarnings("checkstyle:Indentation")
    public void addEdge(String source, String target) {
        String lowerSource = source.toLowerCase();
        String lowerTarget = target.toLowerCase();

        // 如果顶点不存在则添加
        addVertex(lowerSource);
        addVertex(lowerTarget);

        // 添加或增加边的权重
        Map<String, Integer> edges = adjacencyMap.get(lowerSource);
        edges.put(lowerTarget, edges.getOrDefault(lowerTarget, 0) + 1);
    }

    // 获取图中所有顶点
    @SuppressWarnings("checkstyle:Indentation")
    public Set<String> getVertices() {
        return new HashSet<>(vertices);
    }

    // 获取从source到target的边的权重
    @SuppressWarnings("checkstyle:Indentation")
    public int getEdgeWeight(String source, String target) {
        String lowerSource = source.toLowerCase();
        String lowerTarget = target.toLowerCase();

        if (!adjacencyMap.containsKey(lowerSource)) {
            return 0;
        }
        Map<String, Integer> edges = adjacencyMap.get(lowerSource);
        return edges.getOrDefault(lowerTarget, 0);
    }

    // 获取顶点的所有出边
    @SuppressWarnings("checkstyle:Indentation")
    public Map<String, Integer> getOutgoingEdges(String vertex) {
        String lowerVertex = vertex.toLowerCase();

        if (!adjacencyMap.containsKey(lowerVertex)) {
            return new HashMap<>();
        }
        return new HashMap<>(adjacencyMap.get(lowerVertex));
    }

    // 检查图中是否包含顶点
    @SuppressWarnings("checkstyle:Indentation")
    public boolean containsVertex(String vertex) {
        String lowerVertex = vertex.toLowerCase();
        return vertices.contains(lowerVertex);
    }

    // 获取指向给定顶点的所有顶点
    @SuppressWarnings("checkstyle:Indentation")
    public Set<String> getIncomingVertices(String vertex) {
        String lowerVertex = vertex.toLowerCase();
        Set<String> incomingVertices = new HashSet<>();

        for (String source : vertices) {
            if (getEdgeWeight(source, lowerVertex) > 0) {
                incomingVertices.add(source);
            }
        }

        return incomingVertices;
    }

    // 获取顶点的出度（出边数量）
    @SuppressWarnings("checkstyle:Indentation")
    public int getOutDegree(String vertex) {
        String lowerVertex = vertex.toLowerCase();
        if (!adjacencyMap.containsKey(lowerVertex)) {
            return 0;
        }
        return adjacencyMap.get(lowerVertex).size();
    }

    // 获取图的总边数
    @SuppressWarnings("checkstyle:Indentation")
    public int getTotalEdges() {
        int total = 0;
        for (Map<String, Integer> edges : adjacencyMap.values()) {
            total += edges.size();
        }
        return total;
    }
}

@SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocType"})
public class TextGraphAnalysis {
    @SuppressWarnings("checkstyle:Indentation")
    private DirectedGraph graph;
    @SuppressWarnings("checkstyle:Indentation")
    private Random rng; // 随机数生成器
    @SuppressWarnings("checkstyle:Indentation")
    private static final double DAMPING_FACTOR = 0.85; // PageRank算法的阻尼因子d
    @SuppressWarnings("checkstyle:Indentation")
    private static final int MAX_ITERATIONS = 100; // PageRank迭代的最大次数
    @SuppressWarnings("checkstyle:Indentation")
    private static final double CONVERGENCE_THRESHOLD = 0.0001; // PageRank收敛阈值

    // 存储单词出现频率，用于TF-IDF计算
    @SuppressWarnings("checkstyle:Indentation")
    private Map<String, Integer> wordFrequency;
    @SuppressWarnings("checkstyle:Indentation")
    private int totalWords = 0;

    // 存储原始文本，用于词频计算
    @SuppressWarnings("checkstyle:Indentation")
    private List<String> textWords;

    // 导出DOT文件的默认路径
    @SuppressWarnings("checkstyle:Indentation")
    private String defaultDotPath = "graph.dot";
    @SuppressWarnings("checkstyle:Indentation")
    private String defaultImagePath = "graph.png";

    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocMethod"})
    public TextGraphAnalysis() {
        this.graph = new DirectedGraph();
        this.rng = new SecureRandom();
        this.wordFrequency = new HashMap<>();
        this.textWords = new ArrayList<>();
    }

    private boolean isValidPath(String filePath) {
        try {
            Path path = Paths.get(filePath).normalize();
            Path currentDir = Paths.get(System.getProperty("user.dir"));

            // 修正逻辑：确保路径在当前工作目录下且不包含危险字符
            return path.startsWith(currentDir) &&
                    !path.toString().contains("..") &&
                    (!path.isAbsolute() || path.startsWith(currentDir));
        } catch (Exception e) {
            return false;
        }
    }

    // 改进的文本预处理方法
    @SuppressWarnings("checkstyle:Indentation")
    private String preprocessText(String text) {
        // 转换为小写
        text = text.toLowerCase();

        // 处理常见的缩写和连字符
        text = text.replaceAll("'re", " are");
        text = text.replaceAll("'ve", " have");
        text = text.replaceAll("'ll", " will");
        text = text.replaceAll("'d", " would");
        text = text.replaceAll("n't", " not");
        text = text.replaceAll("'s", ""); // 去掉所有格

        // 将非字母字符替换为空格
        text = text.replaceAll("[^a-zA-Z\\s]", " ");

        // 将多个连续空格替换为单个空格
        text = text.replaceAll("\\s+", " ").trim();

        return text;
    }

    // 处理文本文件并构建有向图
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocMethod"})
    public void processFile(String filePath) throws IOException {

        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(new FileInputStream(filePath), StandardCharsets.UTF_8))) {
            StringBuilder content = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append(" "); // 将换行符替换为空格
            }

            // 使用改进的文本预处理
            String text = preprocessText(content.toString());

            // 将文本分割成单词
            String[] words = text.split("\\s+");
            List<String> wordList = new ArrayList<>();

            for (String word : words) {
                if (!word.isEmpty()) {
                    wordList.add(word);
                    textWords.add(word);
                    wordFrequency.put(word, wordFrequency.getOrDefault(word, 0) + 1);
                    totalWords++;
                }
            }

            // 根据相邻单词构建有向图
            for (int i = 0; i < wordList.size() - 1; i++) {
                if (!wordList.get(i).isEmpty() && !wordList.get(i + 1).isEmpty()) {
                    graph.addEdge(wordList.get(i), wordList.get(i + 1));
                }
            }
        }
    }

    // 显示有向图
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:ParameterName",
            "checkstyle:MissingJavadocMethod"})
    public void showDirectedGraph(DirectedGraph G) {
        System.out.println("有向图结构:");
        System.out.println("===========");
        System.out.println("顶点数量: " + G.getVertices().size());

        // 打印每个顶点及其出边
        List<String> sortedVertices = new ArrayList<>(G.getVertices());
        Collections.sort(sortedVertices);

        for (String vertex : sortedVertices) {
            System.out.println("\n顶点: " + vertex);
            Map<String, Integer> edges = G.getOutgoingEdges(vertex);
            if (edges.isEmpty()) {
                System.out.println("  无出边");
            } else {
                System.out.println("  出边:");
                List<String> sortedTargets = new ArrayList<>(edges.keySet());
                Collections.sort(sortedTargets);

                for (String target : sortedTargets) {
                    System.out.println("    → " + target + " (权重: " + edges.get(target) + ")");
                }
            }
        }
        System.out.println("\n===========");
    }

    // 转义DOT文件中的特殊字符
    @SuppressWarnings("checkstyle:Indentation")
    private String escapeDotString(String str) {
        return str.replace("\\", "\\\\")
                .replace("\"", "\\\"")
                .replace("\n", "\\n")
                .replace("\r", "\\r")
                .replace("\t", "\\t");
    }

    // 改进的DOT文件导出方法
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:LineLength", "checkstyle:AbbreviationAsWordInName", "checkstyle:MissingJavadocMethod"})
    public boolean exportGraphToDOT(String dotFilePath) {
        String filePath = dotFilePath.isEmpty() ? defaultDotPath : dotFilePath;

        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(filePath), StandardCharsets.UTF_8))) {

            writer.println("digraph TextGraph {");
            writer.println("  rankdir=LR;");  // 从左到右布局
            writer.println("  node [shape=ellipse, style=filled, fillcolor=lightblue, fontname=\"Arial\"];");
            writer.println("  edge [color=gray, fontname=\"Arial\"];");
            writer.println("  bgcolor=white;");
            writer.println();

            // 添加所有顶点
            for (String vertex : graph.getVertices()) {
                String escapedVertex = escapeDotString(vertex);
                writer.println("  \"" + escapedVertex + "\";");
            }
            writer.println();

            // 添加所有边
            for (String vertex : graph.getVertices()) {
                Map<String, Integer> edges = graph.getOutgoingEdges(vertex);
                for (Map.Entry<String, Integer> edge : edges.entrySet()) {
                    String escapedSource = escapeDotString(vertex);
                    String escapedTarget = escapeDotString(edge.getKey());
                    int weight = edge.getValue();

                    // 根据权重调整边的粗细和颜色
                    String edgeStyle = "";
                    if (weight > 1) {
                        edgeStyle = String.format("[label=\"%d\", penwidth=%.1f, color=\"blue\"]",
                                weight, Math.min(weight * 0.5 + 0.5, 3.0));
                    } else {
                        edgeStyle = "[color=\"gray\"]";
                    }

                    writer.println("  \"" + escapedSource + "\" -> \"" + escapedTarget + "\" " + edgeStyle + ";");
                }
            }

            writer.println("}");

            System.out.println("已将图导出为DOT文件: " + filePath);
            return true;
        } catch (IOException e) {
            System.err.println("无法创建DOT文件: " + filePath + " - " + e.getMessage());
            return false;
        }
    }

    // 改进的Graphviz图像生成方法
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:LineLength", "checkstyle:MissingJavadocMethod"})
    public boolean generateGraphImage(String dotFilePath, String imageFilePath) {
        String dotPath = dotFilePath.isEmpty() ? defaultDotPath : dotFilePath;
        String imgPath = imageFilePath.isEmpty() ? defaultImagePath : imageFilePath;

        try {
            // 检查DOT文件是否存在
            File dotFile = new File(dotPath);
            if (!dotFile.exists()) {
                System.err.println("DOT文件不存在: " + dotPath);
                return false;
            }

            // 构建ProcessBuilder而不是Runtime.exec()
            ProcessBuilder pb = new ProcessBuilder();

            // 尝试不同的Graphviz命令
            String[] commands = {"dot", "dot.exe"};
            boolean foundDot = false;

            for (String cmd : commands) {
                try {
                    pb.command(cmd, "-V");
                    Process testProcess = pb.start();
                    testProcess.waitFor();
                    if (testProcess.exitValue() == 0) {
                        foundDot = true;
                        pb.command(cmd, "-Tpng", dotPath, "-o", imgPath);
                        break;
                    }
                } catch (IOException | InterruptedException e) {
                    // 继续尝试下一个命令
                }
            }

            if (!foundDot) {
                System.err.println("未找到Graphviz命令。请确保已安装Graphviz并添加到PATH中。");
                return false;
            }

            System.out.println("正在生成图像文件...");
            Process process = pb.start();

            // 读取错误流
            StringBuilder errorOutput = new StringBuilder();
            try (BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream(), StandardCharsets.UTF_8))) {
                String errorLine;
                while ((errorLine = errorReader.readLine()) != null) {
                    errorOutput.append(errorLine).append("\n");
                }
            }

            int result = process.waitFor();

            if (result == 0) {
                System.out.println("成功生成图像文件: " + imgPath);
                return true;
            } else {
                System.err.println("生成图像失败！退出代码: " + result);
                if (errorOutput.length() > 0) {
                    System.err.println("错误信息: " + errorOutput.toString());
                }
                return false;
            }
        } catch (IOException | InterruptedException e) {
            System.err.println("执行命令时出错: " + e.getMessage());
            return false;
        }
    }

    // 查询桥接词
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:LineLength", "checkstyle:MissingJavadocMethod"})
    public String queryBridgeWords(String word1, String word2) {
        String lowerWord1 = word1.toLowerCase();
        String lowerWord2 = word2.toLowerCase();

        // 检查单词是否存在于图中
        if (!graph.containsVertex(lowerWord1) && !graph.containsVertex(lowerWord2)) {
            return "图中不存在 \"" + word1 + "\" 和 \"" + word2 + "\"!";
        } else if (!graph.containsVertex(lowerWord1)) {
            return "图中不存在 \"" + word1 + "\"!";
        } else if (!graph.containsVertex(lowerWord2)) {
            return "图中不存在 \"" + word2 + "\"!";
        }

        // 查找桥接词
        List<String> bridgeWords = new ArrayList<>();
        for (String vertex : graph.getVertices()) {
            if (graph.getEdgeWeight(lowerWord1, vertex) > 0 && graph.getEdgeWeight(vertex, lowerWord2) > 0) {
                bridgeWords.add(vertex);
            }
        }

        // 格式化结果
        if (bridgeWords.isEmpty()) {
            return "从 \"" + word1 + "\" 到 \"" + word2 + "\" 没有桥接词!";
        } else if (bridgeWords.size() == 1) {
            return "从 \"" + word1 + "\" 到 \"" + word2 + "\" 的桥接词是: \"" + bridgeWords.get(0) + "\".";
        } else {
            StringBuilder result = new StringBuilder("从 \"" + word1 + "\" 到 \"" + word2 + "\" 的桥接词有: ");
            for (int i = 0; i < bridgeWords.size() - 1; i++) {
                result.append("\"").append(bridgeWords.get(i)).append("\", ");
            }
            result.append("和 \"").append(bridgeWords.get(bridgeWords.size() - 1)).append("\".");
            return result.toString();
        }
    }

    // 根据桥接词生成新文本
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:LineLength", "checkstyle:MissingJavadocMethod"})
    public String generateNewText(String inputText) {
        // 使用改进的文本预处理
        String text = preprocessText(inputText);

        // 将文本分割成单词
        String[] wordsArray = text.split("\\s+");
        List<String> words = new ArrayList<>();
        for (String word : wordsArray) {
            if (!word.isEmpty()) {
                words.add(word);
            }
        }

        // 处理0或1个单词的情况
        if (words.size() <= 1) {
            return inputText;
        }

        StringBuilder newText = new StringBuilder(words.get(0));

        // 处理每对相邻单词
        for (int i = 0; i < words.size() - 1; i++) {
            String word1 = words.get(i);
            String word2 = words.get(i + 1);

            // 查找桥接词
            List<String> bridgeWords = new ArrayList<>();
            if (graph.containsVertex(word1) && graph.containsVertex(word2)) {
                for (String vertex : graph.getVertices()) {
                    if (graph.getEdgeWeight(word1, vertex) > 0 && graph.getEdgeWeight(vertex, word2) > 0) {
                        bridgeWords.add(vertex);
                    }
                }
            }

            // 如果找到桥接词，随机选择一个插入
            if (!bridgeWords.isEmpty()) {
                String bridgeWord = bridgeWords.get(rng.nextInt(bridgeWords.size()));
                newText.append(" ").append(bridgeWord);
            }

            // 添加下一个单词
            newText.append(" ").append(word2);
        }

        return newText.toString();
    }

    // 计算两个单词之间的所有最短路径（可选功能增强）
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocMethod"})
    public String calcShortestPath(String word1, String word2) {
        String lowerWord1 = word1.toLowerCase();
        String lowerWord2 = word2.toLowerCase();

        // 当只提供一个单词时，计算到所有其他单词的最短路径（可选功能）
        if (word2.isEmpty()) {
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
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocMethod"})
    public String calcPathsFromSingleWord(String word) {
        if (!graph.containsVertex(word)) {
            return "图中不存在 \"" + word + "\"!";
        }

        StringBuilder result = new StringBuilder("从 \"" + word + "\" 到所有其他单词的最短路径:\n");
        result.append("==================================\n");

        // 对每个其他单词计算最短路径
        for (String target : graph.getVertices()) {
            if (!target.equals(word)) {
                String pathResult = findAllShortestPaths(word, target);
                // 如果有路径，添加到结果中
                if (!pathResult.contains("不存在路径")) {
                    result.append(pathResult).append("\n----------------------------------\n");
                }
            }
        }

        return result.toString();
    }

    // 查找两个单词之间的所有最短路径（可选功能）
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:OperatorWrap", "checkstyle:LineLength", "checkstyle:RightCurly", "checkstyle:MissingJavadocMethod"})
    public String findAllShortestPaths(String source, String target) {
        // 距离映射
        Map<String, Integer> distances = new HashMap<>();
        // 前驱节点映射，存储多个前驱节点以支持所有路径
        Map<String, List<String>> previousNodes = new HashMap<>();

        PriorityQueue<String> unvisited = new PriorityQueue<>((a, b) ->
                Integer.compare(distances.get(a), distances.get(b)));

        // 初始化距离
        for (String vertex : graph.getVertices()) {
            distances.put(vertex, Integer.MAX_VALUE);
            previousNodes.put(vertex, new ArrayList<>());
        }
        distances.put(source, 0);
        unvisited.add(source);

        // 已处理的节点集合
        Set<String> processed = new HashSet<>();

        // Dijkstra算法
        while (!unvisited.isEmpty()) {
            String current = unvisited.poll();

            // 如果已经处理过，跳过
            if (processed.contains(current)) {
                continue;
            }

            // 标记为已处理
            processed.add(current);

            // 探索邻居
            for (Map.Entry<String, Integer> edge : graph.getOutgoingEdges(current).entrySet()) {
                String neighbor = edge.getKey();
                int weight = edge.getValue();
                int distance = distances.get(current) + weight;

                // 如果找到更短的路径
                if (distance < distances.get(neighbor)) {
                    // 更新距离和前驱节点
                    distances.put(neighbor, distance);
                    previousNodes.get(neighbor).clear();
                    previousNodes.get(neighbor).add(current);

                    // 添加到优先队列
                    unvisited.add(neighbor);
                }
                // 如果找到相同长度的路径
                else if (distance == distances.get(neighbor)) {
                    // 添加另一个前驱节点
                    previousNodes.get(neighbor).add(current);
                }
            }
        }

        // 检查是否存在路径
        if (distances.get(target) == Integer.MAX_VALUE) {
            return "从 \"" + source + "\" 到 \"" + target + "\" 不存在路径!";
        }

        // 重构所有最短路径
        List<List<String>> allPaths = new ArrayList<>();
        List<String> currentPath = new ArrayList<>();
        reconstructAllPaths(source, target, previousNodes, currentPath, allPaths);

        // 格式化结果
        StringBuilder result = new StringBuilder("从 \"" + source + "\" 到 \"" + target + "\" 的最短路径 (长度: " +
                distances.get(target) + "):\n");

        for (int i = 0; i < allPaths.size(); i++) {
            result.append("路径 ").append(i + 1).append(": ");
            for (int j = 0; j < allPaths.get(i).size() - 1; j++) {
                result.append(allPaths.get(i).get(j)).append(" → ");
            }
            result.append(allPaths.get(i).get(allPaths.get(i).size() - 1)).append("\n");
        }

        return result.toString();
    }

    // 递归重构所有最短路径
    @SuppressWarnings("checkstyle:Indentation")
    private void reconstructAllPaths(String source, String current,
                                     Map<String, List<String>> previousNodes,
                                     List<String> currentPath,
                                     List<List<String>> allPaths) {
        // 将当前节点添加到路径中
        currentPath.add(current);

        // 基本情况：到达源节点
        if (current.equals(source)) {
            // 创建路径副本并反转
            List<String> completePath = new ArrayList<>(currentPath);
            Collections.reverse(completePath);
            allPaths.add(completePath);
        } else {
            // 递归探索所有前驱节点
            for (String prev : previousNodes.get(current)) {
                reconstructAllPaths(source, prev, previousNodes, currentPath, allPaths);
            }
        }

        // 回溯：从路径中移除当前节点
        currentPath.remove(currentPath.size() - 1);
    }

    // 计算单词的PageRank值，支持TF-IDF优化（可选功能）
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocMethod"})
    public Double calPageRank(String word) {
        String lowerWord = word.toLowerCase();

        if (!graph.containsVertex(lowerWord)) {
            return 0.0;
        }

        int n = graph.getVertices().size();
        Map<String, Double> pageRank = new HashMap<>();
        Map<String, Double> newPageRank = new HashMap<>();

        // 初始化PageRank值，使用TF-IDF优化（可选功能）
        boolean useTfIdf = true; // 设置为true使用TF-IDF优化

        if (useTfIdf) {
            // 计算TF-IDF
            double totalSum = 0.0;
            Map<String, Double> tfidf = new HashMap<>();

            for (String vertex : graph.getVertices()) {
                // 计算TF（词频）
                double tf = (double) wordFrequency.getOrDefault(vertex, 0) / totalWords;

                // 计算IDF（逆文档频率）- 简化版本
                double idf = 1.0; // 基础值

                // 根据节点的连接情况调整IDF
                int inDegree = graph.getIncomingVertices(vertex).size();
                int outDegree = graph.getOutDegree(vertex);

                // 重要节点（有更多的IDF权重
                if (inDegree > 0 || outDegree > 0) {
                    idf = Math.log10((double) n / (1.0 + inDegree + outDegree));
                    idf = Math.max(idf, 0.5); // 确保IDF不会太小
                }

                // 计算TF-IDF
                double score = tf * idf;
                tfidf.put(vertex, score);
                totalSum += score;
            }

            // 归一化TF-IDF值作为初始PageRank
            for (String vertex : graph.getVertices()) {
                pageRank.put(vertex, tfidf.get(vertex) / totalSum);
            }
        } else {
            // 使用标准均匀分布
            for (String vertex : graph.getVertices()) {
                pageRank.put(vertex, 1.0 / n);
            }
        }

        // 迭代计算
        for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
            double maxDifference = 0.0;

            // 计算每个顶点的新PageRank值
            for (String vertex : graph.getVertices()) {
                // 计算所有指向该顶点的贡献的PR(v)/L(v)之和
                double sum = 0.0;
                Set<String> incomingVertices = graph.getIncomingVertices(vertex);

                for (String source : incomingVertices) {
                    int outDegree = graph.getOutDegree(source);
                    if (outDegree > 0) {
                        sum += pageRank.get(source) / outDegree;
                    }
                }

                // PageRank公式: PR(u) = (1-d)/N + d * sum(PR(v)/L(v))
                double newRank = (1 - DAMPING_FACTOR) / n + DAMPING_FACTOR * sum;
                newPageRank.put(vertex, newRank);

                // 跟踪最大变化
                maxDifference = Math.max(maxDifference, Math.abs(newRank - pageRank.get(vertex)));
            }

            // 更新PageRank值
            pageRank = new HashMap<>(newPageRank);

            // 检查收敛性
            if (maxDifference < CONVERGENCE_THRESHOLD) {
                System.out.println("PageRank在" + (iteration + 1) + "次迭代后收敛。");
                break;
            }
        }

        return pageRank.get(lowerWord);
    }

    // 在图上进行随机游走
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocMethod"})
    public String randomWalk() {
        if (graph.getVertices().isEmpty()) {
            return "图为空!";
        }

        // 随机选择起始顶点
        List<String> vertices = new ArrayList<>(graph.getVertices());
        String current = vertices.get(rng.nextInt(vertices.size()));

        // 跟踪已访问的边以检测循环
        Set<String> visitedEdges = new HashSet<>();
        List<String> path = new ArrayList<>();
        path.add(current);

        // 继续游走直到重复边或到达没有出边的顶点
        while (true) {
            Map<String, Integer> outgoingEdges = graph.getOutgoingEdges(current);

            // 如果没有出边则终止
            if (outgoingEdges.isEmpty()) {
                break;
            }

            // 随机选择一条出边
            List<String> targets = new ArrayList<>(outgoingEdges.keySet());
            String next = targets.get(rng.nextInt(targets.size()));

            // 创建边标识符
            String edge = current + " → " + next;

            // 如果已经访问过这条边则终止
            if (visitedEdges.contains(edge)) {
                break;
            }

            // 将边添加到已访问集合，将顶点添加到路径
            visitedEdges.add(edge);
            path.add(next);
            current = next;
        }

        // 格式化结果
        StringBuilder result = new StringBuilder("随机游走:\n");
        for (String word : path) {
            result.append(word).append(" ");
        }

        // 将游走保存到文件
        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream("random_walk.txt"), StandardCharsets.UTF_8))) {
            for (String word : path) {
                writer.print(word + " ");
            }
            result.append("\n\n随机游走已保存到文件: random_walk.txt");
        } catch (IOException e) {
            result.append("\n\n保存随机游走到文件失败。");
        }

        return result.toString();
    }

    // 高级显示最短路径的DOT文件生成（可选功能扩展）
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:LineLength", "checkstyle:NeedBraces", "checkstyle:OperatorWrap", "checkstyle:AbbreviationAsWordInName", "checkstyle:MissingJavadocMethod"})
    public boolean exportPathToDOT(String source, String target,
                                   List<List<String>> paths,
                                   String dotFilePath) {
        String filePath = dotFilePath.isEmpty() ?
                "path_" + source + "_to_" + target + ".dot" : dotFilePath;


        try (PrintWriter writer = new PrintWriter(new OutputStreamWriter(
                new FileOutputStream(filePath), StandardCharsets.UTF_8))) {

            writer.println("digraph TextGraph {");
            writer.println("  rankdir=LR;");
            writer.println("  node [shape=ellipse, style=filled, fillcolor=lightblue];");
            writer.println("  edge [color=gray];");
            writer.println();

            // 添加所有顶点
            for (String vertex : graph.getVertices()) {
                // 高亮源和目标顶点
                if (vertex.equals(source) || vertex.equals(target)) {
                    writer.println("  \"" + vertex + "\" [fillcolor=gold];");
                } else {
                    // 检查顶点是否在任何路径上
                    boolean onPath = false;
                    for (List<String> path : paths) {
                        if (path.contains(vertex)) {
                            onPath = true;
                            break;
                        }
                    }

                    if (onPath) {
                        writer.println("  \"" + vertex + "\" [fillcolor=lightgreen];");
                    } else {
                        writer.println("  \"" + vertex + "\";");
                    }
                }
            }

            // 定义不同颜色的路径
            String[] pathColors = {"red", "blue", "green", "purple", "orange", "brown"};

            // 添加所有边
            for (String vertex : graph.getVertices()) {
                Map<String, Integer> edges = graph.getOutgoingEdges(vertex);
                for (Map.Entry<String, Integer> edge : edges.entrySet()) {
                    // 检查边是否在任何路径上
                    boolean onAnyPath = false;
                    int pathIndex = -1;

                    for (int i = 0; i < paths.size(); i++) {
                        List<String> path = paths.get(i);
                        for (int j = 0; j < path.size() - 1; j++) {
                            if (path.get(j).equals(vertex) && path.get(j + 1).equals(edge.getKey())) {
                                onAnyPath = true;
                                pathIndex = i;
                                break;
                            }
                        }
                        if (onAnyPath) break;
                    }

                    if (onAnyPath) {
                        String color = pathColors[pathIndex % pathColors.length];
                        writer.println("  \"" + vertex + "\" -> \"" + edge.getKey()
                                + "\" [label=\"" + edge.getValue() + "\", weight=" + edge.getValue()
                                + ", color=" + color + ", penwidth=2.0];");
                    } else {
                        writer.println("  \"" + vertex + "\" -> \"" + edge.getKey()
                                + "\" [label=\"" + edge.getValue() + "\", weight=" + edge.getValue() + "];");
                    }
                }
            }

            // 添加图例
            writer.println("  subgraph cluster_legend {");
            writer.println("    label=\"路径图例\";");
            writer.println("    style=filled;");
            writer.println("    color=lightgrey;");

            for (int i = 0; i < Math.min(paths.size(), pathColors.length); i++) {
                writer.println("    \"路径" + (i + 1) + "\" [shape=plaintext, fillcolor=white, label=\"路径" + (i + 1) + "\"];");
                writer.println("    \"颜色" + (i + 1) + "\" [shape=point, fillcolor=" + pathColors[i] + ", color=" + pathColors[i] + ", width=0.3];");
                writer.println("    \"路径" + (i + 1) + "\" -> \"颜色" + (i + 1) + "\" [style=invis];");
            }

            writer.println("  }");
            writer.println("}");

            System.out.println("已将路径导出为DOT文件: " + filePath);
            return true;
        } catch (IOException e) {
            System.err.println("无法创建DOT文件: " + filePath + " - " + e.getMessage());
            return false;
        }
    }

    // 主函数
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:LeftCurly", "checkstyle:MultipleVariableDeclarations", "checkstyle:VariableDeclarationUsageDistance", "checkstyle:MissingJavadocMethod"})
    public static void main(String[] args) {
        TextGraphAnalysis textGraph = new TextGraphAnalysis();

        // 处理输入文件
        System.out.println("欢迎使用文本图分析系统");
        System.out.println("======================");

        // 获取文件路径
        String filePath;
        Scanner scanner = new Scanner(System.in, StandardCharsets.UTF_8.name());
        if (args.length > 0) {
            filePath = args[0];
        } else {
            System.out.print("请输入文本文件的路径: ");
            filePath = scanner.nextLine();
        }

        // 处理文件
        try {
            System.out.println("正在处理文件: " + filePath);
            textGraph.processFile(filePath);
            System.out.println("文件处理成功。");
        } catch (Exception e) {
            System.err.println("错误: " + e.getMessage());
            return;
        }

        // 菜单循环
        boolean exit = false;
        while (!exit) {
            System.out.println("\n菜单:");
            System.out.println("1. 显示有向图");
            System.out.println("2. 导出有向图（可选功能）");
            System.out.println("3. 查询桥接词");
            System.out.println("4. 生成新文本");
            System.out.println("5. 计算最短路径");
            System.out.println("6. 计算PageRank");
            System.out.println("7. 随机游走");
            System.out.println("0. 退出");
            System.out.print("请输入选择: ");

            int choice;
            String input = scanner.nextLine();
            try {
                choice = Integer.parseInt(input);
            } catch (NumberFormatException e) {
                System.out.println("输入无效。请输入一个数字。");
                continue;
            }

            switch (choice) {
                case 0:
                    exit = true;
                    break;

                case 1:
                    textGraph.showDirectedGraph(textGraph.graph);
                    break;

                case 2: {
                    // 可选功能：导出图
                    String dotFilePath, imageFilePath;
                    System.out.print("请输入DOT文件的保存路径（默认为graph.dot）: ");
                    dotFilePath = scanner.nextLine();

                    if (textGraph.exportGraphToDOT(dotFilePath)) {
                        System.out.print("是否生成图像文件？(y/n): ");
                        String generateImage = scanner.nextLine();

                        if (generateImage.equals("y") || generateImage.equals("Y")) {
                            System.out.print("请输入图像文件的保存路径（默认为graph.png）: ");
                            imageFilePath = scanner.nextLine();
                            textGraph.generateGraphImage(dotFilePath, imageFilePath);
                        }
                    }
                    break;
                }

                case 3: {
                    String word1, word2;
                    System.out.print("请输入第一个单词: ");
                    word1 = scanner.nextLine();
                    System.out.print("请输入第二个单词: ");
                    word2 = scanner.nextLine();
                    System.out.println(textGraph.queryBridgeWords(word1, word2));
                    break;
                }

                case 4: {
                    System.out.print("请输入文本: ");
                    String inputText = scanner.nextLine();
                    System.out.println("生成的文本: " + textGraph.generateNewText(inputText));
                    break;
                }

                case 5: {
                    String startWord, endWord;
                    System.out.print("请输入起始单词（如果只想计算单个单词到所有单词的路径，请只输入起始单词并留空目标单词）: ");
                    startWord = scanner.nextLine();
                    if (!startWord.isEmpty()) {
                        System.out.print("请输入目标单词（留空将计算到所有单词的最短路径）: ");
                        endWord = scanner.nextLine();

                        String pathResult = textGraph.calcShortestPath(startWord, endWord);
                        System.out.println(pathResult);

                        // 尝试这是正关于显示所有路径的扩展代码
                    }
                    break;
                }

                case 6: {
                    System.out.print("请输入要计算PageRank的单词: ");
                    String word = scanner.nextLine();
                    Double pageRank = textGraph.calPageRank(word);
                    System.out.printf("\"" + word + "\" 的PageRank值: %.4f%n", pageRank);
                    break;
                }

                case 7:
                    System.out.println(textGraph.randomWalk());
                    break;

                default:
                    System.out.println("选择无效。请重试。");
            }
        }

        scanner.close();
        System.out.println("再见!");
    }
}