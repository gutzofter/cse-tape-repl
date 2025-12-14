# Scenario Compiler Input v0

Statements:
- SCENARIO <id>
- SET <KEY>=<VALUE>
- EMIT <event_type> HEX <hexbytes>
- EMIT <event_type> STR "<string>"
- WAIT <n> ms
- ASSERT_PRESSURE <LAT|INS|QUE> <OK|WARN|FAIL> [SCOPE LAST|WORST] [TIMEOUT <n> ms]
- END <PASS|WARN|FAIL>
