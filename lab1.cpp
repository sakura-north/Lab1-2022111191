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

//Ide�޸�

/**
 * �ı�ͼ��������
 * 
 * ��������ļ��ж�ȡ�ı������ݵ���������ϵ��������ͼ��
 * ���ṩ���·������ܣ�
 * - ��ʾ����ͼ�ṹ
 * - �����ŽӴ�
 * - �����ŽӴ��������ı�
 * - ���ҵ���֮������·��
 * - ����PageRankֵ
 * - ��ͼ��ִ���������
 * 
 * �������п�ѡ���ܣ�
 * - ������ͼ���ӻ�������Ϊͼ���ļ�
 * - ���㲢��ʾ�������·��
 * - ���㵥�ʵ������������ʵ����·��
 * - ʹ��TF-IDF�Ż�PageRank��ʼֵ
 */
class DirectedGraph {
private:
    // �ڽӱ�Դ���� -> (Ŀ�굥��, ����Ƶ��)
    std::map<std::string, std::map<std::string, int>> adjacencyMap;
    std::set<std::string> vertices; // �洢���ж��㣨���ʣ�

public:
    DirectedGraph() {}

    // ��ͼ����Ӷ���
    void addVertex(const std::string& vertex) {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        vertices.insert(lowerVertex);
        if (adjacencyMap.find(lowerVertex) == adjacencyMap.end()) {
            adjacencyMap[lowerVertex] = std::map<std::string, int>();
        }
    }

    // ��ͼ����ӱߣ�Ȩ��Ϊ1��������Ѵ���������Ȩ��
    void addEdge(const std::string& source, const std::string& target) {
        std::string lowerSource = source;
        std::string lowerTarget = target;
        std::transform(lowerSource.begin(), lowerSource.end(), lowerSource.begin(), ::tolower);
        std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

        // ������㲻���������
        addVertex(lowerSource);
        addVertex(lowerTarget);
        
        // ��ӻ����ӱߵ�Ȩ��
        adjacencyMap[lowerSource][lowerTarget]++;
    }

    // ��ȡͼ�����ж���
    const std::set<std::string>& getVertices() const {
        return vertices;
    }

    // ��ȡ��source��target�ıߵ�Ȩ��
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

    // ��ȡ��������г���
    const std::map<std::string, int>& getOutgoingEdges(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        
        static const std::map<std::string, int> emptyMap;
        if (adjacencyMap.find(lowerVertex) == adjacencyMap.end()) {
            return emptyMap;
        }
        return adjacencyMap.at(lowerVertex);
    }

    // ���ͼ���Ƿ��������
    bool containsVertex(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        return vertices.find(lowerVertex) != vertices.end();
    }

    // ��ȡָ�������������ж���
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

    // ��ȡ����ĳ��ȣ�����������
    int getOutDegree(const std::string& vertex) const {
        std::string lowerVertex = vertex;
        std::transform(lowerVertex.begin(), lowerVertex.end(), lowerVertex.begin(), ::tolower);
        if (adjacencyMap.find(lowerVertex) == adjacencyMap.end()) {
            return 0;
        }
        return adjacencyMap.at(lowerVertex).size();
    }
    
    // ��ȡͼ���ܱ���
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
    std::mt19937 rng; // �����������
    static constexpr double DAMPING_FACTOR = 0.85; // PageRank�㷨����������d
    static constexpr int MAX_ITERATIONS = 100; // PageRank������������
    static constexpr double CONVERGENCE_THRESHOLD = 0.0001; // PageRank������ֵ
    
    // �洢���ʳ���Ƶ�ʣ�����TF-IDF����
    std::map<std::string, int> wordFrequency;
    int totalWords = 0;

    // �洢ԭʼ�ı������ڴ�Ƶ����
    std::vector<std::string> textWords;
    
    // ����DOT�ļ���Ĭ��·��
    std::string defaultDotPath = "graph.dot";
    std::string defaultImagePath = "graph.png";

public:
    TextGraphAnalysis() : rng(std::random_device()()) {
        // ����������ӳ�ʼ�������������
    }

    // �����ı��ļ�����������ͼ
    void processFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("�޷����ļ�: " + filePath);
        }

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + " "; // �����з��滻Ϊ�ո�
        }

        // ���ı�ת��ΪСд�������������滻Ϊ�ո�
        std::string text = content;
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        
        // ������ĸ�ַ��滻Ϊ�ո�
        for (auto& c : text) {
            if (!std::isalpha(c)) {
                c = ' ';
            }
        }

        // ����������ո��滻Ϊ�����ո�
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

        // ���ı��ָ�ɵ���
        std::vector<std::string> words;
        std::istringstream iss(cleanedText);
        std::string word;
        while (iss >> word) {
            words.push_back(word);
            textWords.push_back(word);
            wordFrequency[word]++;
            totalWords++;
        }

        // �������ڵ��ʹ�������ͼ
        for (size_t i = 0; i < words.size() - 1; ++i) {
            if (!words[i].empty() && !words[i+1].empty()) {
                graph.addEdge(words[i], words[i+1]);
            }
        }
    }

    // ��ʾ����ͼ
    void showDirectedGraph() {
        std::cout << "����ͼ�ṹ:" << std::endl;
        std::cout << "===========" << std::endl;
        std::cout << "��������: " << graph.getVertices().size() << std::endl;

        // ��ӡÿ�����㼰�����
        std::vector<std::string> sortedVertices(graph.getVertices().begin(), graph.getVertices().end());
        std::sort(sortedVertices.begin(), sortedVertices.end());

        for (const auto& vertex : sortedVertices) {
            std::cout << "\n����: " << vertex << std::endl;
            const auto& edges = graph.getOutgoingEdges(vertex);
            if (edges.empty()) {
                std::cout << "  �޳���" << std::endl;
            } else {
                std::cout << "  ����:" << std::endl;
                std::vector<std::string> sortedTargets;
                for (const auto& edge : edges) {
                    sortedTargets.push_back(edge.first);
                }
                std::sort(sortedTargets.begin(), sortedTargets.end());

                for (const auto& target : sortedTargets) {
                    std::cout << "    �� " << target << " (Ȩ��: " << edges.at(target) << ")" << std::endl;
                }
            }
        }
        std::cout << "\n===========" << std::endl;
    }

    // ����ͼΪDOT��ʽ�ļ�����ѡ���ܣ�
    bool exportGraphToDOT(const std::string& dotFilePath = "") {
        std::string filePath = dotFilePath.empty() ? defaultDotPath : dotFilePath;
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "�޷�����DOT�ļ�: " << filePath << std::endl;
            return false;
        }

        file << "digraph TextGraph {" << std::endl;
        file << "  node [shape=ellipse, style=filled, fillcolor=lightblue];" << std::endl;
        file << "  edge [color=gray];" << std::endl;

        // ������ж���
        for (const auto& vertex : graph.getVertices()) {
            file << "  \"" << vertex << "\";" << std::endl;
        }

        // ������б�
        for (const auto& vertex : graph.getVertices()) {
            const auto& edges = graph.getOutgoingEdges(vertex);
            for (const auto& edge : edges) {
                file << "  \"" << vertex << "\" -> \"" << edge.first 
                     << "\" [label=\"" << edge.second << "\", weight=" << edge.second << "];" << std::endl;
            }
        }

        file << "}" << std::endl;
        file.close();
        
        std::cout << "�ѽ�ͼ����ΪDOT�ļ�: " << filePath << std::endl;
        return true;
    }

    // ʹ��Graphviz��DOT�ļ�ת��Ϊͼ�񣨿�ѡ���ܣ�
    bool generateGraphImage(const std::string& dotFilePath = "", const std::string& imageFilePath = "") {
        std::string dotPath = dotFilePath.empty() ? defaultDotPath : dotFilePath;
        std::string imgPath = imageFilePath.empty() ? defaultImagePath : imageFilePath;
        
        // ��������Graphviz������
        std::string command = "dot -Tpng \"" + dotPath + "\" -o \"" + imgPath + "\"";
        
        std::cout << "����ִ������: " << command << std::endl;
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "�ɹ�����ͼ���ļ�: " << imgPath << std::endl;
            return true;
        } else {
            std::cerr << "����ͼ��ʧ�ܣ���ȷ���Ѱ�װGraphviz����ӵ�PATH�У�" << std::endl;
            return false;
        }
    }

    // ��ѯ�ŽӴ�
    std::string queryBridgeWords(const std::string& word1, const std::string& word2) {
        std::string lowerWord1 = word1;
        std::string lowerWord2 = word2;
        std::transform(lowerWord1.begin(), lowerWord1.end(), lowerWord1.begin(), ::tolower);
        std::transform(lowerWord2.begin(), lowerWord2.end(), lowerWord2.begin(), ::tolower);

        // ��鵥���Ƿ������ͼ��
        if (!graph.containsVertex(lowerWord1) && !graph.containsVertex(lowerWord2)) {
            return "ͼ�в����� \"" + word1 + "\" �� \"" + word2 + "\"!";
        } else if (!graph.containsVertex(lowerWord1)) {
            return "ͼ�в����� \"" + word1 + "\"!";
        } else if (!graph.containsVertex(lowerWord2)) {
            return "ͼ�в����� \"" + word2 + "\"!";
        }

        // �����ŽӴ�
        std::vector<std::string> bridgeWords;
        for (const auto& vertex : graph.getVertices()) {
            if (graph.getEdgeWeight(lowerWord1, vertex) > 0 && graph.getEdgeWeight(vertex, lowerWord2) > 0) {
                bridgeWords.push_back(vertex);
            }
        }

        // ��ʽ�����
        if (bridgeWords.empty()) {
            return "�� \"" + word1 + "\" �� \"" + word2 + "\" û���ŽӴ�!";
        } else if (bridgeWords.size() == 1) {
            return "�� \"" + word1 + "\" �� \"" + word2 + "\" ���ŽӴ���: \"" + bridgeWords[0] + "\".";
        } else {
            std::string result = "�� \"" + word1 + "\" �� \"" + word2 + "\" ���ŽӴ���: ";
            for (size_t i = 0; i < bridgeWords.size() - 1; ++i) {
                result += "\"" + bridgeWords[i] + "\", ";
            }
            result += "�� \"" + bridgeWords.back() + "\".";
            return result;
        }
    }

    // �����ŽӴ��������ı�
    std::string generateNewText(const std::string& inputText) {
        // ���������ı�
        std::string text = inputText;
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        
        // ������ĸ�ַ��滻Ϊ�ո�
        for (auto& c : text) {
            if (!std::isalpha(c)) {
                c = ' ';
            }
        }

        // ����������ո��滻Ϊ�����ո�
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

        // ���ı��ָ�ɵ���
        std::vector<std::string> words;
        std::istringstream iss(cleanedText);
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        // ����0��1�����ʵ����
        if (words.size() <= 1) {
            return inputText;
        }

        std::string newText = words[0];

        // ����ÿ�����ڵ���
        for (size_t i = 0; i < words.size() - 1; ++i) {
            const std::string& word1 = words[i];
            const std::string& word2 = words[i + 1];

            // �����ŽӴ�
            std::vector<std::string> bridgeWords;
            if (graph.containsVertex(word1) && graph.containsVertex(word2)) {
                for (const auto& vertex : graph.getVertices()) {
                    if (graph.getEdgeWeight(word1, vertex) > 0 && graph.getEdgeWeight(vertex, word2) > 0) {
                        bridgeWords.push_back(vertex);
                    }
                }
            }

            // ����ҵ��ŽӴʣ����ѡ��һ������
            if (!bridgeWords.empty()) {
                std::uniform_int_distribution<> dist(0, bridgeWords.size() - 1);
                std::string bridgeWord = bridgeWords[dist(rng)];
                newText += " " + bridgeWord;
            }

            // �����һ������
            newText += " " + word2;
        }

        return newText;
    }

    // ������������֮����������·������ѡ������ǿ�棩
    std::string calcShortestPath(const std::string& word1, const std::string& word2) {
        std::string lowerWord1 = word1;
        std::string lowerWord2 = word2;
        std::transform(lowerWord1.begin(), lowerWord1.end(), lowerWord1.begin(), ::tolower);
        std::transform(lowerWord2.begin(), lowerWord2.end(), lowerWord2.begin(), ::tolower);

        // �����ṩһ������ʱ�����㵽�����������ʵ����·������ѡ���ܣ�
        if (word2.empty()) {
            return calcPathsFromSingleWord(lowerWord1);
        }

        // ��鵥���Ƿ������ͼ��
        if (!graph.containsVertex(lowerWord1) && !graph.containsVertex(lowerWord2)) {
            return "ͼ�в����� \"" + word1 + "\" �� \"" + word2 + "\"!";
        } else if (!graph.containsVertex(lowerWord1)) {
            return "ͼ�в����� \"" + word1 + "\"!";
        } else if (!graph.containsVertex(lowerWord2)) {
            return "ͼ�в����� \"" + word2 + "\"!";
        }

        // ʹ���޸İ��Dijkstra�㷨�ҳ��������·��
        return findAllShortestPaths(lowerWord1, lowerWord2);
    }

    // ���㵥�����ʵ������������ʵ����·������ѡ���ܣ�
    std::string calcPathsFromSingleWord(const std::string& word) {
        if (!graph.containsVertex(word)) {
            return "ͼ�в����� \"" + word + "\"!";
        }

        std::string result = "�� \"" + word + "\" �������������ʵ����·��:\n";
        result += "==================================\n";

        // ��ÿ���������ʼ������·��
        for (const auto& target : graph.getVertices()) {
            if (target != word) {
                std::string pathResult = findAllShortestPaths(word, target);
                // �����·������ӵ������
                if (pathResult.find("������·��") == std::string::npos) {
                    result += pathResult + "\n----------------------------------\n";
                }
            }
        }

        return result;
    }

    // ������������֮����������·������ѡ���ܣ�
    std::string findAllShortestPaths(const std::string& source, const std::string& target) {
        // ����ӳ��
        std::map<std::string, int> distances;
        // ǰ���ڵ�ӳ�䣨�洢���ǰ���ڵ��Ը�������·����
        std::map<std::string, std::vector<std::string>> previousNodes;
        
        // ���ȶ��е��Զ���Ƚ���
        auto comparator = [&distances](const std::string& a, const std::string& b) {
            return distances[a] > distances[b]; // ��С��
        };
        std::priority_queue<std::string, std::vector<std::string>, decltype(comparator)> unvisited(comparator);

        // ��ʼ������
        for (const auto& vertex : graph.getVertices()) {
            distances[vertex] = std::numeric_limits<int>::max();
            previousNodes[vertex] = std::vector<std::string>();
        }
        distances[source] = 0;
        unvisited.push(source);

        // �Ѵ���Ľڵ㼯��
        std::set<std::string> processed;

        // Dijkstra�㷨
        while (!unvisited.empty()) {
            std::string current = unvisited.top();
            unvisited.pop();

            // ����Ѿ������������
            if (processed.find(current) != processed.end()) {
                continue;
            }
            
            // ���Ϊ�Ѵ���
            processed.insert(current);

            // ̽���ھ�
            for (const auto& edge : graph.getOutgoingEdges(current)) {
                const std::string& neighbor = edge.first;
                int weight = edge.second;
                int distance = distances[current] + weight;

                // ����ҵ����̵�·��
                if (distance < distances[neighbor]) {
                    // ���¾����ǰ���ڵ�
                    distances[neighbor] = distance;
                    previousNodes[neighbor].clear();
                    previousNodes[neighbor].push_back(current);

                    // ��ӵ����ȶ���
                    unvisited.push(neighbor);
                }
                // ����ҵ���ͬ���ȵ�·��
                else if (distance == distances[neighbor]) {
                    // �����һ��ǰ���ڵ�
                    previousNodes[neighbor].push_back(current);
                }
            }
        }

        // ����Ƿ����·��
        if (distances[target] == std::numeric_limits<int>::max()) {
            return "�� \"" + source + "\" �� \"" + target + "\" ������·��!";
        }

        // �ع��������·��
        std::vector<std::vector<std::string>> allPaths;
        std::vector<std::string> currentPath;
        reconstructAllPaths(source, target, previousNodes, currentPath, allPaths);

        // ��ʽ�����
        std::string result = "�� \"" + source + "\" �� \"" + target + "\" �����·�� (����: " + 
                             std::to_string(distances[target]) + "):\n";
        
        for (size_t i = 0; i < allPaths.size(); ++i) {
            result += "·�� " + std::to_string(i + 1) + ": ";
            for (size_t j = 0; j < allPaths[i].size() - 1; ++j) {
                result += allPaths[i][j] + " �� ";
            }
            result += allPaths[i].back() + "\n";
        }

        return result;
    }

    // �ݹ��ع��������·��
    void reconstructAllPaths(const std::string& source, const std::string& current,
                             const std::map<std::string, std::vector<std::string>>& previousNodes,
                             std::vector<std::string>& currentPath,
                             std::vector<std::vector<std::string>>& allPaths) {
        // ����ǰ�ڵ���ӵ�·����
        currentPath.push_back(current);

        // �������������Դ�ڵ�
        if (current == source) {
            // ����·����������ת
            std::vector<std::string> completePath = currentPath;
            std::reverse(completePath.begin(), completePath.end());
            allPaths.push_back(completePath);
        } else {
            // �ݹ�̽������ǰ���ڵ�
            for (const auto& prev : previousNodes.at(current)) {
                reconstructAllPaths(source, prev, previousNodes, currentPath, allPaths);
            }
        }

        // ���ݣ���·�����Ƴ���ǰ�ڵ�
        currentPath.pop_back();
    }

    // ���㵥�ʵ�PageRankֵ��֧��TF-IDF�Ż�����ѡ���ܣ�
    double calPageRank(const std::string& word) {
        std::string lowerWord = word;
        std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), ::tolower);

        if (!graph.containsVertex(lowerWord)) {
            return 0.0;
        }

        int n = graph.getVertices().size();
        std::map<std::string, double> pageRank;
        std::map<std::string, double> newPageRank;

        // ��ʼ��PageRankֵ��ʹ��TF-IDF�Ż�����ѡ���ܣ�
        bool useTfIdf = true; // ����Ϊtrueʹ��TF-IDF�Ż�
        
        if (useTfIdf) {
            // ����TF-IDF
            double totalSum = 0.0;
            std::map<std::string, double> tfidf;
            
            for (const auto& vertex : graph.getVertices()) {
                // ����TF����Ƶ��
                double tf = static_cast<double>(wordFrequency[vertex]) / totalWords;
                
                // ����IDF�����ĵ�Ƶ�ʣ�- �򻯰汾
                double idf = 1.0; // ����ֵ
                
                // ���ݽڵ�������������IDF
                int inDegree = graph.getIncomingVertices(vertex).size();
                int outDegree = graph.getOutDegree(vertex);
                
                // ��Ҫ�ڵ㣨����ȡ��߳��ȣ��и��ߵ�IDF
                if (inDegree > 0 || outDegree > 0) {
                    idf = log10(n / (1.0 + inDegree + outDegree));
                    idf = std::max(idf, 0.5); // ȷ��IDF����̫С
                }
                
                // ����TF-IDF
                double score = tf * idf;
                tfidf[vertex] = score;
                totalSum += score;
            }
            
            // ��һ��TF-IDFֵ��Ϊ��ʼPageRank
            for (const auto& vertex : graph.getVertices()) {
                pageRank[vertex] = tfidf[vertex] / totalSum;
            }
        } else {
            // ʹ�ñ�׼���ȷֲ�
            for (const auto& vertex : graph.getVertices()) {
                pageRank[vertex] = 1.0 / n;
            }
        }

        // ��������
        for (int iteration = 0; iteration < MAX_ITERATIONS; ++iteration) {
            double maxDifference = 0.0;

            // ����ÿ���������PageRankֵ
            for (const auto& vertex : graph.getVertices()) {
                // ��������ָ��ö����ҳ���PR(v)/L(v)֮��
                double sum = 0.0;
                std::set<std::string> incomingVertices = graph.getIncomingVertices(vertex);

                for (const auto& source : incomingVertices) {
                    int outDegree = graph.getOutDegree(source);
                    if (outDegree > 0) {
                        sum += pageRank[source] / outDegree;
                    }
                }

                // PageRank��ʽ: PR(u) = (1-d)/N + d * sum(PR(v)/L(v))
                double newRank = (1 - DAMPING_FACTOR) / n + DAMPING_FACTOR * sum;
                newPageRank[vertex] = newRank;

                // �������仯
                maxDifference = std::max(maxDifference, std::abs(newRank - pageRank[vertex]));
            }

            // ����PageRankֵ
            pageRank = newPageRank;

            // ���������
            if (maxDifference < CONVERGENCE_THRESHOLD) {
                std::cout << "PageRank��" << (iteration + 1) << "�ε�����������" << std::endl;
                break;
            }
        }

        return pageRank[lowerWord];
    }

    // ��ͼ��ִ���������
    std::string randomWalk() {
        if (graph.getVertices().empty()) {
            return "ͼΪ��!";
        }

        // ���ѡ����ʼ����
        std::vector<std::string> vertices(graph.getVertices().begin(), graph.getVertices().end());
        std::uniform_int_distribution<> dist(0, vertices.size() - 1);
        std::string current = vertices[dist(rng)];

        // �����ѷ��ʵı��Լ��ѭ��
        std::set<std::string> visitedEdges;
        std::vector<std::string> path;
        path.push_back(current);

        // ��������ֱ���ظ��߻򵽴�û�г��ߵĶ���
        while (true) {
            const auto& outgoingEdges = graph.getOutgoingEdges(current);

            // ���û�г�����ֹͣ
            if (outgoingEdges.empty()) {
                break;
            }

            // ���ѡ��һ������
            std::vector<std::string> targets;
            for (const auto& edge : outgoingEdges) {
                targets.push_back(edge.first);
            }
            std::uniform_int_distribution<> edgeDist(0, targets.size() - 1);
            std::string next = targets[edgeDist(rng)];

            // �����߱�ʶ��
            std::string edge = current + " �� " + next;

            // ����Ѿ����ʹ���������ֹͣ
            if (visitedEdges.find(edge) != visitedEdges.end()) {
                break;
            }

            // ������ӵ��ѷ��ʼ��ϣ���������ӵ�·��
            visitedEdges.insert(edge);
            path.push_back(next);
            current = next;
        }

        // ��ʽ�����
        std::string result = "�������:\n";
        for (const auto& word : path) {
            result += word + " ";
        }

        // �����߱��浽�ļ�
        std::ofstream file("random_walk.txt");
        if (file.is_open()) {
            for (const auto& word : path) {
                file << word << " ";
            }
            file.close();
            result += "\n\n��������ѱ��浽�ļ�: random_walk.txt";
        } else {
            result += "\n\n����������ߵ��ļ�ʧ�ܣ�";
        }

        return result;
    }
    
    // ������ʾ���·����DOT�ļ����ɣ���ѡ������չ��
    bool exportPathToDOT(const std::string& source, const std::string& target, 
                         const std::vector<std::vector<std::string>>& paths, 
                         const std::string& dotFilePath = "") {
        std::string filePath = dotFilePath.empty() ? "path_" + source + "_to_" + target + ".dot" : dotFilePath;
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "�޷�����DOT�ļ�: " << filePath << std::endl;
            return false;
        }

        file << "digraph TextGraph {" << std::endl;
        file << "  node [shape=ellipse, style=filled, fillcolor=lightblue];" << std::endl;
        file << "  edge [color=gray];" << std::endl;

        // ������ж���
        for (const auto& vertex : graph.getVertices()) {
            // ����Դ��Ŀ�궥��
            if (vertex == source || vertex == target) {
                file << "  \"" << vertex << "\" [fillcolor=gold];" << std::endl;
            } else {
                // ��鶥���Ƿ����κ�·����
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

        // ���岻ͬ��ɫ��·��
        std::vector<std::string> pathColors = {"red", "blue", "green", "purple", "orange", "brown"};
        
        // ������б�
        for (const auto& vertex : graph.getVertices()) {
            const auto& edges = graph.getOutgoingEdges(vertex);
            for (const auto& edge : edges) {
                // �����Ƿ����κ�·����
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

        // ���ͼ��
        file << "  subgraph cluster_legend {" << std::endl;
        file << "    label=\"·��ͼ��\";" << std::endl;
        file << "    style=filled;" << std::endl;
        file << "    color=lightgrey;" << std::endl;
        
        for (size_t i = 0; i < std::min(paths.size(), pathColors.size()); ++i) {
            file << "    \"·��" << (i+1) << "\" [shape=plaintext, fillcolor=white, label=\"·��" << (i+1) << "\"];" << std::endl;
            file << "    \"��ɫ" << (i+1) << "\" [shape=point, fillcolor=" << pathColors[i] << ", color=" << pathColors[i] << ", width=0.3];" << std::endl;
            file << "    \"·��" << (i+1) << "\" -> \"��ɫ" << (i+1) << "\" [style=invis];" << std::endl;
        }
        
        file << "  }" << std::endl;

        file << "}" << std::endl;
        file.close();
        
        std::cout << "�ѽ�·������ΪDOT�ļ�: " << filePath << std::endl;
        return true;
    }
}; // TextGraphAnalysis �����

// ������
int main(int argc, char* argv[]) {
    TextGraphAnalysis textGraph;

    // ���������ļ�
    std::cout << "��ӭʹ���ı�ͼ��������" << std::endl;
    std::cout << "======================" << std::endl;

    // ��ȡ�ļ�·��
    std::string filePath;
    if (argc > 1) {
        filePath = argv[1];
    } else {
        std::cout << "�������ı��ļ���·��: ";
        std::getline(std::cin, filePath);
    }

    // �����ļ�
    try {
        std::cout << "���ڴ����ļ�: " << filePath << std::endl;
        textGraph.processFile(filePath);
        std::cout << "�ļ�����ɹ���" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "����: " << e.what() << std::endl;
        return 1;
    }

    // �˵�ѭ��
    bool exit = false;
    while (!exit) {
        std::cout << "\n�˵�:" << std::endl;
        std::cout << "1. ��ʾ����ͼ" << std::endl;
        std::cout << "2. ��������ͼ����ѡ���ܣ�" << std::endl;
        std::cout << "3. ��ѯ�ŽӴ�" << std::endl;
        std::cout << "4. �������ı�" << std::endl;
        std::cout << "5. �������·��" << std::endl;
        std::cout << "6. ����PageRank" << std::endl;
        std::cout << "7. �������" << std::endl;
        std::cout << "0. �˳�" << std::endl;
        std::cout << "������ѡ��: ";

        int choice;
        std::string input;
        std::getline(std::cin, input);
        try {
            choice = std::stoi(input);
        } catch (const std::exception&) {
            std::cout << "������Ч��������һ�����֣�" << std::endl;
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
                // ��ѡ���ܣ�����ͼ
                std::string dotFilePath, imageFilePath;
                std::cout << "������DOT�ļ��ı���·����Ĭ��Ϊgraph.dot��: ";
                std::getline(std::cin, dotFilePath);
                
                if (textGraph.exportGraphToDOT(dotFilePath)) {
                    std::cout << "�Ƿ�Ҫ����ͼ���ļ���(y/n): ";
                    std::string generateImage;
                    std::getline(std::cin, generateImage);
                    
                    if (generateImage == "y" || generateImage == "Y") {
                        std::cout << "������ͼ���ļ��ı���·����Ĭ��Ϊgraph.png��: ";
                        std::getline(std::cin, imageFilePath);
                        textGraph.generateGraphImage(dotFilePath, imageFilePath);
                    }
                }
                break;
            }

            case 3: {
                std::string word1, word2;
                std::cout << "�������һ������: ";
                std::getline(std::cin, word1);
                std::cout << "������ڶ�������: ";
                std::getline(std::cin, word2);
                std::cout << textGraph.queryBridgeWords(word1, word2) << std::endl;
                break;
            }

            case 4: {
                std::string inputText;
                std::cout << "�������ı�: ";
                std::getline(std::cin, inputText);
                std::cout << "���ɵ��ı�: " << textGraph.generateNewText(inputText) << std::endl;
                break;
            }

            case 5: {
                std::string startWord, endWord;
                std::cout << "��������ʼ���ʣ����ֻ����㵥�����ʵ����е��ʵ�·������ֻ������ʼ���ʲ�����Ŀ�굥�ʣ�: ";
                std::getline(std::cin, startWord);
                if (!startWord.empty()) {
                    std::cout << "������Ŀ�굥�ʣ����ս����㵽���е��ʵ����·����: ";
                    std::getline(std::cin, endWord);
                    
                    std::string pathResult = textGraph.calcShortestPath(startWord, endWord);
                    std::cout << pathResult << std::endl;
                    
                    // ɾ�������������й��ڿ��ӻ��Ĵ���
                }
                break;
            }

            case 6: {
                std::string word;
                std::cout << "������Ҫ����PageRank�ĵ���: ";
                std::getline(std::cin, word);
                double pageRank = textGraph.calPageRank(word);
                std::cout << "\"" << word << "\" ��PageRankֵ: " << std::fixed << std::setprecision(4) << pageRank << std::endl;
                break;
            }

            case 7:
                std::cout << textGraph.randomWalk() << std::endl;
                break;

            default:
                std::cout << "ѡ����Ч�������ԣ�" << std::endl;
        }
    }

    std::cout << "�ټ�!" << std::endl;
    return 0;
}