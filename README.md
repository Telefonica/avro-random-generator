# Arg: Avro Random Generator

__NOTE: Building is required to run the program.__

## What does it do?

#### The boring stuff

Arg reads a schema through either stdin or a CLI-specified file and
generates random data to fit it.

Arg can output data in either JSON or binary format, and when outputting
in JSON, can either print in compact format (one instance of spoofed
data per line) or pretty format.

Arg can output data either to stdout or a file. After outputting all of
its spoofed data, Arg prints a single newline.

The number of instances of spoofed data can also be specified; the
default is currently 1.

#### The cool stuff

Arg also allows for special annotations in the Avro schema it spoofs
that narrow down the kind of data produced. For example, when spoofing
a string, you can currently either specify a length that the string
should be (or one or both of a minimum and maximum that the length
should be), a list of possible strings that the string should come from,
or a regular expression that the string should adhere to. These
annotations are specified inside the schema that Arg spoofs, as parts of
a JSON object with an attribute name of "arg.properties".

These annotations are specified as JSON properties in the schema that
Arg spoofs. They should not collide with any existing properties, or
cause any issues if present when the schema is used with other programs.

## Building

### Local

```
./gradlew build
```

### Docker

```
./lanuza/pipelines/build.sh
```

> EXTENSION_FLAVOUR environment var can be used in order switch between different datasets' extensions 

## CLI Usage

```
lanuza/pipelines/run.sh lanuza/scripts/build-data.sh --dataset-id=Full_LogicalType_Dataset --dataset-version=1.0.0 --records=20 --datasets-in=samples/datasets --datasets-extensions-in=samples/extensions --out=output
```

The output folder is always relative to this project.

In order to introduce your own schemas please check samples folder and replicate datasets and extensions scaffolding.

## Schema annotations

### Annotation types

The following annotations are currently supported:
+ __options:__ Either a JSON array of possibilities that the data for
spoofing this schema should come from, or a JSON object that conforms to
the following format: `{"file": <file>, "encoding": <encoding>}` (both
fields must be specified). If given as an object, a list of data will be
read from the file after decoding with the specified format (currently
"json" and "binary" are the only supported values, and "binary" may be
somewhat buggy).
+ __iteration:__ A JSON object that conforms to the following format:
`{"start": <start>, "restart": <restart>, "step": <step>, "initial": 
<initial> }` ("start" has to be specified, but "restart", "step", 
and "initial" do not). If provided with a
numeric schema, ensures that the first generated value will be equal to
&lt;initial&gt; (or &lt;start&gt; if &lt;initial&gt; is not specified),
and successive values will increase by &lt;step&gt;, wrapping around at 
&lt;restart&gt; back to &lt;start&gt;; &lt;step&gt; will default to 1 if
&lt;restart&gt; is greater than &lt;start&gt;, will default to -1 if
&lt;restart&gt; is less than &lt;start&gt;, and an error will be thrown
if &lt;restart&gt; is equal to &lt;start&gt;. If provided with a boolean
schema, only &lt;start&gt; may be specified; the resulting values will
begin with &lt;start&gt; and alternate from `true` to `false` and from
`false` to `true` from that point on.
> ITERATION_STEP environment var can be used as script argument
+ __range:__ A JSON object that conforms to the following formats:
    - `{"min": <min>, "max": <max>}` (at least one of "min" or "max" must be
    specified). If provided, ensures that the generated number will be
    greater than or equal to &lt;min&gt; and/or strictly less than &lt;max&gt;.
    - `{"start": <iso-date>, "end": <iso-date>}` (at least one of "start" or "end" must be
    specified). If provided, ensures that the generated date will be
    greater than or equal to &lt;start&gt; and/or strictly less than &lt;end&gt;. 
    > Only applicable for the logical-type iso-date
    - `{"start": <iso-datetime>, "end": <iso-datetime>}` (at least one of "start" or "end" must be
    specified). If provided, ensures that the generated datetime will be
    greater than or equal to &lt;start&gt; and/or strictly less than &lt;end&gt;.
    > Only applicable for the logical-type datetime
    > DATE_RANGE_START and DATE_RANGE_END environment var can be used as script arguments
+ __length:__ Either a JSON number or a JSON object that conforms to the
following format: `{"min": <min>, "max": <max>}` (at least one of "min"
or "max" must be specified, and if present, values for either must be
numbers). __Defaults to `{"min": 8, "max": 16}`__.
+ __regex:__ A JSON string describing a regular expression that a string
should conform to.
+ __kind:__ A JSON string describing the kind of string:
    - __uuid__: a random uuid will be generated.
    - __email__: a random email will be generated.
    - __name__: a random person name will be generated.
+ __prefix:__ A JSON string containing a prefix that should be prepended
to the beginning of a string.
+ __suffix:__ A JSON string containing a suffix that should be appended
to the end of a string.
+ __keys:__ A JSON object containing any of the above which is used to
describe the kind of data that should be used for generating keys for
spoofed maps.
+ __odds:__ A JSON float between 0.0 and 1.0 that, when specified with
a boolean schema, specifies the likelihood that the generated value is
`true`.
+ __position:__ A Integer value that indicate the position of the union to select.
+ __distribution:__ A JSON object that conforms to the following formats:
    - `{"0": 0.3, "1": 0.7}` all the union's positions with its probability
+ __region-code:__ A JSON string containing phone number region code ("ES", "EN"....).
+ __unique:__   A JSON boolean describing if the generated value must be unique or not. Note that if you are mixing this property
with fixed options the amount of generated records must be less or equals to the available options.
+ __malformed-rate:__ A JSON number between 0.0 and 1.0 representing the percent of malformed data (only applies to logical-types).

The following schemas support the following annotations:

### Logical Types

4PF logical-types integration, random logical-type data will be generated according with this format. Check the supported [logical types here](https://developers.baikalplatform.com/docs/4.1/datasets/avro-4p.html)


### Primitives

#### null
+ options (although there can only be one option)

#### boolean
+ options
+ iteration
+ odds
+ unique

#### int
+ options
+ range
+ iteration
+ unique

#### long
+ options
+ range
+ iteration
+ unique

#### float
+ options
+ range
+ iteration
+ unique

#### double
+ options
+ range
+ iteration
+ unique

#### bytes
+ options
+ length
+ unique

#### string
+ options
+ length*
+ regex*
+ kind
+ range (only for date based logical types)
+ region-code (only for phone-number logical type)
+ unique
+ malformed-rate (only for date based logical types)

__*Note:__ If both length and regex are specified for a string,
the length property (if a JSON number) becomes a minimum length for the
string

### Complex

#### array
+ options
+ length
+ unique (unique generated values inside array)

#### enum
+ options
+ unique

#### fixed
+ options

#### map
+ options
+ length
+ keys
+ unique (unique generated values inside map)

#### record
+ options

#### union
+ options
+ position
+ distribution
+ unique (unique generated values by union position)

### Example schemas

Example schemas are provided in the test/schemas directory. Here are a
few of them:

#### enum.json

```
{
  "name": "enum_comp",
  "type": "enum",
  "symbols": ["PRELUDE", "ALLEMANDE", "COURANTE", "SARABANDE", "MINUET", "BOURREE", "GAVOTTE", "GIGUE"]
}
```

A non-annotated schema. The resulting output will just be a random
enum chosen from the symbols list.

### regex.json
```
{
  "type": "record",
  "name": "regex_test",
  "fields":
    [
      {
        "name": "no_length_property",
        "type":
          {
            "type": "string",
            "arg.properties": {
              "regex": "[a-zA-Z]{5,15}"
            }
          }
      },
      {
        "name": "number_length_property",
        "type":
          {
            "type": "string",
            "arg.properties": {
              "regex": "[a-zA-Z]*",
              "length": 10
            }
          }
      },
      {
        "name": "min_length_property",
        "type":
          {
            "type": "string",
            "arg.properties": {
              "regex": "[a-zA-Z]{0,15}",
              "length":
                {
                  "min": 5
                }
            }
          }
      },
      {
        "name": "max_length_property",
        "type":
          {
            "type": "string",
            "arg.properties": {
              "regex": "[a-zA-Z]{5,}",
              "length":
                {
                  "max": 16
                }
            }
          }
      },
      {
        "name": "min_max_length_property",
        "type":
          {
            "type": "string",
            "arg.properties": {
              "regex": "[a-zA-Z]*",
              "length":
                {
                  "min": 5,
                  "max": 16
                }
            }
          }
      }
    ]
}
```

An annotated record schema, with a variety of string fields. Each field
has its own way of preventing the specified string from becoming too
long, either via the length annotation or the regex annotation.

### options-file.json

```
{
  "type": "record",
  "name": "sentence",
  "fields": [
    {
      "name": "The",
      "type": {
        "type": "string",
          "arg.properties": {
            "options": [
              "The"
            ]
          }
      }
    },
    {
      "name": "noun",
      "type": {
        "type": "string",
        "arg.properties": {
          "options": {
            "file": "test/schemas/nouns-list.json",
            "encoding": "json"
          }
        }
      }
    },
    {
      "name": "is",
      "type": {
        "type": "string",
        "arg.properties": {
          "options": [
            "is",
            "was",
            "will be",
            "is being",
            "was being",
            "has been",
            "had been",
            "will have been"
          ]
        }
      }
    },
    {
      "name": "degree",
      "type": {
        "type": "string",
        "arg.properties": {
          "options": [
            "not at all",
            "slightly",
            "somewhat",
            "kind of",
            "pretty",
            "very",
            "entirely"
          ]
        }
      }
    },
    {
      "name": "adjective",
      "type": {
        "type": "string",
        "arg.properties": {
          "options": {
            "file": "test/schemas/adjectives-list.json",
            "encoding": "json"
          }
        }
      }
    }
  ]
}
```

A record schema that draws its content from two files, 'nouns-list.json'
and 'adjectives-list.json' to construct a primitive sentence. The script
must be run from the repository base directory in order for this schema
to work with it properly due, to the relative paths of the files.

### options.json

```
{
  "type": "record",
  "name": "options_test_record",
  "fields": [
    {
      "name": "array_field",
      "type": {
        "type": "array",
        "items": "string",
        "arg.properties":
          {
            "options": [
              [
                "Hello",
                "world"
              ],
              [
                "Goodbye",
                "world"
              ],
              [
                "We",
                "meet",
                "again",
                "world"
              ]
            ]
          }
      }
    },
    {
      "name": "enum_field",
      "type": {
        "type": "enum",
        "name": "enum_test",
        "symbols": [
          "HELLO",
          "HI_THERE",
          "GREETINGS",
          "SALUTATIONS",
          "GOODBYE"
        ],
        "arg.properties":
          {
            "options": [
              "HELLO",
              "SALUTATIONS"
            ]
          }
      }
    },
    {
      "name": "fixed_field",
      "type": {
        "type": "fixed",
        "name": "fixed_test",
        "size": 2,
        "arg.properties":
          {
            "options": [
              "\u0034\u0032",
              "\u0045\u0045"
            ]
          }
      }
    },
    {
      "name": "map_field",
      "type": {
        "type": "map",
        "values": "int",
        "arg.properties":
          {
            "options": [
              {
                "zero": 0
              },
              {
                "one": 1,
                "two": 2
              },
              {
                "three": 3,
                "four": 4,
                "five": 5
              },
              {
                "six": 6,
                "seven": 7,
                "eight": 8,
                "nine": 9
              }
            ]
          }
      }
    },
    {
      "name": "map_key_field",
      "type": {
        "type": "map",
        "values": {
          "type": "int",
          "arg.properties": {
            "options": [
              -1,
              0,
              1
            ]
          }
        },
        "arg.properties": {
          "length": 10,
          "keys": {
            "options": [
              "negative",
              "zero",
              "positive"
            ]
          }
        }
      }
    },
    {
      "name": "record_field",
      "type": {
        "type": "record",
        "name": "record_test",
        "fields": [
          {
            "name": "month",
            "type": "string"
          },
          {
            "name": "day",
            "type": "int"
          }
        ],
        "arg.properties": {
          "options": [
            {
              "month": "January",
              "day": 2
            },
            {
              "month": "NANuary",
              "day": 0
            }
          ]
        }
      }
    },
    {
      "name": "union_field",
      "arg.properties": {
        "position": 2
      }
      "type": [
        "null",
        {
          "type": "boolean",
          "arg.properties": {
            "options": [
              true
            ]
          }
        },
        {
          "type": "int",
          "arg.properties": {
            "options": [
              42
            ]
          }
        },
        {
          "type": "long",
          "arg.properties": {
            "options": [
              4242424242424242
            ]
          }
        },
        {
          "type": "float",
          "arg.properties": {
            "options": [
              42.42
            ]
          }
        },
        {
          "type": "double",
          "arg.properties": {
            "options": [
              42424242.42424242
            ]
          }
        },
        {
          "type": "bytes",
          "arg.properties": {
            "options": [
              "NDI="
            ]
          }
        },
        {
          "type": "string",
          "arg.properties": {
            "options": [
              "Forty-two"
            ]
          }
        }
      ]
    }
  ]
}
```

A schema where every field is annotated with an example usage of the
options annotation, as well as an example of the keys annotation.

#### union-distribution.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
  [
    {
      "name": "nullable_col",
      "type": [
        "null",
        {
          "type": "string"
        }
      ],
      "arg.properties": {
        "distribution": {
          "0": 0.3,
          "1": 0.7
        }
      }
    }
  ]
}
```

Note that all enum positions with each distribution must be specified.

#### union-position.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
  [
    {
      "name": "nullable_col",
      "type": [
        "null",
        {
          "type": "string"
        }
      ],
      "arg.properties": {
        "position": 1
      }
    }
  ]
}
```

#### phone-number.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
    [
      {
        "name": "caller_phone_with_prefix_id",
        "type": {
          "type": "string",
          "logicalType": "phone-number",
          "arg.properties": {
            "region-code": "ES"
          }
        },
        "doc": "Phone number (in the case of contact via phone call), WITH_INTERNATIONAL_PREFFIX, used in the contact action. It may not coincide with the phone number associated to the line or SUBSCRIBER_ID/CUSTOMER_ID"
      }
    ]
}
```

#### kind.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
    [
      {
        "name": "email",
        "type": {
          "type": "string",
          "arg.properties": {
            "kind": "email"
          }
        }
      }
    ]
}
```


#### iso-date-range.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
    [
      {
        "name": "day_dt",
        "type": {
          "logicalType": "iso-date",
          "type": "string",
          "arg.properties": {
            "range": {
              "start": "2020-01-01",
              "end": "2020-02-01"
            }
          }
        },
        "doc": "Year, month and day of the data (snapshot of the Customer data)"
      }
    ]
}
```


#### unique-options.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
    [
      {
        "name": "letter",
        "type": {
          "type": "string",
          "arg.properties": {
            "unique": true,
            "options": [
              "A",
              "B",
              "C"
            ]
          }
        }
      }
    ]
}
```



#### malformed-rate.json

```
{ "type": "record",
  "name": "logicals",
  "namespace": "io.confluent.avro.random.generator",
  "fields":
    [
      {
        "name": "day_dt",
        "type": {
          "logicalType": "iso-date",
          "type": "string",
          "arg.properties": {
            "malformed-rate": 0.3
          }
        },
        "doc": "Year, month and day of the data (snapshot of the Customer data)"
      }
    ]
}
```

## Working with malformed data

Malformed data for all column can be generated using the `--malformed-column-rate=0.1` build script option. With this mode all the columns will have the desired percentage of malformed data.

Example:
```
bash lanuza/pipelines/run.sh lanuza/scripts/build-data.sh --dataset-id=Full_LogicalType_Dataset_With_Complex_Types --dataset-version=1.0.0 --records=50 --datasets-in=samples/datasets --datasets-extensions-in=samples/extensions --out=output --num-files=1 --max-threads=1 --start-date=2023-04-24 --end-date=2023-04-27 --malformed-column-rate=0.1 
```


## Working with not informed data

Not informed data can be generated using the `--not-informed` build script option. With this mode the not-informed schema will be generated on the fly and every column will have a 0.1 rate of not-informed data.
Additionally, the not-informed column rate can be configured using `--not-informed-column-rate=0.05`.

Example:
```
bash lanuza/pipelines/run.sh lanuza/scripts/build-data.sh --dataset-id=Full_LogicalType_Dataset_With_Complex_Types --dataset-version=1.0.0 --records=50 --datasets-in=samples/datasets --datasets-extensions-in=samples/extensions --out=output --num-files=1 --max-threads=1 --start-date=2023-04-24 --end-date=2023-04-27 --extension-flavour=.full-malformed --not-informed=true
```
