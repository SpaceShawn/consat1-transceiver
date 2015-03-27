#include "gtest/gtest.h"
#include <SC_serial.h>

class SC_Serial_Test : public ::testing::Test
{
    public:
      int fdin;
    protected:
      virtual void SetUp() {
        fdin = SC_openPort(); 
        if (fdin==-1) exit(EXIT_FAILURE);
        const ::testing::TestInfo* const test_info =
              ::testing::UnitTest::GetInstance()->current_test_info();
        char test_description[255] = {0};
        Shakespeare::log(Shakespeare::NOTICE, PROCESS, ">>>>>>>>>>>>>>>>>>>>>> NEW TEST <<<<<<<<<<<<<<<<<<<<<<");
        sprintf(test_description,"We are in test %s of test case %s.",
                        test_info->name(), test_info->test_case_name());
        Shakespeare::log(Shakespeare::NOTICE, PROCESS, test_description);
      }
      virtual void TearDown() {
        SC_closePort(fdin);
        Shakespeare::log(Shakespeare::NOTICE, PROCESS, ">>>>>>>>>>>>>>>>>>>>>> END TEST <<<<<<<<<<<<<<<<<<<<<<");
      }
      size_t z; // assert loop index
};

// loopback connector 
//
// buffer full
//
// hardware failure
//
