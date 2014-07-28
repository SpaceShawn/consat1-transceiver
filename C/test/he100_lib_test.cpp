#include "gtest/gtest.h"
#include <stdio.h>
#include <fcntl.h>
#include <SC_he100.h>
#include <timer.h>
#include <fletcher.h>
#include <Date.h>
#include <shakespeare.h>
//#include <SC_he100.h>
#include <SpaceDecl.h>

#define PROCESS "HE100"
#define LOG_PATH "/home/logs/HE100"

class Helium_100_Test : public ::testing::Test
{
    protected:
    virtual void SetUp() {
        const ::testing::TestInfo* const test_info =
              ::testing::UnitTest::GetInstance()->current_test_info();
        char test_description[255] = {0};
        sprintf(test_description,"We are in test %s of test case %s.",
                       test_info->name(), test_info->test_case_name());
        Shakespeare::log_shorthand(LOG_PATH, Shakespeare::NOTICE, PROCESS, test_description);
    }
    virtual void TearDown() {
    }
    const static int fdin = 1; // fake file descriptor to simulate HE100
    size_t z; // assert loop index
    //unsigned char noop_ack[8]       = {0x48,0x65,0x20,0x01,0x0a,0x0a,0x35,0xa1};
    //unsigned char tx_ack[8]         = {0x48,0x65,0x20,0x03,0x0a,0x0a,0x37,0xa7};
    //unsigned char setcfg_ack[8]     = {0x48,0x65,0x20,0x06,0x0A,0x0A,0x3A,0xB0};
};

TEST_F(Helium_100_Test, ReadTest) {
    Shakespeare::log_shorthand(LOG_PATH, Shakespeare::NOTICE, PROCESS, "ReadTest");
    // prepare mock bytes to test with
    unsigned char mock_bytes[36] = {0x48,0x65,0x20,0x04,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f};

    // set up our mock serial device
    int pdm; // the master pseudo tty to write to
    int pds; // the slave pseudo tty to read from

    // set up master
    pdm = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (pdm < 0) ASSERT_EQ(0,pdm);

    // assign slave
    grantpt(pdm);
    unlockpt(pdm);
    pds = open(ptsname(pdm), O_RDWR | O_NOCTTY);

    if (pds < 0) ASSERT_EQ(0,pds);
    // write series of bytes to mock serial device, intended to be our incoming transmission
    int w;
    w = write (pds, mock_bytes, 36);
    ASSERT_EQ(w,36);

    // invoke HE100_read
    int r;
    unsigned char payload[255] = {0};
    r = HE100_read(pdm, 2, payload); // TODO ptr-ptr
    
    // analyse the payload, ASSERT (all_expected_bytes, all_actual_bytes)
    int y=8;
    for (z=0; z<26; z++) {
        ASSERT_EQ(
            mock_bytes[y],
            payload[z]
        );
        y++;
    }

    // flush and close the pseudo serial devices
    fsync(pdm);fsync(pds);
    close(pdm);close(pds);

    // final check to make sure read returns successfully
    ASSERT_EQ(26,r);
}

// TODO regenerate sample bytes from latest HE100 boards
// This test verifies the preparation of a transmission frame, and compares
// with a previously captured transmission from the Radio itself
TEST_F(Helium_100_Test, VerifyHeliumFrame)
{
    unsigned char helium_expected[36] = {0x48,0x65,0x20,0x04,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f};
    unsigned char helium_payload_bytes[26] = {0x86,0xA2,0x40,0x40,0x40,0x40,0x60,0xAC,0x8A,0x64,0x86,0xAA,0x82,0xE1,0x03,0xF0,0x6B,0x65,0x6E,0x77,0x6F,0x6F,0x64,0x0D,0x8D,0x08};
    unsigned char helium_receive_command[2] = {0x20, 0x04};
    unsigned char helium_result[255] = {0};
    int r = HE100_prepareTransmission(helium_payload_bytes, helium_result, 26, helium_receive_command);

    for (z=0; z<36; z++) {
        ASSERT_EQ(
            helium_expected[z],
            helium_result[z]
        );
    }
    ASSERT_EQ(CS1_SUCCESS,r);
}
  
// This test catches a bogus byte sequence
TEST_F(Helium_100_Test, Caught)
{
    int expected_reference_result = CS1_INVALID_BYTE_SEQUENCE;
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
    size_t transmit_data_payload_length = 12;
    unsigned char test_payload[13] = "Test Payload"; // 12
    unsigned char transmit_data_command[2] = {0x10, 0x03};
    unsigned char transmit_data_expected[22] = {0x48,0x65,0x10,0x03,0x00,0x0C,0x1F,0x55,0x54,0x65,0x73,0x74,0x20,0x50,0x61,0x79,0x6c,0x6f,0x61,0x64,0x1D,0xD9};
    unsigned char prepare_result[255] = {0}; 
    int r = HE100_prepareTransmission(test_payload, prepare_result, 12, transmit_data_command);
    //HE100_dumpHex(stdout,transmit_data_expected,22);
    //HE100_dumpHex(stdout,prepare_result,22);

	  for (z=0; z<transmit_data_payload_length+10; z++) {
        ASSERT_EQ(
            transmit_data_expected[z],
            prepare_result[z]
        );
    }
    ASSERT_EQ(CS1_SUCCESS,r);
}

// This test verifies the preparation of a NOOP transmission
TEST_F(Helium_100_Test, CorrectNoopPayload)
{
    unsigned char noop_payload[1] = {0};
    unsigned char he100_noop_expected_value[8] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43};
    unsigned char noop_command[2] = {0x10, 0x01};
    //unsigned char noop_command[2] = {CMD_TRANSMIT, CMD_NOOP};
    unsigned char noop_result[255] = {0};
    int r = HE100_prepareTransmission(noop_payload, noop_result, 0, noop_command);
    //HE100_dumpHex(stdout, he100_noop_expected_value, 8);
    //HE100_dumpHex(stdout, noop_result, 8);

	  for (z=0; z<8; z++) {
        ASSERT_EQ(
            he100_noop_expected_value[z],
            noop_result[z]
        );
    }
    ASSERT_EQ(CS1_SUCCESS,r);
}

// This test verifies the preparation of a Soft Reset transmission
TEST_F(Helium_100_Test, CorrectSoftResetPayload)
{
    size_t soft_reset_payload_length = 0;
    unsigned char soft_reset_payload[1] = {0};
    //unsigned char soft_reset_command[2] = {CMD_TRANSMIT, CMD_RESET};
    unsigned char soft_reset_command[2] = {0x10, 0x02};
    unsigned char he100_soft_reset_expected_value[8] = {0x48,0x65,0x10,0x02,0x00,0x00,0x12,0x46};
    unsigned char soft_reset_actual_result[255] = {0};
    int r = HE100_prepareTransmission(soft_reset_payload, soft_reset_actual_result, soft_reset_payload_length, soft_reset_command);

	  for (z=0; z<soft_reset_payload_length+8; z++) {
        ASSERT_EQ(
            he100_soft_reset_expected_value[z],
            soft_reset_actual_result[z]
        );
    }
    ASSERT_EQ(CS1_SUCCESS,r);
}

// This test verifies the preparation of a Fast Set PA transmission, and
// catches an invalid power level setting
TEST_F(Helium_100_Test, CorrectFastSetPaPayload)
{
    size_t fast_set_pa_payload_length = 1;
    unsigned char fast_set_pa_payload[1] = {3};
    //unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    unsigned char fast_set_pa_command[2] = {0x10, 0x20};
    unsigned char fast_set_pa_actual_result[255] = {0};
    int r = HE100_prepareTransmission(fast_set_pa_payload, fast_set_pa_actual_result, 1, fast_set_pa_command);
    unsigned char he100_fast_set_pa_expected_value[11] = {0x48,0x65,0x10,0x20,0x00,0x01,0x31,0xA1,0x03,0x06,0x0C};

    //HE100_dumpHex(stdout, fast_set_pa_actual_result, 11);
    //HE100_dumpHex(stdout, he100_fast_set_pa_expected_value, 11);

	  for (z=0; z<fast_set_pa_payload_length+10; z++) {
        ASSERT_EQ(
            he100_fast_set_pa_expected_value[z],
            fast_set_pa_actual_result[z]
        );
    }
    ASSERT_EQ(CS1_SUCCESS,r);
}

// test passing invalid PA level
TEST_F(Helium_100_Test, InvalidPALevel)
{
    int ipl_actual_value = HE100_fastSetPA (fdin, 300);
    ASSERT_EQ(
        1,
        ipl_actual_value
    );
}

// Function should be tested for a string of bytes similar to expected but invalid
// wrong length
TEST_F(Helium_100_Test, ValidateFrame_WrongLength)
{
    unsigned char response[36] = {0x48,0x65,0x20,0x04,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f}; // real payload
    // first verify with correct length
    ASSERT_EQ(
        0,
        HE100_validateFrame(response,36)
    );
    // then verify incorrect length is caught
    ASSERT_EQ(
        CS1_WRONG_LENGTH,
        HE100_validateFrame(response,37)
    );
}
/* 
// TODO length check not yet implemented in function
TEST_F(Helium_100_Test, DISABLE_PrepareTransmission_WrongLength)
{
    unsigned char helium_payload_bytes[26] = {0x86,0xA2,0x40,0x40,0x40,0x40,0x60,0xAC,0x8A,0x64,0x86,0xAA,0x82,0xE1,0x03,0xF0,0x6B,0x65,0x6E,0x77,0x6F,0x6F,0x64,0x0D,0x8D,0x08};
    unsigned char helium_receive_command[2] = {0x20, 0x04};
    unsigned char transmission[255] = {0};
    ASSERT_EQ(CS1_WRONG_LENGTH,HE100_prepareTransmission(helium_payload_bytes,transmission,28,helium_receive_command));
}
*/
// BEACON TESTING
TEST_F(Helium_100_Test, PrepareBeacon)
{
    int r=1; // return value
    unsigned char set_beacon_expected_value[4][11] = {
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x00,0xB8,0x28},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x01,0xB9,0x29},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x02,0xBA,0x2A},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x03,0xBB,0x2B}
    };

    //size_t set_beacon_payload_length = 1;
    unsigned char set_beacon_payload[1];
    //unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    unsigned char set_beacon_command[2] = {0x10, 0x11};
    unsigned char set_beacon_actual_value[255];

    int i;
    for (i=0;i<4;i++)
    {
        set_beacon_payload[0] = i;
        r = HE100_prepareTransmission(set_beacon_payload, set_beacon_actual_value, 1, set_beacon_command);

        for (z=0; z<11; z++) {
            ASSERT_EQ(
                set_beacon_expected_value[i][z],
                set_beacon_actual_value[z]
            );
        }
        ASSERT_EQ(CS1_SUCCESS, r);
    }
}

// invalid command
TEST_F(Helium_100_Test, InvalidCommand)
{
    unsigned char good_response[36] = {0x48,0x65,0x20,0x04,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f}; // fourth byte is incorrect
    unsigned char bad_response[36] = {0x48,0x65,0x20,0x63,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f}; // fourth byte is incorrect
    int length = 36; // Should be correct length
    ASSERT_EQ(
        0,
        HE100_validateFrame(good_response,length)
    );    ASSERT_EQ(
        HE_INVALID_COMMAND,
        HE100_validateFrame(bad_response,length)
    );
}

TEST_F(Helium_100_Test, ValidateFrame)
{
    unsigned char sv_expected[36] = {0x48,0x65,0x20,0x04,0x00,0x1a,0x3e,0xa6,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x6b,0x65,0x6e,0x77,0x6f,0x6f,0x64,0x0d,0x8d,0x08,0x63,0x9f};
    ASSERT_EQ(
        0,
        HE100_validateFrame(sv_expected,36)
    );
    unsigned char sv_expected2[41] = {0x48,0x65,0x20,0x04,0x00,0x1f,0x43,0xAB,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x33,0x32,0x30,0x30,0x30,0x31,0x33,0x31,0x36,0x34,0x46,0x42,0x0a,0x37,0x05,0x9e,0xDE};
    ASSERT_EQ(
        0,
        HE100_validateFrame(sv_expected2,41)
    );
    unsigned char sv_expected3[41] = {0x48,0x65,0x20,0x04,0x00,0x1f,0x43,0xAB,0x86,0xa2,0x40,0x40,0x40,0x40,0x60,0xac,0x8a,0x64,0x86,0xaa,0x82,0xe1,0x03,0xf0,0x33,0x33,0x30,0x30,0x30,0x31,0x32,0x31,0x35,0x35,0x45,0x46,0x0a,0xc4,0x96,0xbf,0x98};
    ASSERT_EQ(
        0,
        HE100_validateFrame(sv_expected3,41)
    );
}

// invalid checksum
// 1 - pass a frame with an invalid checksum to HE100_validateFrame
// 2 - if an invalid checksum is encountered, the payload should not be persisted,
//     and the event should be logged by shakespeare
TEST_F(Helium_100_Test, CatchBadChecksum)
{
    unsigned char invalid_checksum_response[11] = {0x48,0x65,0x10,0x20,0x00,0x01,0x31,0xA1,0x03,0x06,0x0B}; // payload checksum byte 11 is incorrect
    size_t icr_length = 11;

    int actual_svr_result = HE100_validateFrame(invalid_checksum_response,icr_length);

    ASSERT_EQ(
        HE_FAILED_CHECKSUM,
        actual_svr_result
    );

    invalid_checksum_response[10] = 0x0A; // payload checksum byte 11 is corrected
    invalid_checksum_response[7] = 0x35; // header checksum byte 8 is incorrect
    actual_svr_result = HE100_validateFrame(invalid_checksum_response,icr_length);

    ASSERT_EQ(
        HE_FAILED_CHECKSUM,
        actual_svr_result
    );
}
 
// BEACON TESTING
TEST_F(Helium_100_Test, SetBeaconInterval)
{
    int r=1;
    unsigned char set_beacon_expected_value[4][11] = {
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x00,0xB8,0x28},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x01,0xB9,0x29},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x02,0xBA,0x2A},
        {0x48,0x65,0x10,0x11,0x00,0x01,0x22,0x74,0x03,0xBB,0x2B}
    };

    //size_t set_beacon_payload_length = 1;
    unsigned char set_beacon_payload[1];

    //unsigned char fast_set_pa_command[2] = {CMD_TRANSMIT, CMD_FAST_SET_PA};
    unsigned char set_beacon_command[2] = {0x10, 0x11};
    unsigned char set_beacon_actual_value[255] = {0};

    int i;
    for (i=0;i<4;i++)
    {
        set_beacon_payload[0] = i;
        r = HE100_prepareTransmission(set_beacon_payload, set_beacon_actual_value, 1, set_beacon_command);

        for (z=0; z<11; z++) {
            ASSERT_EQ(
                set_beacon_expected_value[i][z],
                set_beacon_actual_value[z]
            );
        }
        ASSERT_EQ(CS1_SUCCESS,r);
    }
}

TEST_F(Helium_100_Test, TestCollectValidConfig)
{
    unsigned char config1[44] = {0x48,0x65,0x20,0x05,0x00,0x22,0x47,0xb1,0x00,0x87,0x01,0x01,0x00,0x00,0xa8,0x3c,0x02,0x00,0x08,0xab,0x06,0x00,0x56,0x41,0x33,0x4f,0x52,0x42,0x56,0x45,0x32,0x43,0x55,0x41,0x05,0x00,0x00,0x00,0x41,0x80,0x00,0x00,0xfa,0x10};
    unsigned char config2[44] = {0x48,0x65,0x10,0x06,0x00,0x22,0x38,0x74,0x00,0x00,0x01,0x01,0x00,0x00,0x48,0x33,0x02,0x00,0x98,0x93,0x06,0x00,0x56,0x41,0x33,0x4F,0x52,0x42,0x56,0x45,0x32,0x43,0x55,0x41,0x09,0x00,0x00,0x00,0x43,0x00,0x00,0x00,0x33,0x78}; 
    struct he100_settings test_settings_1 = HE100_collectConfig(config1);
    struct he100_settings test_settings_2 = HE100_collectConfig(config2);
    int validation_result = 1; 
    validation_result = HE100_validateConfig(test_settings_1);
    ASSERT_EQ (0, validation_result);   
    validation_result = HE100_validateConfig(test_settings_2);
    ASSERT_EQ (0, validation_result);   
}

/*
TEST_F(Helium_100_Test, TestPrepareConfig)
{
    struct he100_settings good_settings;
    good_settings
    unsigned char config_data[CFG_PAYLOAD_LENGTH];_
    struct he100_settings good_settings = HE100_prepareConfig(config_data, good_config);
    ASSERT_EQ(9600, config_data[]); // TODO test all like this to known good config
}
*/

/*
Send Beacon Data
486510100100217231313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131B4E8
He[16][16][1][0]!r1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111[180][232]
*/

// TODO test pipe operations

// TODO test log operations

// TODO Test the various bitshifting operations occuring in the library
/*
TEST_F(Helium_100_Test, GoodBits)
{
    HE100_fastSetPA (int fdin, int power_level);
    ASSERT_EQ(

    );
}
*/

// TODO test null bytes, passing wrong length, etc
