package io.confluent.avro.random.generator;

import com.github.javafaker.Faker;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.Phonemetadata;
import com.google.i18n.phonenumbers.Phonenumber;
import org.apache.commons.lang3.time.DurationFormatUtils;
import org.apache.commons.text.RandomStringGenerator;
import com.googlecode.ipv6.IPv6Address;
import scala.collection.JavaConverters;
import com.telefonica.baikal.utils.Validations;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.math.BigInteger;
import java.time.*;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
import java.util.concurrent.TimeUnit;

import static java.time.format.DateTimeFormatter.*;
import static org.apache.commons.text.CharacterPredicates.DIGITS;

public class LogicalTypeGenerator {

  /**
   * The name of the attribute for specifying the rate of malformed data for this logical-type column. Must be
   * given as double.
   */
  public static final String MALFORMED_DISTRIBUTION_PROP = "malformed-rate";

  /**
   * The name of the attribute for specifying a region code for phone-number logical-types. Must be
   * given as string.
   */
  public static final String REGION_CODE_PROP = "region-code";

  /**
   * The name of the attribute for specifying a possible range of values of iso-date strings. Must be
   * given as an object.
   */
  public static final String DATE_RANGE_PROP = "range";
  /**
   * The name of the attribute for specifying the (inclusive) minimum value in a range. Must be
   * given as a iso-date string.
   */
  public static final String DATE_RANGE_PROP_START = "start";
  /**
   * The name of the attribute for specifying the (exclusive) maximum value in a range. Must be
   * given as a iso-date string.
   */
  public static final String DATE_RANGE_PROP_END = "end";

  private static final int IMEI_LENGTH = 14;
  private static final int IMSI_LENGTH = 15 - 3;

  private static final int MAX_DURATION_DAYS = 1;

  private static final PhoneNumberUtil phoneNumberUtil = PhoneNumberUtil.getInstance();

  private static final String DEFAULT_START_DATE = "2019-01-01";
  private static final String DEFAULT_END_DATE = String.format("%s-12-31", LocalDate.now().getYear());

  private static final String DEFAULT_START_DATE_TIME = DEFAULT_START_DATE + "T00:00:00Z";
  private static final String DEFAULT_END_DATE_TIME = DEFAULT_END_DATE + "T00:00:00Z";

  private static Date dateBetween(Date startInclusive, Date endExclusive) {
    long startMillis = startInclusive.getTime();
    long endMillis = endExclusive.getTime();
    long randomMillisSinceEpoch = ThreadLocalRandom
        .current()
        .nextLong(startMillis, endMillis);

    return new Date(randomMillisSinceEpoch);
  }

  private final Random random;
  private final RandomStringGenerator digitsGenerator;
  private Faker faker;

  LogicalTypeGenerator(Random random) {
    this.random = random;
    this.digitsGenerator = new RandomStringGenerator.Builder()
        .withinRange('0', 'z')
        .filteredBy(DIGITS)
        .usingRandom(random::nextInt)
        .build();
    this.faker = new Faker(this.random);
  }

  private String getRandomPhoneNumber(String regionCode) {
    String phoneNumber = "";
    try {
      Method getMetadataForRegionOrCallingCode = PhoneNumberUtil.class
          .getDeclaredMethod("getMetadataForRegionOrCallingCode", int.class, String.class);
      getMetadataForRegionOrCallingCode.setAccessible(true);
      Phonemetadata.PhoneMetadata metadata = (Phonemetadata.PhoneMetadata) getMetadataForRegionOrCallingCode.invoke(phoneNumberUtil, phoneNumberUtil.getCountryCodeForRegion(regionCode), regionCode);

      Phonenumber.PhoneNumber example = phoneNumberUtil.getExampleNumber(regionCode);
      String pattern = metadata.getFixedLine().getNationalNumberPattern();
      example.setNationalNumber(Long.parseLong(faker.regexify(pattern)));
      phoneNumberUtil.truncateTooLongNumber(example);
      phoneNumber = phoneNumberUtil.format(example, PhoneNumberUtil.PhoneNumberFormat.E164);
    } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
      System.err.println("An error occurred trying to generate random phone number");
      e.printStackTrace();
    } catch (NumberFormatException e) {
      return getRandomPhoneNumber(regionCode);
    }
    return phoneNumber;
  }

  public String random(String logicalType, Map propertiesProp) {
    switch (logicalType) {
      case "iso8601-timestamp":
      case "datetime":
        Map dateRangeProps = Optional.ofNullable(propertiesProp.get(DATE_RANGE_PROP))
            .map(m -> (Map) m)
            .orElse(new HashMap());

        java.sql.Timestamp isoDateTimeStart = Validations.tryParseDatetime((Optional.ofNullable(dateRangeProps.get(DATE_RANGE_PROP_START))
            .map(Object::toString)
            .orElse(DEFAULT_START_DATE_TIME)), false)
            .getOrElse(() -> {
              throw new IllegalArgumentException(String.format(
                  "Invalid iso date at field '%s' in %s property",
                  DATE_RANGE_PROP_START,
                  DATE_RANGE_PROP
              ));
            });
        java.sql.Timestamp isoDateTimeEnd = Validations.tryParseDatetime((Optional.ofNullable(dateRangeProps.get(DATE_RANGE_PROP_END))
            .map(Object::toString)
            .orElse(DEFAULT_END_DATE_TIME)), false)
            .getOrElse(() -> {
              throw new IllegalArgumentException(String.format(
                  "Invalid iso date at field '%s' in %s property",
                  DEFAULT_END_DATE,
                  DATE_RANGE_PROP
              ));
            });

        if (isoDateTimeStart.after(isoDateTimeEnd)) {
          throw new RuntimeException(String.format(
              "'%s' field must be strictly less than '%s' field in %s property",
              DATE_RANGE_PROP_START,
              DATE_RANGE_PROP_END,
              DATE_RANGE_PROP
          ));
        }
        return ISO_LOCAL_DATE_TIME.format(dateBetween(isoDateTimeStart, isoDateTimeEnd)
            .toInstant()
            .atOffset(ZoneOffset.UTC));
      case "duration":
        Date durationStart = new Date();
        Date durationEnd = new Date(durationStart.getTime() + TimeUnit.DAYS.toMillis(MAX_DURATION_DAYS));
        return DurationFormatUtils.formatPeriodISO(durationStart.getTime(), dateBetween(durationStart, durationEnd).getTime());
      case "time":
        LocalDateTime time = LocalDateTime.of(LocalDate.now(),
            LocalTime.of(
                random.nextInt(24), random.nextInt(60),
                random.nextInt(60), random.nextInt(999999999 + 1)
            )
        );
        return DateTimeFormatter.ISO_TIME.format(time);
      case "decimal-string":
        return String.format(Locale.US, "%.8f", random.nextDouble());
      case "phone-number":
        String regionCode = Optional.ofNullable(propertiesProp.get(REGION_CODE_PROP))
            .map(Object::toString)
            .orElseGet(() -> {
              List<String> regions = new ArrayList<>(phoneNumberUtil.getSupportedRegions());
              return regions.get(random.nextInt(regions.size()));
            });
        return getRandomPhoneNumber(regionCode);
      case "iso-date":
        dateRangeProps = Optional.ofNullable(propertiesProp.get(DATE_RANGE_PROP))
            .map(m -> (Map) m)
            .orElse(new HashMap());

        java.sql.Date isoDateStart = Validations.tryParseDate((Optional.ofNullable(dateRangeProps.get(DATE_RANGE_PROP_START))
            .map(Object::toString)
            .orElse(DEFAULT_START_DATE)))
            .getOrElse(() -> {
              throw new IllegalArgumentException(String.format(
                  "Invalid iso date at field '%s' in %s property",
                  DATE_RANGE_PROP_START,
                  DATE_RANGE_PROP
              ));
            });
        java.sql.Date isoDateEnd = Validations.tryParseDate((Optional.ofNullable(dateRangeProps.get(DATE_RANGE_PROP_END))
            .map(Object::toString)
            .orElse(DEFAULT_END_DATE)))
            .getOrElse(() -> {
              throw new IllegalArgumentException(String.format(
                  "Invalid iso date at field '%s' in %s property",
                  DEFAULT_END_DATE,
                  DATE_RANGE_PROP
              ));
            });

        if (isoDateStart.after(isoDateEnd)) {
          throw new RuntimeException(String.format(
              "'%s' field must be strictly less than '%s' field in %s property",
              DATE_RANGE_PROP_START,
              DATE_RANGE_PROP_END,
              DATE_RANGE_PROP
          ));
        }
        return ISO_LOCAL_DATE.format(dateBetween(isoDateStart, isoDateEnd)
            .toInstant()
            .atOffset(ZoneOffset.UTC));
      case "country-code-numeric":
        List<String> numericCodes = JavaConverters.seqAsJavaList(Validations.countryCodeNumeric());
        return numericCodes.get(random.nextInt(numericCodes.size()));
      case "country-code-alpha-2":
        List<String> alpha2Codes = JavaConverters.seqAsJavaList(Validations.countryCode());
        return alpha2Codes.get(random.nextInt(alpha2Codes.size()));
      case "country-code-alpha-3":
        List<String> alpha3Codes = JavaConverters.seqAsJavaList(Validations.countryCode3());
        return alpha3Codes.get(random.nextInt(alpha3Codes.size()));
      case "currency-code-alpha":
        List<String> alphaCurrency = JavaConverters.seqAsJavaList(Validations.currencyCode().toSeq());
        return alphaCurrency.get(random.nextInt(alphaCurrency.size()));
      case "currency-code-numeric":
        List<Object> numericCurrencyCode = JavaConverters
            .seqAsJavaList(Validations.currencyCodeNumeric().toSeq());
        int currencyCode = Integer.parseInt(numericCurrencyCode
            .get(random.nextInt(numericCurrencyCode.size()))
            .toString());
        return String.format("%03d", currencyCode);
      case "imei":
        return digitsGenerator.generate(LogicalTypeGenerator.IMEI_LENGTH);
      case "imsi":
        List<String> mccs = JavaConverters.seqAsJavaList(Validations.mccList());
        String mcc = mccs.get(random.nextInt(mccs.size()));
        return mcc + digitsGenerator.generate(LogicalTypeGenerator.IMSI_LENGTH);
      case "ipv4":
        Random r = new Random();
        return r.nextInt(256) + "." + r.nextInt(256) + "." + r.nextInt(256) + "." + r.nextInt(256);
      case "ipv6":
        Random r2 = new Random();
        return IPv6Address.fromBigInteger(BigInteger.valueOf(r2.nextInt(99999))).toString();
      case "time-zone":
        List<String> timezones = JavaConverters.seqAsJavaList(Validations.availableTimeZones());
        return timezones.get(random.nextInt(timezones.size()));
      default:
        throw new IllegalArgumentException("Unsupported logical type: " + logicalType);
    }
  }

  public Double randomDouble(String logicalType, Map propertiesProp) {
    switch (logicalType) {
      case "latitude":
        return -90 + random.nextInt(180)  + random.nextDouble();
      case "longitude":
        return -180 + random.nextInt(360) + random.nextDouble();
      default:
        throw new IllegalArgumentException("Unsupported logical type: " + logicalType);
    }
  }

}
