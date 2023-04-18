package io.confluent.avro.random.generator;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Random;
import java.util.stream.Collectors;

import static io.confluent.avro.random.generator.util.ResourceUtil.loadContent;
import static junit.framework.TestCase.assertEquals;

@RunWith(Parameterized.class)
public class GeneratorTest {

  private static final String SCHEMA_TEST_DIR = "test-schemas";
  private final String fileName;
  private final String content;

  /**
   * Run the test for each test schema.
   *
   * @return array of [fileName, file-content]
   */
  @Parameterized.Parameters(name = "{0}")
  public static Collection<Object[]> data() {
    return findTestSchemas().stream()
        .map(fileName -> new Object[]{fileName, loadContent(SCHEMA_TEST_DIR + "/" + fileName)})
        .collect(Collectors.toCollection(ArrayList::new));
  }

  public GeneratorTest(final String fileName, final String content) {
    this.fileName = fileName;
    this.content = content;
  }

  @Test
  public void shouldHandleSchema() {
    final Generator generator = new Generator.Builder()
        .schemaString(content)
        .build();
    final Object generated = generator.generate();
    System.out.println(fileName + ": " + generated);
  }

  private static List<String> findTestSchemas() {
    final InputStream testDir =
        GeneratorTest.class.getClassLoader().getResourceAsStream(SCHEMA_TEST_DIR);

    try (BufferedReader reader = new BufferedReader(
        new InputStreamReader(testDir, StandardCharsets.UTF_8))) {

      return reader.lines()
          .filter(filename -> filename.endsWith(".json"))
          .collect(Collectors.toList());

    } catch (final IOException ioe) {
      throw new RuntimeException("failed to find test schemas", ioe);
    }
  }

  @Test
  public void shouldGenerateValuesDeterministically() {
    long seed = 100L;
    Generator generatorA = new Generator.Builder()
        .schemaString(content)
        .random(new Random(seed))
        .build();
    Generator generatorB = new Generator.Builder()
        .schemaString(content)
        .random(new Random(seed))
        .build();
    assertEquals(generatorA.generate(), generatorB.generate());
    assertEquals(generatorA.generate(), generatorB.generate());
  }
}