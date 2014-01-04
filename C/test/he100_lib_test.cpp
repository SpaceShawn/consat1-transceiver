#include "gtest/gtest.h"
#include "../inc/SC_he100.h"
#include "../inc/timer.h"

class Helium_100_Test : public ::testing::Test
{
    protected:
    virtual void SetUp() { }

    const static int fdin = 1; // fake file descriptor to simulate HE100   
    size_t z; // assert loop index
};

// will be tested in following tests, but isolate some 
// test null bytes, passing wrong length, etc
//struct HE100_checksum HE100_fletcher16 (char *data, size_t bytes);

//struct HE100_checksum HE100_KBPayloadChecksum (unsigned char *data, size_t size);
/**
 * Kevin Brown's Fletcher checksum, use to compare to ours
 */
struct HE100_checksum
HE100_KBPayloadChecksum (unsigned char *data, size_t size)
{
    size_t i = 0;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;

    for( i = 0; i < size; i++ )
    {
        ck_a += data[i];
        ck_b += ck_a;
    }

    HE100_checksum r;
        
    r.sum1 = ck_a;
    r.sum2 = ck_b;

    return r;
}

// Pass the function some data and check against expected result
unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

// Test the various bitshifting operations occuring in the library
/*
TEST_F(Helium_100_Test, GoodBits)
{
    HE100_fastSetPA (int fdin, int power_level);
    ASSERT_EQ(
               
    );
}
*/

/*
// Test the fletcher checksum
// NOTE: reality on the HE100 is subjective and unexpected
//EXPECTED http://www.lammertbies.nl/comm/info/crc-calculation.html
TEST_F(Helium_100_Test, FletcherChecksum)
{

    // unsigned char *checksum_bytes[97] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghjklmnopqrstuvwxyz~!@#$%^&*()_+`1234567890-={}\\|:\"<>?[];',./";
    
    //ACTUAL
    unsigned char checksum_bytes[56] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghjklmnopqrstuvwxyz~!@";
    HE100_checksum checksum_result = HE100_fletcher16(checksum_bytes,56);
    
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
*/

// Compare our fletcher algorithm with Kevin Brown's
TEST_F(Helium_100_Test, BrownVsWorld)
{
    unsigned char checksum_bytes[56] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcedefghjklmnopqrstuvwxyz~!@";
    HE100_checksum our_result = HE100_fletcher16(checksum_bytes,56);
    HE100_checksum his_result = HE100_KBPayloadChecksum(checksum_bytes,56);
    
    ASSERT_EQ(
        his_result.sum1,
        our_result.sum1
    ); 
    ASSERT_EQ(
        his_result.sum2,
        our_result.sum2
    );  
}

// Compare our fletcher algorithm with Kevin Brown's
TEST_F(Helium_100_Test, TestIncomingChecksum)
{
    unsigned char checksum_bytes[33] = {0x20,0x04,0x00,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08};
    HE100_checksum our_result = HE100_fletcher16(checksum_bytes,33);
    
    ASSERT_EQ(
        0x63,
        our_result.sum1
    ); 
    ASSERT_EQ(
        0x9f,
        our_result.sum2
    );  
}

TEST_F(Helium_100_Test, VerifyHeliumFrame)
{
    unsigned char helium_expected[37] = {0x48,0x65,0x20,0x04,0x00,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f};

    unsigned char helium_payload_bytes[27] = {0xA6,0x86,0xA2,0x40,0x40,0x40,0x40,0x60,0xAC,0x8A,0x64,0x86,0xAA,0x82,0xE1,0x03,0xF0,0x6B,0x65,0x6E,0x77,0x6F,0x6F,0x64,0x0D,0x8D,0x08};
    unsigned char helium_receive_command[2] = {0x20, 0x04};
    unsigned char *helium_result = HE100_prepareTransmission(helium_payload_bytes, 27, helium_receive_command);

    HE100_dumpHex(stdout, helium_result, 36);
    HE100_dumpHex(stdout, helium_payload_bytes, 36);

    for (z=0; z<37; z++) {
        ASSERT_EQ(
            helium_expected[z],
            helium_result[z]
        );
    }

    /* 
    ASSERT_EQ(
        helium_expected[35], //0x63,
        helium_result[35]
    );
    ASSERT_EQ(
        helium_expected[36], //0x9f,
        helium_result[36]
    );
    */
}

// Test writing to the helium device
TEST_F(Helium_100_Test, GoodWrite)
{
    unsigned char write_test[8] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43}; // 8
    int write_result = HE100_write(fdin, write_test, 8);
    
    ASSERT_EQ(
        1,
        write_result
    );
}

// Test a bogus byte sequence
TEST_F(Helium_100_Test, Caught)
{
    int expected_reference_result = -1;
    int actual_reference_result;

    unsigned char bad_sequence[8] = {0x47,0x65,0x10,0x01,0x00,0x00,0x11,0x43};
    actual_reference_result = HE100_referenceByteSequence(bad_sequence, 0);
    ASSERT_EQ(expected_reference_result,actual_reference_result);

    bad_sequence[0] = 0x48; bad_sequence[1] = 0x64;
    actual_reference_result = HE100_referenceByteSequence(bad_sequence, 1); 
    ASSERT_EQ(expected_reference_result,actual_reference_result);    
    
    bad_sequence[1] = 0x65; bad_sequence[2] = 0x35;
    actual_reference_result = HE100_referenceByteSequence(bad_sequence, 1);    
    ASSERT_EQ(expected_reference_result,actual_reference_result);    
}

// verify transmit data preparation bytes - with "Test Payload" message
TEST_F(Helium_100_Test, CorrectPayloadPreparation)
{
    //memset ( test_payload, "Test Payload", transmit_data_payload_length*sizeof(unsigned char) );
    size_t transmit_data_payload_length = 12;
    unsigned char test_payload[13] = "Test Payload"; // 12
    unsigned char transmit_data_command[2] = {0x10, 0x03};
    //unsigned char transmit_data_expected_value[transmit_data_payload_length+WRAPPER_LENGTH] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0,0x0,0x0,0x0,0x0,0x0};
    unsigned char transmit_data_expected[22] = {0x48,0x65,0x10,0x03,0x00,0x0C,0x1F,0x55,0x54,0x65,0x73,0x74,0x20,0x50,0x61,0x79,0x6c,0x6f,0x61,0x64,0x1D,0xD9};
    unsigned char *prepare_result = HE100_prepareTransmission(test_payload, 12, transmit_data_command);	
    
    HE100_dumpHex(stdout,transmit_data_expected,22);
    HE100_dumpHex(stdout,prepare_result,22);

	for (z=0; z<transmit_data_payload_length+10; z++) {
        ASSERT_EQ(
            transmit_data_expected[z],
            prepare_result[z]
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
    
    HE100_dumpHex(stdout, he100_noop_expected_value, 8);
    HE100_dumpHex(stdout, noop_result, 8);

	for (z=0; z<8; z++) {
        ASSERT_EQ(
            he100_noop_expected_value[z],
            noop_result[z]
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

    unsigned char *soft_reset_actual_result = HE100_prepareTransmission(soft_reset_payload, soft_reset_payload_length, soft_reset_command);

	for (z=0; z<soft_reset_payload_length+8; z++) {
        ASSERT_EQ(
            he100_soft_reset_expected_value[z],
            soft_reset_actual_result[z]
        );
    }
}

// verify Fast Set PA preparation bytes
TEST_F(Helium_100_Test, CorrectFastSetPaPayload)
{
    size_t fast_set_pa_payload_length = 1;
    unsigned char fast_set_pa_payload[1] = {3};
    //unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    unsigned char fast_set_pa_command[2] = {0x10, 0x20};
    unsigned char *fast_set_pa_actual_result = HE100_prepareTransmission(fast_set_pa_payload, 1, fast_set_pa_command);
    unsigned char he100_fast_set_pa_expected_value[11] = {0x48,0x65,0x10,0x20,0x00,0x01,0x31,0xA1,0x03,0x06,0x0C};
    
    HE100_dumpHex(stdout, fast_set_pa_actual_result, 11);
    HE100_dumpHex(stdout, he100_fast_set_pa_expected_value, 11);

	for (z=0; z<fast_set_pa_payload_length+10; z++) {
        ASSERT_EQ(    
            he100_fast_set_pa_expected_value[z],
            fast_set_pa_actual_result[z]
        );
    }
}

// test passing invalid PA level
TEST_F(Helium_100_Test, InvalidPALevel)
{
    int ipl_actual_value = HE100_fastSetPA (fdin, 300);
    ASSERT_EQ(
        -1,
        ipl_actual_value
    );
}

// Function should be tested for a string of bytes similar to expected but invalid

/*
// wrong length 
TEST_F(Helium_100_Test, WrongLength)
{
    ASSERT_EQ(
        -1,
        HE100_storeValidResponse(response,length)
    );
}
*/

// invalid checksum
// 1 - pass a frame with an invalid checksum to HE100_storeValidResponse 
// 2 - if an invalid checksum is encountered, the payload should not be persisted,
//     and the event should be logged by shakespeare
TEST_F(Helium_100_Test, CatchBadChecksum)
{
    unsigned char invalid_checksum_response[11] = {0x48,0x65,0x10,0x20,0x00,0x01,0x31,0xA1,0x03,0x06,0x0B}; // payload checksum byte 11 is incorrect
    size_t icr_length = 11;

    int actual_svr_result = HE100_storeValidResponse(invalid_checksum_response,icr_length);

    ASSERT_EQ(
        -1,
        actual_svr_result
    );
    
    invalid_checksum_response[10] = 0x0A; // payload checksum byte 11 is corrected
    invalid_checksum_response[7] = 0x35; // header checksum byte 8 is incorrect 
    actual_svr_result = HE100_storeValidResponse(invalid_checksum_response,icr_length);    
    
    ASSERT_EQ(
        -1,
        actual_svr_result
    );
}

/*
// invalid command
TEST_F(Helium_100_Test, InvalidCommand)
{
    ASSERT_EQ(
        -1,
        HE100_storeValidResponse(response,length)
    );
}
*/

// BEACON TESTING
TEST_F(Helium_100_Test, SetBeaconInterval)
{
    unsigned char set_beacon_expected_value[4][11] = {
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x00,0xB8,0x28},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x01,0xB9,0x29},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x02,0xBA,0x2A},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x03,0xBB,0x2B}
    };
    
    size_t set_beacon_payload_length = 1;
    unsigned char set_beacon_payload[1]; 
    
    //unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    unsigned char set_beacon_command[2] = {0x10, 0x11};
    unsigned char *set_beacon_actual_value;

    int i;
    for (i=0;i<4;i++)
    {
        set_beacon_payload[0] = i;
        set_beacon_actual_value = HE100_prepareTransmission(set_beacon_payload, 1, set_beacon_command);
        
        for (z=0; z<11; z++) {
            ASSERT_EQ(
                set_beacon_expected_value[i][z],
                set_beacon_actual_value[z]
            );
        }
    }
}

/*
Send Beacon Data
486510100100217231313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131B4E8
He[16][16][1][0]!r1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111[180][232]
*/

// test pipe operations

// test log operations
