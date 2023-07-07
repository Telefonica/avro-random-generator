package io.confluent.avro.random.generator;

import com.telefonica.baikal.utils.Validations;
import org.apache.avro.generic.GenericRecord;
import org.geotools.data.geojson.GeoJSONReader;
import org.junit.Test;
import org.locationtech.jts.geom.Geometry;

import java.io.IOException;

import static io.confluent.avro.random.generator.util.ResourceUtil.builderWithSchema;
import static io.confluent.avro.random.generator.util.ResourceUtil.generateRecordWithSchema;
import static org.junit.Assert.*;


public class LogicalTypeGeneratorTest {

  @Test
  public void shouldCreateValidISODate() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/iso-date.json");

    String field = "day_dt";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid iso-date: " + value, Validations.isValidDate(value));
  }

  @Test
  public void shouldCreateValidISODateInRange() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/iso-date-range.json");

    String field = "day_dt";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid iso-date: " + value, Validations.isValidDate(value));
  }


  @Test
  public void shouldCreateValidDateTime() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/datetime.json");

    String field = "interaction_tm";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid datetime: " + value, Validations.isValidDatetime(value));
  }

  @Test
  public void shouldCreateValidDeprecatedIso8601DateTime() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/iso8601-timestamp.json");

    String field = "interaction_tm";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid datetime: " + value, Validations.isValidDatetime(value));
  }

  @Test
  public void shouldCreateValidPhoneNumber() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/phone-number.json");

    String field = "caller_phone_with_prefix_id";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid phone number: " + value, Validations.isValidPhoneNumber(value));
  }

  @Test
  public void shouldCreateMultipleValidPhoneNumber() {
    Generator generator = builderWithSchema("test-schemas/logical-types/phone-number.json");
    String field = "caller_phone_with_prefix_id";
    for (int i = 0; i < 1000; i++) {
      GenericRecord record = (GenericRecord) generator.generate();
      assertNotNull(record.get("caller_phone_with_prefix_id"));
      String value = record.get(field).toString();
      System.out.println("Generated value is: " + value);
      assertTrue("Invalid phone number: " + value, Validations.isValidPhoneNumber(value));
    }
  }

  @Test
  public void shouldCreateRandomPhoneNumber() {
    Generator generator = builderWithSchema("test-schemas/logical-types/phone-number-region.json");

    GenericRecord record1 = (GenericRecord) generator.generate();
    GenericRecord record2 = (GenericRecord) generator.generate();
    String field = "caller_phone_with_prefix_id";
    assertNotNull(record1.get(field));
    assertNotNull(record2.get(field));
    String value1 = record1.get(field).toString();
    String value2 = record2.get(field).toString();
    System.out.println("Generated value 1 is: " + value1);
    System.out.println("Generated value 2 is: " + value2);

    assertNotEquals(value1, value2);
  }

  @Test
  public void shouldCreateValidPhoneNumberInSpecificRegion() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/phone-number-region.json");

    String field = "caller_phone_with_prefix_id";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid phone number: " + value, Validations.isValidPhoneNumber(value));
    assertTrue("Invalid ES phone number: " + value, value.startsWith("+34"));
  }

  @Test
  public void shouldCreateValidCountryCodeNumeric() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/country-code-numeric.json");

    String field = "country";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid country code: " + value, Validations.isValidCountryCodeNumeric(value));
  }

  @Test
  public void shouldCreateValidCountryCodeAlpha2() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/country-code-alpha2.json");

    String field = "country";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid country code alpha-2: " + value, Validations.isValidCountryCode2(value));
  }

  @Test
  public void shouldCreateValidCountryCodeAlpha3() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/country-code-alpha3.json");

    String field = "country";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid country code alpha-3: " + value, Validations.isValidCountryCode3(value));
  }

  @Test
  public void shouldCreateValidCurrencyCodeNumeric() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/currency-code-numeric.json");

    String field = "currency";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid currency code: " + value, Validations.isValidCurrencyNumeric(value));
  }

  @Test
  public void shouldCreateValidCurrencyCodeAlpha() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/currency-code-alpha.json");

    String field = "currency";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid currency code: " + value, Validations.isValidCurrency(value));
  }

  @Test
  public void shouldCreateValidIMEI() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/imei.json");

    String field = "imei";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid imei: " + value, Validations.isValidIMEINumber(value));
  }

  @Test
  public void shouldCreateValidIMSI() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/imsi.json");

    String field = "imsi";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid imsi: " + value, Validations.isValidIMSINumber(value));
  }

  @Test
  public void shouldCreateValidDuration() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/duration.json");

    String field = "call_duration";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid call_duration: " + value, Validations.isValidDuration(value));
  }

  @Test
  public void shouldCreateValidTime() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/time.json");

    String field = "time";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid time: " + value, Validations.isValidTime(value));
  }

  @Test
  public void shouldCreateValidDecimalString() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/decimal-string.json");

    String field = "number";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid decimal-string: " + value, Validations.tryParseDecimal(value).nonEmpty());
  }

  @Test
  public void shouldCreateValidTimezone() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/timezone.json");

    String field = "timezone";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid timezone: " + value, Validations.isValidTimeZone(value));
  }

  @Test
  public void shouldCreateValidIPv4() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/ipv4.json");

    String field = "ip_address";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid ip_address v4: " + value, Validations.isValidIPv4(value));
  }

  @Test
  public void shouldCreateValidIPv6() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/ipv6.json");

    String field = "ip_address";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid ip_address v6: " + value, Validations.isValidIPv6(value));
  }

  @Test
  public void shouldCreateValidLatitude() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/latitude.json");

    String field = "latitude";
    assertNotNull(record.get(field));
    Double value = Double.valueOf(record.get(field).toString());
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid latitude: " + value, Validations.isValidLatitude(value));
  }

  @Test
  public void shouldCreateValidLongitude() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/longitude.json");

    String field = "longitude";
    assertNotNull(record.get(field));
    Double value = Double.valueOf(record.get(field).toString());
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid latitude: " + value, Validations.isValidLongitude(value));
  }

  @Test
  public void shouldCreateValidLGeometry() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/geometry.json");
    String field = "geometry";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid geometry: " + value, Validations.isValidGeometry(value));
  }

  @Test
  public void shouldCreateValidLFeature() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/feature.json");
    String field = "feature";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid feature: " + value, Validations.isValidFeature(value));
  }

  @Test
  public void shouldCreateValidLFeatureCollection() {
    GenericRecord record = generateRecordWithSchema("test-schemas/logical-types/feature-collection.json");
    String field = "featureCollection";
    assertNotNull(record.get(field));
    String value = record.get(field).toString();
    System.out.println("Generated value is: " + value);
    assertTrue("Invalid feature-collection: " + value, Validations.isValidFeatureCollection(value));
  }

}
