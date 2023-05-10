package io.confluent.avro.random.generator;

import com.telefonica.baikal.utils.Validations;
import org.apache.avro.generic.GenericRecord;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import static io.confluent.avro.random.generator.util.ResourceUtil.*;
import static org.junit.Assert.*;


public class CustomExtensionsGeneratorTest {

  @Rule
  public ExpectedException exceptionRule = ExpectedException.none();


  private static boolean isInt(String number) {
    if (number == null) {
      return false;
    }
    try {
      Integer.parseInt(number);
    } catch (NumberFormatException nfe) {
      return false;
    }
    return true;
  }

  @Test
  public void shouldCreateUnionWithPosition0() {
    GenericRecord record = generateRecordWithSchema("test-schemas/extensions/unions-position-0.json");

    String field = "nullable_col";
    assertNull(record.get(field));
  }

  @Test
  public void shouldCreateUnionWithPosition1() {
    GenericRecord record = generateRecordWithSchema("test-schemas/extensions/unions-position-1.json");

    String field = "nullable_col";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
  }

  @Test
  public void shouldCreateUnionWithDistributedProv() {
    Generator generator = builderWithSchema("test-schemas/extensions/unions-distribution.json");

    GenericRecord record;
    List<Object> results = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      record = (GenericRecord) generator.generate();
      Object col = record.get("nullable_col");
      if (col != null) {
        results.add(col);
      }
    }

    assertEquals("Wrong union distribution", 0.7, ((double) results.size()) / 100, 0.1);
  }

  @Test
  public void shouldCreateNotInformedUnionWithoutDistributionProv() {
    Generator generator = builderWithSchema("test-schemas/extensions/auto-not-informed-distribution.json", Optional.of(Generator.DEFAULT_NOT_INFORMED_RATE), Optional.empty());

    GenericRecord record;
    List<Object> results = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      record = (GenericRecord) generator.generate();
      Object col = record.get("col");
      if (col instanceof String) {
        results.add(col);
      }
    }

    assertEquals("Wrong union distribution", 0.9, ((double) results.size()) / 100, 0.1);
  }

  @Test
  public void shouldCreateNotInformedUnionMultipleTypesWithoutDistributionProv() {
    Generator generator = builderWithSchema("test-schemas/extensions/auto-not-informed-distribution-multiple-types.json", Optional.of(Generator.DEFAULT_NOT_INFORMED_RATE), Optional.empty());

    GenericRecord record;
    List<Object> stringResults = new ArrayList<>();
    List<Object> intResults = new ArrayList<>();
    List<Object> notInformedResults = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      record = (GenericRecord) generator.generate();
      Object col = record.get("col");
      if (col instanceof String) {
        stringResults.add(col);
      } else if (col instanceof Integer) {
        intResults.add(col);
      } else {
        notInformedResults.add(col);
      }
    }

    assertEquals("Wrong string distribution", 0.45, ((double) stringResults.size()) / 100, 0.1);
    assertEquals("Wrong int distribution", 0.45, ((double) intResults.size()) / 100, 0.1);
    assertEquals("Wrong not informed distribution", 0.1, ((double) notInformedResults.size()) / 100, 0.1);
    assertNotEquals("Empty not informed values", notInformedResults.size(), 0);
  }

  @Test
  public void shouldFailIfTheDistributionIfMalformed() {
    exceptionRule.expect(RuntimeException.class);
    exceptionRule.expectMessage("all probabilities of distribution property must sum 1");
    generateRecordWithSchema("test-schemas/extensions/unions-distribution-error.json");
  }

  @Test
  public void shouldNotFailWith3ValuesInUnion() {

    Generator generator = builderWithSchema("test-schemas/extensions/3-unions-distribution.json");

    GenericRecord record;
    List<Object> results = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      record = (GenericRecord) generator.generate();
      Object col = record.get("nullable_col");
      if (col != null && isInt(col.toString())) {
        results.add(col);
      }
    }

    assertEquals("Wrong union distribution", 0.8, ((double) results.size()) / 100, 0.1);

  }

  @Test
  public void shouldCreateUniqueFieldValues() {
    Generator generator = builderWithSchema("test-schemas/extensions/unique-options.json");

    GenericRecord record;
    List<String> results = new ArrayList<>();
    for (int i = 0; i < 3; i++) {
      record = (GenericRecord) generator.generate();
      Object col = record.get("letter");
      if (col != null) {
        results.add((String) col);
      }
    }

    assertEquals("The number of generated records must be 3", 3, results.size(), 0);
    assertTrue("A result does not exists", results.contains("A"));
    assertTrue("B result does not exists", results.contains("B"));
    assertTrue("C result does not exists", results.contains("C"));
  }

  @Test
  public void shouldFailIfIsImpossibleGenerateUniqueValues() {
    exceptionRule.expect(RuntimeException.class);
    exceptionRule.expectMessage("letter field options out of stock, it could be due to the generation of more records than possible unique values");

    Generator generator = builderWithSchema("test-schemas/extensions/unique-options.json");

    GenericRecord record;
    List<String> results = new ArrayList<>();
    for (int i = 0; i < 4; i++) {
      record = (GenericRecord) generator.generate();
      Object col = record.get("letter");
      if (col != null) {
        results.add((String) col);
      }
    }
  }

  @Test
  public void shouldCreateUniqueFieldValuesWithAnOptionsFile() {
    Generator generator = builderWithSchema("test-schemas/extensions/unique-options-file.json");

    GenericRecord record;
    List<String> results = new ArrayList<>();
    for (int i = 0; i < 4308; i++) {
      record = (GenericRecord) generator.generate();
      String col = record.get("name").toString();
      if (col != null && !results.contains(col)) {
        results.add(col);
      }
    }

    assertEquals("The number of generated records must be 3", 4308, results.size(), 0);
    assertTrue("aardvark result does not exists", results.contains("aardvark"));
    assertTrue("grandfather result does not exists", results.contains("grandfather"));
    assertTrue("zucchini result does not exists", results.contains("zucchini"));
  }

  @Test
  public void shouldFailIfIsImpossibleGenerateUniqueValuesWithOptionsFile() {
    exceptionRule.expect(RuntimeException.class);
    exceptionRule.expectMessage("name field options out of stock, it could be due to the generation of more records than possible unique values");


    Generator generator = builderWithSchema("test-schemas/extensions/unique-options-file.json");

    GenericRecord record;
    List<String> results = new ArrayList<>();
    for (int i = 0; i < 4309; i++) {
      record = (GenericRecord) generator.generate();
      String col = record.get("name").toString();
      if (col != null && !results.contains(col)) {
        results.add(col);
      }
    }
  }

  @Test
  public void shouldCreateFieldWithMalformedRate() {
    Generator generator = builderWithSchema("test-schemas/extensions/malformed-distribution.json");

    GenericRecord record;
    List<Object> results = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      record = (GenericRecord) generator.generate();
      String col = (String) record.get("day_dt");
      if (Validations.isValidDate(col)) {
        results.add(col);
      }
    }

    assertEquals("Wrong malformed distribution", 0.7, ((double) results.size()) / 100, 0.1);
  }

  @Test
  public void shouldCreateFieldWithGlobalMalformedRate() {
    Generator generator = builderWithSchemaAndMalformedRate("test-schemas/extensions/malformed-without-distribution.json", 0.2);

    GenericRecord record;
    List<Object> results = new ArrayList<>();
    for (int i = 0; i < 100; i++) {
      record = (GenericRecord) generator.generate();
      String col = (String) record.get("day_dt");
      if (Validations.isValidDate(col)) {
        results.add(col);
      }
    }

    assertEquals("Wrong malformed distribution", 0.8, ((double) results.size()) / 100, 0.1);
  }
}
