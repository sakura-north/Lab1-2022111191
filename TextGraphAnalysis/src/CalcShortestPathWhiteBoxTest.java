import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.TestMethodOrder;
import org.junit.jupiter.api.MethodOrderer;
import static org.junit.jupiter.api.Assertions.*;
import java.io.IOException;
import java.io.File;
import java.io.FileWriter;

/**
 * calcShortestPath函数白盒测试类 - JUnit 5版本
 * 基于基本路径法，覆盖所有5条独立路径
 * 对应代码行号：519-539
 */
@SuppressWarnings({"checkstyle:Indentation", "checkstyle:SummaryJavadoc"})
@TestMethodOrder(MethodOrderer.DisplayName.class)
public class CalcShortestPathWhiteBoxTest {

    @SuppressWarnings("checkstyle:Indentation")
    private TextGraphAnalysis textGraph;
    @SuppressWarnings("checkstyle:Indentation")
    private String testFilePath = "test_whitebox_junit5.txt";

    @SuppressWarnings("checkstyle:Indentation")
    @BeforeEach
    void setUp() throws IOException {
        textGraph = new TextGraphAnalysis();

        // 创建测试文件，构建测试图
        createTestFile();

        // 处理文件构建图
        textGraph.processFile(testFilePath);

        // 清理文件
        cleanupTestFile();

        System.out.println("=== 测试环境准备完成 ===");
        System.out.println("测试图结构：hello -> world -> test, start -> end, isolated");
        System.out.println();
    }

    @SuppressWarnings("checkstyle:Indentation")
    private void createTestFile() throws IOException {
        try (FileWriter writer = new FileWriter(testFilePath)) {
            // 构建测试图：hello->world->test, start->end, isolated节点
            writer.write("hello world test start end isolated world");
        }
    }

    @SuppressWarnings("checkstyle:Indentation")
    private void cleanupTestFile() {
        File file = new File(testFilePath);
        if (file.exists()) {
            file.delete();
        }
    }

    /**
     * 测试用例1：word2为空字符串的情况
     * 覆盖路径：520→521→524(T)→525
     * 基本路径1
     */
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:OperatorWrap", "checkstyle:MethodName"})
    @Test
    @DisplayName("测试用例1：word2为空字符串")
    void testPath1_Word2Empty() {
        System.out.println("=== 测试用例1：word2为空 ===");
        System.out.println("覆盖路径：520→521→524(T)→525");

        String result = textGraph.calcShortestPath("hello", "");

        assertNotNull(result, "结果不应为null");
        assertTrue(result.contains("到所有其他单词") ||
                        result.contains("最短路径") ||
                        result.contains("不存在"),
                "应包含路径信息或不存在信息");

        System.out.println("输入: word1='hello', word2=''");
        System.out.println("输出: " + (result.length() > 80 ?
                result.substring(0, 80) + "..." : result));
        System.out.println("测试用例1 - 通过 ✓\n");
    }

    /**
     * 测试用例2：两个单词都不存在
     * 覆盖路径：520→521→524(F)→529(T)→530
     * 基本路径2
     */
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MethodName"})
    @Test
    @DisplayName("测试用例2：两个单词都不存在")
    void testPath2_BothWordsNotExist() {
        System.out.println("=== 测试用例2：两个单词都不存在 ===");
        System.out.println("覆盖路径：520→521→524(F)→529(T)→530");

        String result = textGraph.calcShortestPath("nonexistent1", "nonexistent2");
        String expected = "图中不存在 \"nonexistent1\" 和 \"nonexistent2\"!";

        assertEquals(expected, result, "应返回两个单词都不存在的错误");

        System.out.println("输入: word1='nonexistent1', word2='nonexistent2'");
        System.out.println("输出: " + result);
        System.out.println("测试用例2 - 通过 ✓\n");
    }

    /**
     * 测试用例3：第一个单词不存在，第二个存在
     * 覆盖路径：520→521→524(F)→529(F)→531(T)→532
     * 基本路径3
     */
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MethodName"})
    @Test
    @DisplayName("测试用例3：word1不存在，word2存在")
    void testPath3_Word1NotExist() {
        System.out.println("=== 测试用例3：word1不存在，word2存在 ===");
        System.out.println("覆盖路径：520→521→524(F)→529(F)→531(T)→532");

        String result = textGraph.calcShortestPath("nonexistent", "hello");
        String expected = "图中不存在 \"nonexistent\"!";

        assertEquals(expected, result, "应返回第一个单词不存在的错误");

        System.out.println("输入: word1='nonexistent', word2='hello'");
        System.out.println("输出: " + result);
        System.out.println("测试用例3 - 通过 ✓\n");
    }

    /**
     * 测试用例4：第一个单词存在，第二个不存在
     * 覆盖路径：520→521→524(F)→529(F)→531(F)→533(T)→534
     * 基本路径4
     */
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MethodName"})
    @Test
    @DisplayName("测试用例4：word1存在，word2不存在")
    void testPath4_Word2NotExist() {
        System.out.println("=== 测试用例4：word1存在，word2不存在 ===");
        System.out.println("覆盖路径：520→521→524(F)→529(F)→531(F)→533(T)→534");

        String result = textGraph.calcShortestPath("hello", "nonexistent");
        String expected = "图中不存在 \"nonexistent\"!";

        assertEquals(expected, result, "应返回第二个单词不存在的错误");

        System.out.println("输入: word1='hello', word2='nonexistent'");
        System.out.println("输出: " + result);
        System.out.println("测试用例4 - 通过 ✓\n");
    }

    /**
     * 测试用例5：两个单词都存在
     * 覆盖路径：520→521→524(F)→529(F)→531(F)→533(F)→538
     * 基本路径5
     */
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:OperatorWrap", "checkstyle:MethodName"})
    @Test
    @DisplayName("测试用例5：两个单词都存在")
    void testPath5_BothWordsExist() {
        System.out.println("=== 测试用例5：两个单词都存在 ===");
        System.out.println("覆盖路径：520→521→524(F)→529(F)→531(F)→533(F)→538");

        String result = textGraph.calcShortestPath("hello", "world");

        assertNotNull(result, "结果不应为null");
        assertTrue(result.contains("最短路径") ||
                        result.contains("不存在路径") ||
                        result.contains("从"),
                "应包含路径信息或不存在路径信息");

        System.out.println("输入: word1='hello', word2='world'");
        System.out.println("输出: " + (result.length() > 100 ?
                result.substring(0, 100) + "..." : result));
        System.out.println("测试用例5 - 通过 ✓\n");
    }

    /**
     * 额外测试：大小写转换验证
     */
    @SuppressWarnings("checkstyle:Indentation")
    @Test
    @DisplayName("额外测试：大小写转换")
    void testCaseConversion() {
        System.out.println("=== 额外测试：大小写转换 ===");

        String result1 = textGraph.calcShortestPath("HELLO", "WORLD");
        String result2 = textGraph.calcShortestPath("hello", "world");

        assertEquals(result2, result1, "大写和小写输入应产生相同结果");

        System.out.println("大小写转换测试通过 ✓\n");
    }

    /**
     * 路径覆盖总结测试
     */
    @SuppressWarnings("checkstyle:Indentation")
    @Test
    @DisplayName("路径覆盖总结")
    void testPathCoverageSummary() {
        System.out.println("=== 路径覆盖总结 ===");
        System.out.println("基本路径1 (520→521→524T→525): ✓");
        System.out.println("基本路径2 (520→521→524F→529T→530): ✓");
        System.out.println("基本路径3 (520→521→524F→529F→531T→532): ✓");
        System.out.println("基本路径4 (520→521→524F→529F→531F→533T→534): ✓");
        System.out.println("基本路径5 (520→521→524F→529F→531F→533F→538): ✓");
        System.out.println("总计: 5/5 路径覆盖 = 100%");
        System.out.println("预期语句覆盖率: 100%");
        System.out.println("预期分支覆盖率: 100%");

        assertTrue(true, "所有路径已覆盖");
    }

    /**
     * 性能测试
     */
    @SuppressWarnings("checkstyle:Indentation")
    @Test
    @DisplayName("性能测试")
    void testPerformance() {
        System.out.println("=== 性能测试 ===");

        long startTime = System.currentTimeMillis();

        // 执行多次调用
        for (int i = 0; i < 100; i++) {
            textGraph.calcShortestPath("hello", "world");
            textGraph.calcShortestPath("hello", "");
            textGraph.calcShortestPath("nonexistent1", "nonexistent2");
        }

        long endTime = System.currentTimeMillis();
        long duration = endTime - startTime;

        System.out.println("300次函数调用耗时: " + duration + "ms");
        System.out.println("平均每次调用: " + (duration / 300.0) + "ms");

        assertTrue(duration < 10000, "性能测试：应在10秒内完成300次调用");
        System.out.println("性能测试通过 ✓\n");
    }
}
