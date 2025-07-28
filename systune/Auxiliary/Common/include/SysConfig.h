#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <cstdint>

class SysConfig {
private:
    int8_t mReqType;
    const char* mProp;
    const char* mValue;
    const char* mDefValue;
    const char* mBuffer;
    uint64_t mBufferSize;
    int32_t mClientPID;
    int32_t mClientTID;

public:
    int8_t getRequestType();
    const char* getProp();
    const char* getValue();
    const char* getDefaultValue();
    const char* getBuffer();
    int32_t getClientPID();
    int32_t getClientTID();
    uint64_t getBufferSize();

    void setRequestType(int8_t type);
    void setProp(const char* prop);
    void setValue(const char* value);
    void setDefaultValue(const char* defValue);
    void setClientPID(int32_t clientPID);
    void setClientTID(int32_t clientTID);
    void setBufferSize(uint64_t bufferSize);
};

#endif
