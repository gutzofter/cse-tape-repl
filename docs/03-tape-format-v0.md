# Scenario Tape v0

[TAPE_HEADER][CODE]

CODE is instruction stream: [OP:u8][FLAGS:u8][LEN:u16][BODY:LEN]

Opcodes:
- SET (0x01)
- EMIT (0x02)
- WAIT (0x03)
- ASSERT_PRESSURE (0x04)
- END (0x05)
