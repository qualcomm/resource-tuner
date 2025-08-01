import os

# Auxiliary tests
os.system("./Tests/Unit/TimerTest")
os.system("./Tests/Unit/SafeOpsTest")

os.system("./Tests/Unit/MiscTests")
os.system("./Tests/Unit/ThreadPoolTests")
os.system("./Tests/Unit/MemoryPoolTests")
os.system("./Tests/Unit/RequestMapTests")
os.system("./Tests/Unit/ResourceProcessorTests")
os.system("./Tests/Unit/SysSignalConfigProcessorTests")
os.system("./Tests/Unit/SysConfigAPITests")
os.system("./Tests/Unit/SysConfigProcessorTests")
os.system("./Tests/Unit/TargetConfigProcessorTests")
os.system("./Tests/Unit/ClientDataManagerTests")
# os.system("./Tests/Unit/RateLimiterTests")

# os.system("python ../Tests/System/ServerLifecyleTests.py")
