import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.AfterEach;
import static org.junit.jupiter.api.Assertions.*;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

@SuppressWarnings({"checkstyle:Indentation", "checkstyle:MissingJavadocType"})
public class TextGraphAnalysisTest {

    private TextGraphAnalysis textGraph;

    @BeforeEach
    void setUp() {
        textGraph = new TextGraphAnalysis();
        setupTestGraph();
    }

    private void setupTestGraph() {
        try {
            File testFile = new File("test_input.txt");
            FileWriter writer = new FileWriter(testFile);

            writer.write("the big cat sits on the mat. ");
            writer.write("a small dog runs fast. ");
            writer.write("hello world peace. ");
            writer.write("the big dog runs. ");
            writer.close();

            textGraph.processFile("test_input.txt");
            testFile.delete();

        } catch (IOException e) {
            fail("Failed to setup test graph: " + e.getMessage());
        }
    }

    // 测试用例1: 测试存在单个桥接词的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC001: 测试存在桥接词的情况")
    void testQueryBridgeWords_ExistingBridge() {
        String result = textGraph.queryBridgeWords("the", "cat");

        assertNotNull(result, "结果不应该为null");
        assertTrue(result.contains("big"), "应该包含桥接词'big'");
        assertTrue(result.contains("桥接词"), "应该包含'桥接词'提示");

        System.out.println("TC001通过: " + result);
    }

    // 测试用例2: 测试存在桥接词的情况2
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC002: 测试存在桥接词的情况2")
    void testQueryBridgeWords_ExistingBridge2() {
        String result = textGraph.queryBridgeWords("hello", "peace");

        assertNotNull(result, "结果不应该为null");
        assertTrue(result.contains("world"), "应该包含桥接词'world'");

        System.out.println("TC002通过: " + result);
    }

    // 测试用例3: 测试直接相邻单词无桥接词的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC003: 测试直接相邻的单词")
    void testQueryBridgeWords_DirectlyConnected() {
        String result = textGraph.queryBridgeWords("the", "big");

        assertNotNull(result, "直接相邻单词查询结果不应该为null");
        assertTrue(result.contains("没有桥接词"), "直接相邻的单词之间应该没有桥接词");

        System.out.println("TC003通过: " + result);
    }

    // 测试用例4: 测试第一个单词不存在的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC004: 测试第一个单词不存在")
    void testQueryBridgeWords_FirstWordNotExist() {
        String result = textGraph.queryBridgeWords("nonexist", "cat");

        assertNotNull(result, "结果不应该为null");
        assertTrue(result.contains("不存在"), "应该包含'不存在'提示");
        assertTrue(result.contains("nonexist"), "应该包含不存在的单词");

        System.out.println("TC004通过: " + result);
    }

    // 测试用例5: 测试第二个单词不存在的情况
    @SuppressWarnings({"checkstyle:Indentation", "checkstyle:MethodName"})
    @Test
    @DisplayName("TC005: 测试第二个单词不存在")
    void testQueryBridgeWords_SecondWordNotExist() {
        String result = textGraph.queryBridgeWords("the", "nonexist");

        assertNotNull(result, "结果不应该为null");
        assertTrue(result.contains("不存在"), "应该包含'不存在'提示");
        assertTrue(result.contains("nonexist"), "应该包含不存在的单词");

        System.out.println("TC005通过: " + result);
    }

    // 测试用例6: 测试两个单词都不存在的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC006: 测试两个单词都不存在")
    void testQueryBridgeWords_BothWordsNotExist() {
        String result = textGraph.queryBridgeWords("nonexist1", "nonexist2");

        assertNotNull(result, "结果不应该为null");
        assertTrue(result.contains("不存在"), "应该包含'不存在'提示");
        assertTrue(result.contains("nonexist1") && result.contains("nonexist2"),
                "应该包含两个不存在的单词");

        System.out.println("TC006通过: " + result);
    }

    // 测试用例7: 测试空字符串输入的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC007: 测试空字符串输入")
    void testQueryBridgeWords_EmptyString() {
        String result = textGraph.queryBridgeWords("", "cat");

        assertNotNull(result, "结果不应该为null");
        assertTrue(result.contains("不存在"), "应该处理空字符串为不存在的情况");

        System.out.println("TC007通过: " + result);
    }

    // 测试用例8: 测试相同单词输入的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC008: 测试相同单词输入")
    void testQueryBridgeWords_SameWords() {
        String result = textGraph.queryBridgeWords("the", "the");

        assertNotNull(result, "相同单词输入结果不应该为null");
        assertTrue(result.contains("没有桥接词"), "相同单词之间应该没有桥接词");

        System.out.println("TC008通过: " + result);
    }

    // 测试用例9: 测试大小写不敏感的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC009: 测试大小写不敏感")
    void testQueryBridgeWords_CaseInsensitive() {
        String result1 = textGraph.queryBridgeWords("THE", "CAT");
        String result2 = textGraph.queryBridgeWords("the", "CAT");
        String result3 = textGraph.queryBridgeWords("the", "cat");

        assertNotNull(result1, "大写输入结果不应该为null");
        assertNotNull(result2, "混合大小写结果不应该为null");
        assertNotNull(result3, "小写输入结果不应该为null");

        assertTrue(result1.contains("big"), "大写输入应该找到桥接词");
        assertTrue(result2.contains("big"), "混合大小写应该找到桥接词");
        assertTrue(result3.contains("big"), "小写输入应该找到桥接词");

        System.out.println("TC009通过:");
        System.out.println("  大写: " + result1);
        System.out.println("  混合: " + result2);
        System.out.println("  小写: " + result3);
    }

    // 测试用例10: 测试单词顺序敏感性的情况
    @SuppressWarnings("checkstyle:MethodName")
    @Test
    @DisplayName("TC010: 测试单词顺序敏感性")
    void testQueryBridgeWords_OrderSensitive() {
        String result1 = textGraph.queryBridgeWords("the", "cat");
        String result2 = textGraph.queryBridgeWords("cat", "the");

        assertNotNull(result1, "正向查询结果不应该为null");
        assertNotNull(result2, "反向查询结果不应该为null");

        System.out.println("TC010通过:");
        System.out.println("  the->cat: " + result1);
        System.out.println("  cat->the: " + result2);
    }

    @AfterEach
    void tearDown() {
        System.out.println("测试用例执行完成\n");
    }

    // 如果JUnit运行器不工作，可以用这个main方法
    @SuppressWarnings("checkstyle:MissingJavadocMethod")
    public static void main(String[] args) {
        TextGraphAnalysisTest test = new TextGraphAnalysisTest();

        try {
            test.setUp();

            System.out.println("=== 开始运行JUnit风格的测试 ===\n");

            test.testQueryBridgeWords_ExistingBridge();
            test.testQueryBridgeWords_ExistingBridge2();
            test.testQueryBridgeWords_DirectlyConnected();
            test.testQueryBridgeWords_FirstWordNotExist();
            test.testQueryBridgeWords_SecondWordNotExist();
            test.testQueryBridgeWords_BothWordsNotExist();
            test.testQueryBridgeWords_EmptyString();
            test.testQueryBridgeWords_SameWords();
            test.testQueryBridgeWords_CaseInsensitive();
            test.testQueryBridgeWords_OrderSensitive();

            System.out.println("=== 所有测试完成 ===");

        } catch (Exception e) {
            System.err.println("测试执行出错: " + e.getMessage());
            e.printStackTrace();
        }
    }
}