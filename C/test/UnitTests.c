#include "gtest/gtest.h"
#include "../inc/SC_he100.h"
#include "../inc/timer.h"

class Helium_100_Test : public ::testing::Test
{
    protected
    virtual void SetUp()
    {

    }

    unsigned char he100_noop_expected_value = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43};
    unsigned char he100_soft_reset_expected_value = {0x48,0x65,0x10,0x02,0x00,0x00,0x12,0x46};
    unsigned char he100_fast_set_pa_expected_value = {0x48,0x65,0x10,0x20,0x00,0x01,0x31,0xA1,0x03,0x06,0x0C};
}

// will be tested in following tests, but isolate some 
// test null bytes, passing wrong length, etc
struct HE100_checksum HE100_fletcher16 (char *data, size_t bytes);

// Pass the function some data and check against expected result
unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

// verify transmit data preparation bytes - with "Test Payload" message
void test_transmitData_HE100_prepareTransmission()
{
    size_t transmit_data_payload_length = 12;
    unsigned char test_payload[transmit_data_payload_length] = "Test Payload";
    unsigned char transmit_data_command[2] = {CMD_TRANSMIT, CMD_TRANSMIT_DATA};
    unsigned char transmit_data_expected_value[transmit_data_payload_length+WRAPPER_LENGTH] = {0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,0x,};
    ASSERT_EQ(
        HE100_prepareTransmission(transmit_data_payload, transmit_data_length, transmit_data_command),
        transmit_data_expected_value
    );
}

// verify NOOP preparation bytes
void test_noop_HE100_prepareTransmission()
{
    size_t noop_payload_length = 0;
    unsigned char noop_payload[noop_payload_length] = {0};
    unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};
    ASSERT_EQ(
        HE100_prepareTransmission(noop_payload, 0, noop_command),
        he100_noop_expected_value
    );
}

// verify Soft Reset preparation bytes
void test_softReset_HE100_prepareTransmission()
{
    size_t soft_reset_payload_length = 0;
    unsigned char soft_reset_payload[soft_reset_payload_length] = {0};
    unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
    ASSERT_EQ(
        HE100_prepareTransmission(soft_reset_payload, 0, soft_reset_command),
        he100_soft_reset_expected_value
    );
}

// verify Fast Set PA preparation bytes
void test_fastSetPA_HE100_prepareTransmission()
{
    unsigned char fast_set_pa_payload[1] = {11};
    unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    
    ASSERT_EQ(    
        HE100_prepareTransmission(fast_set_pa_payload, 1, fast_set_pa_command),
        he100_fast_set_pa_expected_value
    );
}

// Function should be tested for a string of bytes similar to expected but invalid

// wrong length 
void test_HE100_validateFrame(response, length)
{

}

// invalid checksum
void test_HE100_validateFrame(response, length)
{

}

// invalid command
void test_HE100_validateFrame(response, length)
{

}
