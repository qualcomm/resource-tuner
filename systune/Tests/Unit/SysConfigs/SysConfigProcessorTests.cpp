#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "SysConfigProcessor.h"
#include "SysConfigPropRegistry.h"
#include "Extensions.h"
#include "Utils.h"
#include "Logger.h"

URM_REGISTER_CONFIG(PROPERTIES_CONFIG, "../Tests/Configs/testPropertiesConfig.json")

#define TOTAL_SYS_CONFIGS_PROPS_COUNT 14

class SysConfigProcessorTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            std::shared_ptr<SysConfigProcessor> sysConfigProcessor =
                SysConfigProcessor::getInstance(Extensions::getPropertiesConfigFilePath());

            if(RC_IS_NOTOK(sysConfigProcessor->parseSysConfigs())) {
                LOGE("URM_TEST_SYSCONFIG_PARSER", "SysConfig Config Parsing Failed");
                return;
            }
        }
    }
};

TEST_F(SysConfigProcessorTests, TestSysConfigProcessorJSONDataIntegrity1) {
    ASSERT_NE(SysConfigPropRegistry::getInstance(), nullptr);
}

TEST_F(SysConfigProcessorTests, TestSignalConfigProcessorJSONDataIntegrity2) {
    ASSERT_EQ(SysConfigPropRegistry::getInstance()->getPropertiesCount(), TOTAL_SYS_CONFIGS_PROPS_COUNT);
}
