#include "gtest/gtest.h"

#include <string>

#include "Common/Logging.h"

class ParseTest : public testing::Test {
public:
    virtual void SetUp() {

    }

    virtual void TearDown() {
    }
};

// Test that testing works (TODO: Add actual tests!)
TEST_F(ParseTest, Dummy){
    const std::string sky = "blue";
    ASSERT_TRUE(sky == "blue");
}