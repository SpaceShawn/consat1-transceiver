// will be tested in following tests, but isolate some 
// test null bytes, passing wrong length, etc
struct HE100_checksum HE100_fletcher16 (char *data, size_t bytes);

// Pass the function some data and check against expected result
unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

void test_transmitData_HE100_prepareTransmission()
{
    size_t transmit_data_payload_length = 12;
    unsigned char test_payload[transmit_data_payload_length] = "Test Payload";
    unsigned char transmit_data_command[2] = {CMD_TRANSMIT, CMD_TRANSMIT_DATA};
    unsigned char transmit_data_expected_value[payload_length+10] = {0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,};
    assert(
        HE100_prepareTransmission(transmit_data_payload, transmit_data_length, transmit_data_command),
        transmit_data_expected_value;
    );
}

void test_noop_HE100_prepareTransmission()
{
    size_t soft_reset_payload_length = 0;
    unsigned char soft_reset_payload[soft_reset_payload_length] = {0};
    unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
    unsigned char soft_reset_expected_value[reset_payload_length+10] = {0x,0x,0x,0x,0x,0x,0x,0x};
    assert(
        HE100_prepareTransmission(soft_reset_payload, 0, soft_reset_command),
        soft_reset_expected_value;
    );
}

void test_softReset_HE100_prepareTransmission()
{
    size_t soft_reset_payload_length = 0;
    unsigned char soft_reset_payload[soft_reset_payload_length] = {0};
    unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
    unsigned char soft_reset_expected_value[reset_payload_length+10] = {0x,0x,0x,0x,0x,0x,0x,0x};
    assert(
        HE100_prepareTransmission(soft_reset_payload, 0, soft_reset_command),
        soft_reset_expected_value;
    );
}

// Function should be tested for a string of bytes similar to expected but invalid
// e.g. wrong length, 
int HE100_validateResponse (char *response, size_t length);
