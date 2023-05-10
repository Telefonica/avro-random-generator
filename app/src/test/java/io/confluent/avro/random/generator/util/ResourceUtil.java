package io.confluent.avro.random.generator.util;

import io.confluent.avro.random.generator.Generator;
import io.confluent.avro.random.generator.GeneratorTest;
import org.apache.avro.generic.GenericRecord;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Optional;
import java.util.stream.Collectors;


public final class ResourceUtil {

  private ResourceUtil() {
  }

  /**
   * Load file contents from classpath as string.
   * @param filePath the relative resource path.
   * @return the file contents.
   */
  public static String loadContent(final String filePath) {
    final InputStream testDir =
        GeneratorTest.class.getClassLoader().getResourceAsStream(filePath);

    try (BufferedReader reader = new BufferedReader(
        new InputStreamReader(testDir, StandardCharsets.UTF_8))) {

      return reader.lines().collect(Collectors.joining("\n"));
    } catch (final IOException ioe) {
      throw new RuntimeException("failed to find test test-schema " + filePath, ioe);
    }
  }

  public static GenericRecord generateRecordWithSchema(String path) {
    return generateRecordWithSchema(path, Optional.empty());
  }

  public static GenericRecord generateRecordWithSchema(String path, Optional<Double> notInformedColumnRate) {
    Generator generator = builderWithSchema(path, notInformedColumnRate, Optional.empty());
    return (GenericRecord) generator.generate();
  }

  public static Generator builderWithSchema(String path) {
    return builderWithSchema(path, Optional.empty(), Optional.empty());
  }

  public static Generator builderWithSchemaAndMalformedRate(String path, Double malformedColumnRate) {
    return builderWithSchema(path, Optional.empty(), Optional.of(malformedColumnRate));
  }

  public static Generator builderWithSchema(String path, Optional<Double> notInformedColumnRate, Optional<Double> malformedColumnRate) {
    String schema = ResourceUtil.loadContent(path);
    return new Generator.Builder().schemaString(schema, notInformedColumnRate.isPresent())
            .malformedColumnRate(malformedColumnRate)
            .notInformedColumnRate(notInformedColumnRate)
            .build();
  }

}
