#include "gtest/gtest.h"
#include "../inc/SC_he100.h"
#include "../inc/timer.h"
#include "../inc/fletcher.h"

class Helium_100_Live_Radio_Test : public ::testing::Test
{
    protected:
    virtual void SetUp() {
    }
    int fdin = HE100_openPort();
    virtual void TearDown() {
      HE100_closePort(fdin);
    }
    //if (fdin==0) exit(EXIT_FAILURE);
    size_t z; // assert loop index
};

// will be tested in following tests, but isolate some
// test null bytes, passing wrong length, etc
//struct fletcher_checksum fletcher_checksum16 (char *data, size_t bytes);

// Pass the function some data and check against expected result
unsigned char * HE100_prepareTransmission (unsigned char *payload, size_t length, unsigned char *command);

// Test the various bitshifting operations occuring in the library
/*
TEST_F(Helium_100_Live_Radio_Test, GoodBits)
{
    HE100_fastSetPA (int fdin, int power_level);
    ASSERT_EQ(

    );
}
*/

// Test writing to the helium device
TEST_F(Helium_100_Live_Radio_Test, GoodWrite)
{
    unsigned char write_test[8] = {0x48,0x65,0x10,0x01,0x00,0x00,0x11,0x43}; // 8
    int write_result = HE100_write(fdin, write_test, 8);

    ASSERT_EQ(
        0,
        write_result
    );
}

// Test NOOP
TEST_F(Helium_100_Live_Radio_Test, NOOP)
{
    int noop_result = HE100_NOOP(fdin);

    ASSERT_EQ(
        0,
        noop_result
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// Test transmitData
TEST_F(Helium_100_Live_Radio_Test, TransmitData)
{
    unsigned char payload[5] = {0x48,0x65,0x6c,0x6c,0x6f};
    int transmit_result = HE100_transmitData(fdin,payload,5);

    ASSERT_EQ(
        0,
        transmit_result
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// Test setBeaconInterval
TEST_F(Helium_100_Live_Radio_Test, SetBeaconInterval)
{
    int beacon_interval_result = HE100_NOOP(fdin);

    ASSERT_EQ(
        0,
        beacon_interval_result 
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// Test setBeaconMessage
TEST_F(Helium_100_Live_Radio_Test, SetBeaconMessage)
{
    unsigned char payload[7] = {0x62,0x65,0x61,0x63,0x6F,0x6E,0x0A};
    int beacon_message_result = HE100_setBeaconMessage(fdin,payload,7);

    ASSERT_EQ(
        0,
        beacon_message_result 
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// Test fastSetPA
TEST_F(Helium_100_Live_Radio_Test, FastSetPA)
{
    int fast_set_pa_result = HE100_fastSetPA(fdin,7);

    ASSERT_EQ(
        0,
        fast_set_pa_result 
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// Test softReset
TEST_F(Helium_100_Live_Radio_Test, SoftReset)
{
    int soft_reset_result = HE100_softReset(fdin);

    ASSERT_EQ(
        0,
        soft_reset_result
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// test readFirmwareRevision
TEST_F(Helium_100_Live_Radio_Test, ReadFirmwareRevision)
{
    int read_firmware_result = HE100_readFirmwareRevision(fdin);

    ASSERT_EQ(
        0,
        read_firmware_result 
    );
    // TODO READ THE ACTUAL BYTE SEQUENCE RETURNED
}

// test passing invalid PA level
TEST_F(Helium_100_Live_Radio_Test, InvalidPALevel)
{
    int ipl_actual_value = HE100_fastSetPA (fdin, 300);
    ASSERT_EQ(
        -1,
        ipl_actual_value
    );
}

// BEACON TESTING
TEST_F(Helium_100_Live_Radio_Test, SetBeaconThorough)
{
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
