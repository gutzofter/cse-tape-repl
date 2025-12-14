#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum { RESULT_PASS=0, RESULT_WARN=1, RESULT_FAIL=2 } ScenarioResult;
typedef enum { RUNMODE_VIRTUAL=0, RUNMODE_REAL=1 } RunMode;

typedef enum { PRESSURE_OK=0, PRESSURE_WARN=1, PRESSURE_FAIL=2 } PressureState;

typedef enum { METRIC_LATENCY_EXEC_US=1, METRIC_INSTR_PER_EVENT=2, METRIC_QUEUE_DEPTH=3 } PressureMetricId;

typedef enum {
  CFG_EVENT_TIME_BUDGET_US = 1,
  CFG_INSTR_BUDGET         = 2,
  CFG_LANE_CAP_DEFAULT     = 3,
  CFG_LATENCY_MODEL_ID     = 10,
  CFG_INSTR_MODEL_ID       = 11,
  CFG_QUEUE_MODEL_ID       = 12,
  CFG_DOCTOR_PERIOD_MS     = 20
} ConfigKey;

typedef enum { VAL_U32=1, VAL_U16=2, VAL_BYTES=5 } ValueType;
