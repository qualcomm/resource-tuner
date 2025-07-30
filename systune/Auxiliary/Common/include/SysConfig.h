#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <cstdint>
#include <string>

#include "ErrCodes.h"
#include "Logger.h"
#include "SafeOps.h"

class SysConfig {
private:
    int8_t mReqType;
    std::string mProp;
    std::string mValue;
    std::string mDefValue;
    std::string mBuffer;
    uint64_t mBufferSize;
    int32_t mClientPID;
    int32_t mClientTID;

public:
    int8_t getRequestType();
    const std::string getProp();
    const std::string getValue();
    const std::string getDefaultValue();
    int32_t getClientPID();
    int32_t getClientTID();
    uint64_t getBufferSize();

    ErrCode serialize(char* buf);
    ErrCode deserialize(char* buf);

    void setRequestType(int8_t type);
    void setProp(const std::string& prop);
    void setValue(const std::string& value);
    void setDefaultValue(const std::string& defValue);
    void setClientPID(int32_t clientPID);
    void setClientTID(int32_t clientTID);
    void setBufferSize(uint64_t bufferSize);
};

#endif
