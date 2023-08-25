#!/usr/bin/env bash

trap ctrl_c INT

function ctrl_c() {
    echo "Stopping program... "
    for pid in ${pids[*]}; do
      echo "Killing subprocess $pid"
      kill -9 $pid
    done
    echo "Stop program."
}

ARG_DEFS=(
  "--dataset-id=(.*)"
  "--dataset-version=(.*)"
  "--records=(.*)"
  "--datasets-in=(.*)"
  "--datasets-extensions-in=(.*)"
  "--out=(.*)"
  "[--file-prefix=(.*)]"
  "[--num-files=(.*)]"
  "[--show=(true|false)]"
  "[--max-threads=(.*)]"
  "[--init-iteration=(.*)]"
  "[--start-date=(.*)]"
  "[--end-date=(.*)]"
  "[--extension-flavour=(.*)]"
  "[--not-informed=(true|false)]"
  "[--malformed-column-rate=(.*)]"
)

function init() {
  export SHOW=${SHOW:-false}
  export NUM_FILES=${NUM_FILES:-1}
  export MAX_THREADS=${MAX_THREADS:-1}
  export INIT_ITERATION=${INIT_ITERATION:-0}
  export START_DATE=${START_DATE:-$(date --iso-8601=date)}
  export START_DATE=$(date -I -d "$START_DATE") || (echo "worng start date format"; exit -1)
  export END_DATE=${END_DATE:-$(date -I -d "${START_DATE} + 1 day")}
  export END_DATE=$(date -I -d "$END_DATE") || (echo "worng end date format"; exit -1)
  export EXTENSION_FLAVOUR=${EXTENSION_FLAVOUR:-""}
  export FILE_PREFIX=${FILE_PREFIX:-"1"}
  export NOT_INFORMED=${NOT_INFORMED:-"false"}
  export MALFORMED_COLUMN_RATE=${MALFORMED_COLUMN_RATE:-""}
  export OUT=${OUT:-"datasets-out"}
}

function run() {
  local jar_location=$(ls app/build/libs/avro-random-generator-*-all.jar)

  local out_dir="${OUT}/${DATASET_ID}/${DATASET_VERSION}"
  rm -rf $out_dir || true
  mkdir -p $out_dir

  local out_schemas_dir="$out_dir/schemas"
  rm -rf ${out_schemas_dir} || true
  mkdir -p ${out_schemas_dir}

  local schema_orig=$(cat ${DATASETS_IN}/${DATASET_ID}/v${DATASET_VERSION}/${DATASET_ID}_${DATASET_VERSION}.avsc)
  local schema_extension=$(cat ${DATASETS_EXTENSIONS_IN}/${DATASET_ID}/v${DATASET_VERSION}/extensions${EXTENSION_FLAVOUR}.json || (true ; echo "{}"))
  local merged_schema_path="${out_schemas_dir}/merged_schema.json"
  python3 lib/merge-schemas.py -s "$schema_orig" -e "$schema_extension" --out "${merged_schema_path}"

  export DATE_RANGE_START="${START_DATE}"
  while [ "${DATE_RANGE_START}" != "${END_DATE}" ]; do
    export DATE_RANGE_END=$(date -I -d "${DATE_RANGE_START} + 1 day")

    local counter=0
    local current_threads=0
    while [  $counter -lt $NUM_FILES ]; do

      if [ "${SHOW}" = "true" ]; then
        out="${out_dir}/${DATASET_ID}.${FILE_PREFIX}.${counter}_${DATE_RANGE_START}.json"
        touch $out
        ARGS="-j -p -o ${out}"
      else
        out="${out_dir}/${DATASET_ID}.${FILE_PREFIX}.${counter}_${DATE_RANGE_START}.avro"
        touch $out
        ARGS="-b -o ${out}"
      fi

      export ITERATION_STEP="$(( ${RECORDS} * ${counter} + ${INIT_ITERATION} ))"
      envsubst < ${merged_schema_path} > ${out_schemas_dir}/schema_${counter}_${DATE_RANGE_START}.json

      local extra_args=""
      if [ -n "${MALFORMED_COLUMN_RATE}" ]; then
        extra_args=" --malformed-column-rate=${MALFORMED_COLUMN_RATE}"
      fi

      java -XX:+UseContainerSupport -XX:MaxRAMPercentage=75 -jar ${jar_location} -f ${out_schemas_dir}/schema_${counter}_${DATE_RANGE_START}.json -i ${RECORDS} ${ARGS} --not-informed ${NOT_INFORMED}"${extra_args}" &
      pids[$counter]=$!
      echo "Generating ${out} in background, current thread [${pids[current_threads]}]"

      local current_threads=$(( current_threads + 1 ))

      if [ "${current_threads}" = "${MAX_THREADS}" ]; then
        for index in "${!pids[@]}"; do
          if [ "${pids[index]}" != "" ]; then
            wait ${pids[index]}
            local current_threads=$(( current_threads - 1 ))
            pids[$index]=""
            break
          fi
        done
      fi

      local counter=$(( counter + 1 ))
    done

    export DATE_RANGE_START="${DATE_RANGE_END}"
  done
}

source $(dirname $0)/../base.inc
