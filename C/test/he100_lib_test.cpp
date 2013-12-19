#include "gtest/gtest.h"
#include "../src/SC_he100.h"
#include "../src/timer.h"

class Helium_100_Test : public ::testing::Test
{
    protected:
    virtual void SetUp() { }
    
    size_t z; // assert loop index
};

// will be tested in following tests, but isolate some 
// test null bytes, passing wrong length, etc
struct HE100_checksum HE100_fletcher16 (char *data, size_t bytes);

// Pass the function some data and check against expected result
unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

// Test the fletcher checksum
TEST_F(Helium_100_Test, GoodChecksum)
{

    //    unsigned char *checksum_bytes[97] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghjklmnopqrstuvwxyz~!@#$%^&*()_+`1234567890-={}\\|:\"<>?[];',./";
    
    //ACTUAL
    unsigned char checksum_bytes[56] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghjklmnopqrstuvwxyz~!@";
    HE100_checksum checksum_result = HE100_fletcher16(checksum_bytes,56);
    
    //EXPECTED http://www.lammertbies.nl/comm/info/crc-calculation.html
    // NOTE: reality on the HE100 is subjective and unexpected
    HE100_checksum expected_result;
    expected_result.sum1 = 0x27;
    expected_result.sum2 = 0xD3;

    ASSERT_EQ(
        checksum_result.sum1,
        expected_result.sum1 
    ); 
    ASSERT_EQ(
        checksum_result.sum2,
        expected_result.sum2
    );
}

// Test writing to the helium device
TEST_F(Helium_100_Test, GoodWrite)
{
    const static int fdin = 1; // fake file descriptor to simulate HE100
    unsigned char write_test[8] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43}; // 8
    int write_result = HE100_write(fdin, write_test, 8);
    
    ASSERT_EQ(
        write_result,
        1
    );
}

// Test a bogus byte sequence
TEST_F(Helium_100_Test, Caught)
{
    int expected_reference_result = -1;
    int actual_reference_result;

    unsigned char bad_sequence[8] = {0x47,0x65,0x10,0x01,0x00,0x00,0x11,0x43};
    actual_reference_result = HE100_referenceByteSequence(bad_sequence, 0);
    ASSERT_EQ(actual_reference_result,expected_reference_result);

    bad_sequence[0] = 0x48; bad_sequence[1] = 0x64;
    actual_reference_result = HE100_referenceByteSequence(bad_sequence, 1); 
    ASSERT_EQ(actual_reference_result,expected_reference_result);    
    
    bad_sequence[1] = 0x65; bad_sequence[2] = 0x35;
    actual_reference_result = HE100_referenceByteSequence(bad_sequence, 1);    
    ASSERT_EQ(actual_reference_result,expected_reference_result);    
}

// verify transmit data preparation bytes - with "Test Payload" message
TEST_F(Helium_100_Test, CorrectPayloadPreparation)
{
    //memset ( test_payload, "Test Payload", transmit_data_payload_length*sizeof(unsigned char) );
    size_t transmit_data_payload_length = 12;
    unsigned char test_payload[13] = "Test Payload"; // 12
    unsigned char transmit_data_command[2] = {0x10, 0x03};
    //unsigned char transmit_data_expected_value[transmit_data_payload_length+WRAPPER_LENGTH] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0,0x0,0x0,0x0,0x0,0x0};
    unsigned char transmit_data_expected_value[22] = {0x48,0x65,0x10,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0,0x0,0x0,0x0,0x0,0x0};
    unsigned char *prepare_result = HE100_prepareTransmission(test_payload, 12, transmit_data_command);	
    
	for (z = 0; z<transmit_data_payload_length+10; z++) {
        ASSERT_EQ(
            prepare_result[z],
            transmit_data_expected_value[z]
        );
    }
}

// verify NOOP preparation bytes
TEST_F(Helium_100_Test, CorrectNoopPayload)
{
    unsigned char noop_payload[1] = {0};
    unsigned char he100_noop_expected_value[8] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43};
    unsigned char noop_command[2] = {0x10, 0x01};
    //unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};

    unsigned char *noop_result = HE100_prepareTransmission(noop_payload, 0, noop_command);

	for (z = 0; z<8; z++) {
        ASSERT_EQ(
            noop_result[z],
            he100_noop_expected_value[z]
        );
    }
}

// verify Soft Reset preparation bytes
TEST_F(Helium_100_Test, CorrectSoftResetPayload)
{
    size_t soft_reset_payload_length = 0;
    unsigned char soft_reset_payload[1] = {0};
    //unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
    unsigned char soft_reset_command[2] = {0x10, 0x02};
    unsigned char he100_soft_reset_expected_value[8] = {0x48,0x65,0x10,0x02,0x00,0x00,0x12,0x46};

    unsigned char *soft_reset_result = HE100_prepareTransmission(soft_reset_payload, soft_reset_payload_length, soft_reset_command);

	for (z = 0; z<soft_reset_payload_length+8; z++) {
        ASSERT_EQ(
            soft_reset_result[z],
            he100_soft_reset_expected_value[z]
        );
    }
}

// verify Fast Set PA preparation bytes
TEST_F(Helium_100_Test, CorrectFastSetPaPayload)
{
    size_t fast_set_pa_payload_length = 1;
    unsigned char fast_set_pa_payload[1] = {11};
    //unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    unsigned char fast_set_pa_command[2] = {0x10, 0x20};
    unsigned char he100_fast_set_pa_expected_value[11] = {0x48,0x65,0x10,0x20,0x00,0x01,0x31,0xA1,0x03,0x06,0x0C};

    unsigned char *fast_set_pa_result = HE100_prepareTransmission(fast_set_pa_payload, 1, fast_set_pa_command);

	for (z = 0; z<fast_set_pa_payload_length+10; z++) {
        ASSERT_EQ(    
            fast_set_pa_result[z],
            he100_fast_set_pa_expected_value[z]
        );
    }
}

// Function should be tested for a string of bytes similar to expected but invalid

/*
// wrong length 
TEST_F(Helium_100_Test, WrongLength)
{
    ASSERT_EQ(
        HE100_storeValidResponse(response,length),
        -1
    );
}
*/

// invalid checksum
TEST_F(Helium_100_Test, InvalidChecksum)
{
    unsigned char invalid_checksum_response[12] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    size_t icr_length = 12;

    int actual_svr_result = HE100_storeValidResponse(invalid_checksum_response,icr_length);

    ASSERT_EQ(
        actual_svr_result,
        -1
    );
}

/*
// invalid command
TEST_F(Helium_100_Test, InvalidCommand)
{
    ASSERT_EQ(
        HE100_storeValidResponse(response,length),
        -1
    );
}
*/
